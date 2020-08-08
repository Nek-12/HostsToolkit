#pragma once
#include "src/const.h"
#include "src/network.h"
#include <QThread>
#include <filesystem>
#include <fstream>
#include <qmutex.h>
#include <qprogressdialog.h>
#include <qurl.h>
#include <qwaitcondition.h>
#include <set>

struct Stats {
    qulonglong lines         = 0;
    qulonglong size          = 0; // In bytes
    qulonglong comments      = 0;
    qulonglong sources       = 0;
    qulonglong seconds_added = 0;
    qulonglong removed       = 0;
};
//! Qt containers are slower than std.

// Manages threads and stores data needed for them to run
class Engine : public QObject {
    Q_OBJECT
public:
    explicit Engine(QObject* parent);
    // Starts working. Once finishes, emits ready(data)
    void start_work(const std::string& path, bool rem_comments, bool rem_dups,
                    bool add_credits, bool add_stats);
    void add_custom(const std::string& item);
    void add_file(const std::string& item);
    void add_url(const QUrl& item);
    void rem_custom(size_t row);
    void rem_file(size_t row);
    void rem_url(size_t row);
    bool save_entries(const std::string& path);
    ~Engine() override;
    // Return the number of sources loaded, counting all custom as one
    [[nodiscard]] size_t sources() const {
        return urls.size() + filepaths.size() + custom_lines.empty();
    }
    [[nodiscard]] bool is_pending() const { return pending; }
    [[nodiscard]] bool busy() const { return working; }
public slots:
    void stop(); //immediately abort the work
    void all_dls_finished();
signals:
    void message(const QString&);    // signals any messages
    void progress(int);              // signals the percentage of work done
    void success();                  // finished work
    void failed(const QString& msg); // couldn't finish
    void stats_ready(Stats);
    void state_updated(); //-> are changes pending or not?

private:
    [[nodiscard]] std::pair<bool, std::string> process_line(std::string) const;
    bool                     working          = false; //is working
    bool                     abort            = false; //aborted
    bool pending = false; // changes are pending
    bool comms = false;
    bool dups = false;
    bool credits = false;
    bool stats = false;
    std::vector<std::string> filepaths;
    std::vector<std::string> custom_lines;
    std::vector<QUrl>        urls;
    DownloadManager          dl_mgr;
    std::string              path;
};
