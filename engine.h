#pragma once
#include "network.h"
#include <QThread>
#include <filesystem>
#include <fstream>
#include <qmutex.h>
#include <qurl.h>
#include <qwaitcondition.h>
#include <set>
// NOLINTNEXTLINE
#define SPLIT_CHAR '>'
#define CREDITS                                                                \
    "# This file was generated with HostsTools: "                            \
    "https://github.com/Nek-12/HostsTools \n"
// TODO: Comment code
// TODO: Add mutex or disallow saving file until the thread has finished!
// TODO: Handle failing downloads or too slow downloads
// TODO: Open a new progressbar window. The feedback right now is nonexistent
// TODO: Fix that free() error
// TODO: The entries from the config aren't being added for some reason. Why?
// TODO: Don't let exceptions propagate through QT code. Handle them/replace
// them with warnings
// TODO: Test the app with 10+ sources. Test performance

struct Stats {
    qulonglong lines         = 0;
    qulonglong size          = 0; // In bytes
    qulonglong comments      = 0;
    qulonglong sources       = 0;
    qulonglong seconds_added = 0;
    qulonglong removed       = 0;
};
// NOTE: Qt containers are slower than std.

// Creates and parses the hosts files for food and shelter
class Slave : public QThread {
    Q_OBJECT
public:
    // Copies the data to avoid sharing
    Slave(QObject* parent, bool rem_comments, bool rem_dups, bool add_credits, bool add_stats,
          std::vector<std::string> filepaths,
          std::vector<std::string> custom_lines, std::vector<QUrl> urls);
    // Starts working on the data and then exits (don't forget to delete)
    void run() override;
    void stop() { abort = true; dl_mgr.stop(); }
public slots:
    void all_dls_finished();

signals:
    void success(std::string);
    void stats(Stats);
    void failure(QString); //signals failure (thread exited without applying the file)
    void progress(int);    // signals the percentage of work done
    void message(QString); // signals any message
    void request_dl(QUrl); //signals when the thread is requesting some download

private:
    [[nodiscard]] std::pair<bool, std::string> process_line(std::string) const;
    DownloadManager                            dl_mgr;
    bool abort = false, rem_comments, rem_dups, add_credits, add_stats;
    std::vector<std::string> filepaths, custom_lines;
    std::vector<QUrl>        urls;
};

// Manages threads and stores data needed for them to run
class Engine : public QObject {
    Q_OBJECT
public:
    explicit Engine(QObject* parent): QObject(parent) {}
    // Starts working. Once finishes, emits ready(data)
    void start_work(const std::string& hosts, bool rem_comments, bool rem_dups,
                    bool add_credits, bool add_stats);

    void add_custom(const std::string& item);
    void add_file(const std::string& item);
    void add_url(const QUrl& item);
    void rem_custom(size_t row);
    void rem_file(size_t row);
    void rem_url(size_t row);
    void save_entries(std::ofstream& f);
    ~Engine() override;
    // Return the number of sources loaded, counting all custom as one
    [[nodiscard]] size_t sources() const {
        return urls.size() + filepaths.size() + custom_lines.empty();
    }
    [[nodiscard]] bool is_pending() const { return pending; }
public slots:
    // get results from the slave
    void thread_success(const std::string&);
    void thread_failure(const QString& msg);
    void stop();

signals:
    void ready();  // finished work
    void failed(const QString& msg); // couldn't finish
    void stats(Stats);
    void progress(int);    // signals the percentage of work done
    void message(QString); // signals any messages
    void state_updated();

private:
    Slave*                   slave   = nullptr;
    bool                     working = false;
    bool                     pending = false;
    std::vector<std::string> filepaths;
    std::vector<std::string> custom_lines;
    std::vector<QUrl>        urls;
    std::string              hosts_path;
};
