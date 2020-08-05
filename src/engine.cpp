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
    qDebug() << "Creating an Engine...";
    progress_bar_ptr =
        new QProgressDialog("Building hosts file...", "Abort", 0, 100);
    progress_bar_ptr->setWindowTitle("Please stand by...");
    progress_bar_ptr->setAutoClose(true);
    progress_bar_ptr->setWindowModality(Qt::WindowModal);
    progress_bar_ptr->setAutoReset(false);
    progress_bar_ptr->reset();
    qDebug() << "Created and hidden the progress window";
    connect(progress_bar_ptr, &QProgressDialog::canceled, this, &Engine::stop);
}

void Engine::start_work(const std::string& hosts, bool rem_comments,
                        bool rem_dups, bool add_credits, bool add_stats) {
    if (slave) {
        emit failed("The process is already running");
        return;
    }
    progress_bar_ptr->reset();
    qDebug() << "Employing a slave...";
    slave   = new Slave(this, rem_comments, rem_dups, add_credits, add_stats,
                      filepaths, custom_lines, urls, hosts);
    pending = false;
    emit state_updated();
    // connect the thread
    connect(slave, &Slave::success, this,
            &Engine::thread_success); // signal success
    connect(slave, &Slave::failure, this,
            &Engine::thread_failure); // signal failure
    connect(slave, &Slave::progress, this, &Engine::progress);
    connect(slave, &Slave::message, this, &Engine::message);
    connect(slave, &Slave::stats, this, &Engine::stats);
    qDebug() << "Showing the progressbar";
    progress_bar_ptr->show();
    qDebug() << "Starting engine's slave";
    slave->start();
    // forget about the slave for now
}

void Engine::thread_success() {
    stop();
    emit ready();
}

void Engine::thread_failure(const QString& msg) {
    stop();
    pending = true;
    emit state_updated();
    emit failed(msg);
}

void Engine::progress(int n) { progress_bar_ptr->setValue(n); }
void Engine::message(const QString& s) { progress_bar_ptr->setLabelText(s); }

Engine::~Engine() {
    stop();
    delete progress_bar_ptr;
}

void Engine::stop() {
    if (slave) {
        slave->stop();
        slave->wait();
        delete slave;
        slave = nullptr;
    }
    fs::remove_all(DL_FOLDER); // delete temp folder
    progress_bar_ptr->reset();
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

void Engine::save_entries(std::ofstream& f) {
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
}

//              -----------SLAVE-----------
Slave::Slave(QObject* parent, bool rem_comments, bool rem_dups,
             bool add_credits, bool add_stats,
             std::vector<std::string> filepaths,
             std::vector<std::string> custom_lines,
             const std::vector<QUrl>& urls, std::string path) :
    QThread(parent),
    dl_mgr(this), rem_comments(rem_comments), rem_dups(rem_dups),
    add_credits(add_credits), add_stats(add_stats),
    filepaths(std::move(filepaths)), custom_lines(std::move(custom_lines)),
    path(std::move(path)) {
    connect(&dl_mgr, &DownloadManager::message, this, &Slave::message);
    connect(&dl_mgr, &DownloadManager::all_finished, this,
            &Slave::all_dls_finished);
    connect(&dl_mgr, &DownloadManager::dl_failed, this, &Slave::failure);
    connect(&dl_mgr, &DownloadManager::progress, this, &Slave::progress);
    // Add downloads, but don't go yet.
    for (const auto& url : urls)
        dl_mgr.append(url);
    qDebug() << "Created new Slave";
}

void Slave::run() {
    emit message("Downloading files...");
    abort = false;
    dl_mgr.go(); // begin downloading and wait...
}

// Create and apply the file. The most important function
void Slave::all_dls_finished() try {
    if (abort)
        return;
    std::fstream ret(path); // open a file
    if (!ret)
        emit failure("Couldn't open the file");
    QString msg = "Processing files... \n";
    qDebug() << msg;
    emit       message(msg);                   // send info periodically
    qulonglong commented_lines = 0, total = 0; // for stats
                                               // 1. Add credits
    if (add_credits)
        ret << CREDITS;
    // 2. Add custom entries
    ret << "\n# ------------- C U S T O M ------------- \n";
    for (const auto& l : custom_lines)
        ret << l << '\n';
    // 3. Process downloaded files
    std::stringstream f_contents;
    for (const auto& fname : filepaths) { // for every file specified
        if (abort)
            return;
        std::ifstream f(fname);
        if (f) { // open, read
            f_contents << f.rdbuf();
        } else {
            throw std::runtime_error(
                std::string("Couldn't open file: ").append(fname));
        }
    }
    // 4. Add the files we downloaded
    msg = "Processing downloaded sources... \n";
    qDebug() << msg;
    emit message(msg);
    // ".temp/hosts_0" or similar
    for (const auto& p : fs::recursive_directory_iterator(DL_FOLDER)) {
        // iterate over every file in the dir
        if (fs::is_directory(p))
            continue; // exclude directory from output
        qDebug() << "Found file " << QString::fromStdString(p.path());
        if (abort) // check periodically
            return;
        std::ifstream f(p.path()); // open the file
        if (f) {
            f_contents << f.rdbuf();
            qDebug() << "Added file " << QString::fromStdString(p.path());
        } else {
            throw std::runtime_error(
                std::string("Couldn't open file:").append(p.path()));
        }
    }
    size_t total_lines = 0; //! Possible division by zero
    if (add_stats) {        // count the contents of both streams
        //? ret.clear();
        total_lines += std::count(std::istreambuf_iterator<char>(ret),
                                  std::istreambuf_iterator<char>(), '\n');
        qDebug() << "total_lines before merging files: " << total_lines;
        auto s = f_contents.str();
        total_lines += std::count(s.begin(), s.end(), '\n');
    }
    assert(total_lines > 0); // overflow
    qDebug() << "Starting to merge files";
    // 5. Process the data in the files
    if (rem_dups) {
        ret << "\n# ------------- DEDUPLICATED AND SORTED ------------- \n";
        std::set<std::string> strset; // set does not store duplicates
        while (f_contents) {          // Process lines
            if (abort)
                return;
            if (add_stats)
                emit progress(int(strset.size() * 100 / total_lines));
            std::string line;
            std::getline(f_contents, line);
            auto pair = process_line(line);
            commented_lines += pair.first; // remove comments if needed and
                                           // increment the counter
            if (!pair.second.empty())
                strset.insert(pair.second);
        }
        total = strset.size(); // without duplicates
        for (const auto& s : strset)
            ret << s << '\n'; // save the changes, no duplicates
    } else {                  // don't remove duplicates
        std::vector<std::string> strvec;
        ret << "\n# ------------- MERGED ------------- \n";
        while (f_contents) { // Process lines
            if (abort)
                return;
            if (add_stats)
                emit progress(int(strvec.size() * 100 / total_lines));
            std::string line;
            std::getline(f_contents, line);
            auto pair = process_line(line);
            commented_lines += pair.first; // remove comments if needed and
                                           // increment the counter
            if (!pair.second.empty())
                strvec.push_back(pair.second);
        }
        total = strvec.size();
        for (const auto& s : strvec) {
            ret << s << '\n'; // write every line back
        }
    }
    f_contents.clear(); // reduce memory overhead
    qDebug() << "Finished merging files";
    // 6. Get remaining statistics
    if (add_stats) { // save calculation time by skipping
        emit        message("Getting your statistics...");
        Stats       st;
        char        symbol = 0;
        std::string opt;
        emit        progress(10);
        symbol = '#';
        st.comments += std::count(std::istreambuf_iterator<char>(ret),
                                  std::istreambuf_iterator<char>(), symbol);
        emit progress(50);
        if (abort)
            return;
        symbol = '\n';
        st.lines += std::count(std::istreambuf_iterator<char>(ret),
                               std::istreambuf_iterator<char>(), symbol);
        if (abort)
            return;
        emit progress(90);
#ifdef TIME_MULTIPLIER
        stats.seconds_added = stats.lines / TIME_MULTIPLIER;
#endif
        qDebug() << st.lines << " total lines in the resulting file";
        st.removed = st.lines - total; // total does not include duplicates
        st.sources =
            dl_mgr.get_total() + filepaths.size() + 1; // 1 for custom lines
        st.size = fs::file_size(path);                 // in bytes
        emit stats(st);
    }
    emit message("Done!");
    emit progress(100);
    emit success();
} catch (const std::exception& e) {
    auto msg = QString::fromStdString(e.what());
    qDebug() << msg;
    emit failure(msg);
    stop();
    return;
}

auto Slave::process_line(std::string line) const
    -> std::pair<bool, std::string> {
    if (rem_comments) { // remove comments
        auto it = line.find('#');
        if (it != std::string::npos) {
            line.erase(it);
            if (std::all_of(line.begin(), line.end(),
                            isspace)) // Check if only whitespace remains
                line.clear();
            return std::make_pair(true, line);
        }
    }
    return std::make_pair(false, line);
}
