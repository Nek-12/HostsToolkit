#include "engine.h"
#include "network.h"
#include <qdebug.h>
#include <qobject.h>
#include <qthread.h>
#include <qurl.h>
#include <stdexcept>
// TODO: Add translations
//TODO: Make stop() less dirty;

//          -------------ENGINE------------
void Engine::start_work(const std::string& hosts, bool rem_comments, bool rem_dups, bool add_credits,
                       bool add_stats) {
    slave = new Slave(this,rem_comments, rem_dups, add_credits, add_stats, filepaths,
                      custom_lines, urls);
    hosts_path = hosts;
    working    = true;
    pending = false;
    // connect the thread
    connect(slave, &Slave::success, this, &Engine::thread_success); // signal success
    connect(slave, &Slave::failure, this,
            &Engine::thread_failure); // signal failure
    connect(slave, &Slave::progress, this, &Engine::progress);
    connect(slave, &Slave::message, this, &Engine::message);
    connect(slave, &Slave::stats, this, &Engine::stats);
    qDebug() << "Starting engine's slave";
    slave->start();
    //forget about the slave for now
}

//TODO: Maybe the references can cause data loss.
void Engine::thread_success(const std::string& res) {
    slave->deleteLater();
    slave = nullptr;
    working = false;
    std::ofstream f(hosts_path);
    if (f) {
        f << res;
        emit ready();
        emit state_updated();
        return;
    }
    emit failed("Finished the file, but couldn't write it");
}
void Engine::thread_failure(const QString& msg) {
    slave->deleteLater();
    working = false;
    slave = nullptr;
    emit failed(msg);
}

Engine::~Engine() {
    stop();
}

void Engine::stop() {
    if (slave) {
        slave->stop();
        slave->wait();
        slave->deleteLater();
        slave = nullptr;
        working = false;
    }
}

//TODO: Make one function and take parameters instead
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
        f << SPLIT_CHAR << " CUSTOM\n";
        for (const auto& el : custom_lines)
            f << el << '\n';
        f << SPLIT_CHAR << " FILES\n";
        for (const auto& el : filepaths)
            f << el << '\n';
        f << SPLIT_CHAR << " URLS\n";
        for (const auto& el : urls)
            f << el.toEncoded().toStdString() << '\n';
}

//              -----------SLAVE-----------
Slave::Slave(QObject* parent, bool rem_comments, bool rem_dups,
             bool add_credits, bool add_stats,
             std::vector<std::string> filepaths,
             std::vector<std::string> custom_lines, std::vector<QUrl> urls) : QThread(parent), dl_mgr(this),
    rem_comments(rem_comments),
    rem_dups(rem_dups), add_credits(add_credits), add_stats(add_stats),
    filepaths(std::move(filepaths)), custom_lines(std::move(custom_lines)),
    urls(std::move(urls)) {
    qDebug() << "Created new Slave";
    connect(this, &Slave::request_dl, &dl_mgr, &DownloadManager::do_download);
    connect(&dl_mgr, &DownloadManager::all_finished, this,
            &Slave::all_dls_finished);
}

void Slave::run() {
    // process urls
    double i = 0;
    emit message("Downloading files...");
    for (const auto& url : urls) {
        ++i;
        emit progress(int(i / urls.size() * 100));
        emit request_dl(url);
    }
}

void Slave::all_dls_finished() try {
    QString msg = "Downloads finished, processing files... \n";
    qDebug() << msg;
    emit message(msg);
    std::stringstream ret;
    double i = 0;
    qulonglong        commented_lines = 0, total = 0;
    if (add_credits)
        ret << CREDITS;
    ret << "\n# ------------- C U S T O M ------------- \n";
    for (const auto& l : custom_lines)
        ret << l << '\n'; // Add custom
    // process downloaded files
    i  = 0;
    std::stringstream contents;
    for (const auto& fname : filepaths) {
        std::ifstream f(fname);
        if (f) {
            contents << f.rdbuf();
        } else {
            throw std::runtime_error(
                std::string("Couldn't open file:").append(fname));
        }
    }
    auto fname = DL_FOLDER+std::string("hosts_").append(std::to_string(int(i)));
    while (std::filesystem::exists(fname)) {
        // check all files that were saved
        qDebug() << "Found file " << QString::fromStdString(fname);
        if (abort)
            return;
        std::ifstream f(fname);
        if (f) {
            qDebug() << "Opened file " << QString::fromStdString(fname);
            std::stringstream stream;
            contents << f.rdbuf();
            qDebug() << "Added the file " << QString::fromStdString(fname) << '\n';
        } else {
            throw std::runtime_error(
                std::string("Couldn't open file:").append(fname));
        }
        fname = std::string("hosts_").append(std::to_string(++i));
    }
    // TODO: Check for overflow!
    size_t total_lines = 1;
    if (add_stats) {
        std::string str = ret.str(); //Introduces significant overhead, but no other choice
        total_lines = count(str.begin(), str.end(), '\n');
    }
    qDebug() << "Starting to merge files";
    if (rem_dups) {
        ret << "\n# ------------- DEDUPLICATED AND SORTED ------------- \n";
        std::set<std::string> strset;
        while (contents) { // Process lines
            if (abort)
                return;
            if (add_stats)
                emit   progress(int(double(strset.size()) / total_lines * 100));
            std::string line;
            std::getline(contents, line);
            auto pair = process_line(line);
            commented_lines += pair.first; // remove comments if needed and increment the counter
            if (!pair.second.empty())
                strset.insert(pair.second); // set does not store duplicates
        }
        total = strset.size();
        for (const auto &s : strset)
            ret << s << '\n'; // write every line back, no duplicates
    } else { // don't remove duplicates
        std::vector<std::string> strvec;
        ret << "\n# ------------- MERGED ------------- \n";
        while (contents) { // Process lines
            if (abort)
                return;
            if (add_stats)
            emit progress(int(strvec.size() / double(total_lines) * 100));
            std::string line;
            std::getline(contents, line);
            auto pair = process_line(line);
            commented_lines += pair.first; // remove comments if needed and increment the counter
            if (!pair.second.empty())
                strvec.push_back(pair.second);
        }
        total = strvec.size();
        for (const auto &s : strvec) {
            ret << s << '\n'; // write every line back
        }
    }
    contents.clear();
    // start getting remaining statistics
    if (add_stats) {
        emit message("Getting your statistics...");
        Stats       st;
        char        symbol = 0;
        std::string opt;
        i        = 0;
        auto str = ret.str();
        emit progress(10);
        symbol = '#';
        st.comments += std::count(str.begin(), str.end(), symbol);
        emit progress(50);
        if (abort)
            return;
        symbol = '\n';
        st.lines += std::count(str.begin(), str.end(), symbol);
        if (abort)
            return;
        emit progress(100);
#ifdef TIME_MULTIPLIER
        stats.seconds_added = stats.lines / TIME_MULTIPLIER;
#endif
        qDebug() << st.lines;
        st.removed = st.lines - total;
        st.sources = urls.size() + filepaths.size() + 1; // 1 for custom lines
        st.size = str.size()*sizeof(char); //in bytes
        emit stats(st);
    }
    emit message("Done processing!");
    emit progress(100);
    emit success(ret.str());
} catch (const std::exception& e) {
    auto msg = QString::fromStdString(e.what());
    qDebug() << msg;
    emit failure(msg);
    stop();
    return;
}

auto Slave::process_line(std::string line) const -> std::pair<bool,std::string> {
    if (rem_comments) { // remove comments
        auto it = line.find('#');
        if (it != std::string::npos) {
            line.erase(it);
            if (std::all_of(line.begin(), line.end(),
                            isspace)) // Check if only whitespace remains
                line.clear();
            return std::make_pair(true,line);
        }
    }
    return std::make_pair(false,line);
}
