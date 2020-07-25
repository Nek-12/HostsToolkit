#include "src/engine.h"
#include <qglobal.h>
#include <qurl.h>
#include <fstream>
#include <set>
// TODO: Add translations
//TODO: Make stop() less dirty;

// The most important function. Does all the heavy stuff and returns complete
// hosts file to apply
void Engine::start_work(const std::string& hosts, bool rem_comments, bool rem_dups, bool add_credits,
                       bool add_stats) {
    slave = new Slave(rem_comments, rem_dups, add_credits, add_stats, filepaths,
                      custom_lines, urls);
    hosts_path = hosts;
    working = true;
    // connect the thread
    connect(slave, &Slave::success, this, &Engine::thread_success); // signal success
    connect(slave, &Slave::failure, this,
            &Engine::thread_failure); // signal failure
    connect(slave, &Slave::progress, this, &Engine::progress);
    connect(slave, &Slave::message, this, &Engine::message);
    connect(slave, &Slave::stats, this, &Engine::stats);
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
        return;
    }
    emit failed();
}
void Engine::thread_failure() {
    slave->deleteLater();
    working = false;
    slave = nullptr;
    emit failed();
}

void Slave::run() {
    std::stringstream ret;
    qulonglong commented_lines = 0, total = 0;
    if (add_credits)
        ret << CREDITS;
    ret << "\n# ------------- C U S T O M ------------- \n";
    for (const auto &l : custom_lines)
        ret << l << '\n'; // Add custom
    // process urls
    double i = 0;
    for (const auto& url : urls) {
        ++i;
        emit message("Downloading files...");
        emit progress(int(i/urls.size()*100));
        dl_mgr.do_download(url);
    }
    // wait for the downloads
    // TODO: Hacky. Maybe implement some signals?
    while (!dl_mgr.finished()) {
        if (abort) {
            dl_mgr.stop();
            return;
        }
    }
    // process downloaded files
    QString msg = "Downloads finished, processing files... \n";
    qDebug() << msg;
    emit message(msg);
    i  = 0;
    auto fname = QString("hosts_%1").arg(i);
    while (QFile::exists(fname)) {
        // check all files that were saved
        if (abort)
            return;
        std::ifstream f(fname.toStdString());
        if (f) {
            std::stringstream stream;
            ret << f.rdbuf();
            qDebug() << "Added the file " << fname << '\n';
        } else {
            qDebug() << "File exists but can't be opened!\n";
        }
        fname = QString("hosts_%1").arg(++i);
    }
    // TODO: Check for overflow!
    size_t total_lines = 1;
    if (add_stats) {
        std::string str = ret.str(); //Introduces significant overhead, but no other choice
        total_lines = count(str.begin(), str.end(), '\n');
    }
    if (rem_dups) {
        ret << "\n# ------------- DEDUPLICATED AND SORTED ------------- \n";
        std::set<std::string> strset;
        while (ret) { // Process lines
            if (abort)
                return;
            if (add_stats)
                emit   progress(int(double(strset.size()) / total_lines * 100));
            std::string line;
            std::getline(ret, line);
            auto pair = process_line(line);
            commented_lines += pair.first; // remove comments if needed and increment the counter
            if (pair.second.empty()) continue;
            strset.insert(pair.second); // set does not store duplicates
        }
        ret.clear();
        total = strset.size();
        for (const auto &s : strset)
            ret << s << '\n'; // write every line back, no duplicates
    } else { // don't remove duplicates
        std::vector<std::string> strvec;
        ret << "\n# ------------- MERGED ------------- \n";
        while (ret) { // Process lines
            if (abort)
                return;
            if (add_stats)
            emit progress(int(strvec.size() / double(total_lines) * 100));
            std::string line;
            std::getline(ret, line);
            auto pair = process_line(line);
            commented_lines += pair.first; //remove comments on demand
            if (line.empty())
                continue;
            strvec.push_back(pair.second);
        }
        ret.clear();
        total = strvec.size();
        for (const auto &s : strvec) {
            ret << s << '\n'; // write every line back
        }
    }
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

Engine::~Engine() {
    if (slave) {
        slave->stop();
        slave->deleteLater();
    }
}

void Engine::stop() {
    if (slave) {
        slave->stop();
        slave->deleteLater();
        slave = nullptr;
        working = false;
    }
}
