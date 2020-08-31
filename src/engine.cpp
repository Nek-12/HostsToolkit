#include "src/engine.h"
#include "src/network.h"
#include <filesystem>
#include <qdebug.h>
#include <qobject.h>
#include <qprogressdialog.h>
#include <qthread.h>
#include <qurl.h>
#include <stdexcept>
namespace fs = std::filesystem;

//          -------------ENGINE------------
Engine::Engine(QObject* parent) : QObject(parent) {
    connect(&dl_mgr, &DownloadManager::message, this, &Engine::message);
    connect(&dl_mgr, &DownloadManager::all_finished, this,
            &Engine::all_dls_finished);
    connect(&dl_mgr, &DownloadManager::dl_failed, this, &Engine::stop);
    connect(&dl_mgr, &DownloadManager::dl_failed, this, &Engine::failed);
    connect(&dl_mgr, &DownloadManager::progress, this, &Engine::progress);
    connect(&dl_mgr, &DownloadManager::report_speed, this, &Engine::report_speed);
    qDebug() << "Created an Engine";
}

void Engine::start_work(const std::string& hosts_path, bool rem_comments,
                        bool rem_dups, bool add_credits, bool add_stats) {
    comms = rem_comments;
    dups  = rem_dups;
    credits = add_credits;
    stats   = add_stats; // save the data for later
    path = hosts_path;
    if (working) {
        emit failed("The process is already running");
        return;
    }
    fs::remove_all(DL_FOLDER); // clean up an orphaned temp dir
    fs::create_directory(DL_FOLDER);
    qDebug() << "Starting work";
    pending = false;
    emit state_updated();
    emit message("Downloading files...");
    for (const auto& u : urls)
        dl_mgr.append(u);
    abort = false;
    working = true;
    dl_mgr.go(); // begin downloading and wait...
    // forget about the engine for now
}

void Engine::stop() {
    qDebug() << "Ordered engine to stop";
    abort = true;
    pending = true;
    working = false;
    dl_mgr.stop();
    fs::remove_all(DL_FOLDER); // delete temp folder
    emit state_updated();
}

Engine::~Engine() {
    stop();
}

void Engine::add_custom(const std::string& item) {
    custom_lines.push_back(item);
    pending = true;
    emit state_updated();
}
void Engine::add_file(const std::string& item) {
    filepaths.push_back(item);
    pending = true;
    emit state_updated();
}
void Engine::add_url(const QUrl& item) {
    urls.push_back(item);
    pending = true;
    emit state_updated();
}
void Engine::rem_custom(size_t row) {
    custom_lines.erase(custom_lines.begin() + row);
    pending = true;
    emit state_updated();
}
void Engine::rem_file(size_t row) {
    filepaths.erase(filepaths.begin() + row);
    pending = true;
    emit state_updated();
}
void Engine::rem_url(size_t row) {
    urls.erase(urls.begin() + row);
    pending = true;
    emit state_updated();
}

// save stuff
bool Engine::save_entries(const std::string& path) {
    std::ofstream f(path);
    if (!f)
        return false;
    f << "# HostsTools configuration file.\n"
      << CREDITS << "# You may edit this file, "
      << "but don't change the order of >HEADERs!\n";
    f << SPLIT_CHAR << " CUSTOM\n";
    for (const auto& el : custom_lines)
        f << el << '\n';
    f << SPLIT_CHAR << " FILES\n";
    for (const auto& el : filepaths)
        f << el << '\n';
    f << SPLIT_CHAR << " URLS\n";
    for (const auto& el : urls)
        f << el.toString().toStdString() << '\n';
    return true;
}

// Create and apply the file. The most important function
void Engine::all_dls_finished() try {
    report_speed("Working on files...");
    auto check = [this]() {
        if (abort)
            throw std::runtime_error("Operation aborted");
    };
    //if abort was set before downloads have finished, then the error has already been reported.
    if (abort) {
        stop();
        return;
    }
    qDebug() << "Starting to work on file, path: " << QString::fromStdString(path);
    QString msg = "Processing files... \n";
    qDebug() << msg;
    emit       message(msg);                   // send info periodically
    qulonglong commented_lines = 0;
    // 1. Process custom files (first to avoid rewriting the file we loaded. It might be in the list of files!)
    std::stringstream f_contents;
    for (const auto& fname : filepaths) { // for every file specified
        check();
        std::ifstream f(fname);
        if (f) { // open, read
            f_contents << f.rdbuf();
        } else {
            throw std::runtime_error(
                std::string("Couldn't open specified file: ").append(fname));
        }
    }
    // 2. Add the files we downloaded
    msg = "Processing downloaded sources... \n";
    qDebug() << msg;
    emit message(msg);
    // ".temp/hosts_0" or similar
    for (const auto& p : fs::recursive_directory_iterator(DL_FOLDER)) {
        // iterate over every file in the dir
        if (fs::is_directory(p))
            continue; // exclude directory from output
        qDebug() << "Found file " << QString::fromStdString(p.path().string());
        check();
        std::ifstream f(p.path()); // open the file
        if (f) {
            f_contents << f.rdbuf();
            qDebug() << "Added file " << QString::fromStdString(p.path().string());
        } else {
            throw std::runtime_error(
                std::string("Couldn't open file:").append(p.path().string()));
        }
    }
    // 3. Open and clear the desired file
    std::fstream ret(
        path, std::fstream::in | std::fstream::out |
                  std::fstream::trunc); // open a file rw, create if needed.
    // file system dialog will warn the user about overwriting for us.
    if (!ret)
        throw std::runtime_error(
            std::string("Couldn't open file: ").append(path));
    // 4. Add credits
    if (credits)
        ret << CREDITS;
    // 5. Add custom entries
    ret << "\n# ------------- C U S T O M ------------- \n";
    for (const auto& l : custom_lines)
        ret << l << '\n';
    size_t total_lines = 0; //! Possible division by zero
    if (stats) {        // count the contents of both streams
        ret.clear();
        ret.seekg(std::ios::beg);
        total_lines += std::count(std::istreambuf_iterator<char>(ret),
                                  std::istreambuf_iterator<char>(), '\n');
        qDebug() << "total_lines before merging files: " << total_lines;
        auto s = f_contents.str();
        total_lines += std::count(s.begin(), s.end(), '\n');
        assert(total_lines > 0); // overflow
    }
    qDebug() << "Starting to merge files";
    // 6. Process the data in the files
    if (dups) {
        ret << "\n# ------------- DEDUPLICATED AND SORTED ------------- \n";
        std::set<std::string> strset; // set does not store duplicates
        while (f_contents) {          // Process lines
            check();
            if (stats)
                emit progress(int(strset.size() * 100 / total_lines));
            std::string line;
            std::getline(f_contents, line);
            auto pair = process_line(line);
            commented_lines += pair.first; // remove comments if needed and
                                           // increment the counter
            if (!pair.second.empty())
                strset.insert(pair.second);
        }
        for (const auto& s : strset)
            ret << s << '\n'; // save the changes, no duplicates
    } else {                  // don't remove duplicates
        std::vector<std::string> strvec;
        ret << "\n# ------------- MERGED ------------- \n";
        while (f_contents) { // Process lines
            check();
            if (stats)
                emit progress(int(strvec.size() * 100 / total_lines));
            std::string line;
            std::getline(f_contents, line);
            auto pair = process_line(line);
            commented_lines += pair.first; // remove comments if needed and
                                           // increment the counter
            if (!pair.second.empty())
                strvec.push_back(pair.second);
        }
        for (const auto& s : strvec) {
            ret << s << '\n'; // write every line back
        }
    }
    f_contents.clear(); // reduce memory overhead
    qDebug() << "Finished merging files";
    // 7. Get remaining statistics
    if (stats) { // save calculation time by skipping
        emit        message("Getting your statistics...");
        Stats       st;
        char        symbol = 0;
        emit        progress(10);
        symbol = '#';
        ret.seekg(std::ios::beg);
        st.comments += std::count(std::istreambuf_iterator<char>(ret),
                                  std::istreambuf_iterator<char>(), symbol);
        emit progress(50);
        check();
        symbol = '\n';
        ret.seekg(std::ios::beg);
        st.lines += std::count(std::istreambuf_iterator<char>(ret),
                               std::istreambuf_iterator<char>(), symbol); //end result
        check();
        emit progress(90);
#ifdef TIME_MULTIPLIER
        st.seconds_added = st.lines / TIME_MULTIPLIER;
#endif
        qDebug() << st.lines << " total lines in the resulting file";
        st.removed = total_lines - st.lines; // total_lines (before optimizing) - st.lines (after)
        st.sources =
            dl_mgr.get_total() + filepaths.size() + 1; // 1 for custom lines
        st.size = fs::file_size(path);                 // in bytes
        emit stats_ready(st);
    }
    working = false;
    emit message("Done!");
    emit progress(100);
    emit success();
} catch (const std::exception& e) {
    auto msg = QString::fromStdString(e.what());
    qDebug() << msg;
    stop();
    emit failed(msg);
    working = false;
    return;
}

auto Engine::process_line(std::string line) const
    -> std::pair<bool, std::string> {
    if (comms) { // remove comments
        auto it = line.find('#');
        if (it != std::string::npos) {
            line.erase(it);
            if (std::all_of(line.begin(), line.end(),
                            isspace)) // Check if only whitespace remains
                line.clear();
            return std::make_pair(true, line);
        }
    }
    //Replace tabs with spaces
    std::replace_if(line.begin(),line.end(), isspace, ' ');
    // Remove repeating spaces
    std::string::iterator new_end =
        std::unique(line.begin(), line.end(), [](char lhs, char rhs) {
            return (lhs == rhs) && (lhs == ' ');
        });
    line.erase(new_end, line.end());
    return std::make_pair(false, line);
}
