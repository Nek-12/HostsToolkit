#include "network.h"
#include <QThread>
#include <utility>
#include <pthread.h>
#include <qlistwidget.h>
#include <qobjectdefs.h>
class QListWidgetItem;
// NOLINTNEXTLINE
#define CREDITS                                                                \
    "# This file was generated with HostsToolkit: "                            \
    "https://github.com/Nek-12/HostsTools \n";
// TODO: Resize the string before writing!
// TODO: Don't forget to clear the variables to not increase the stack size
// TODO: Remember to never flush the bufer
// TODO:
struct Stats {
    qulonglong lines         = 0;
    qulonglong size          = 0; //In bytes
    qulonglong comments      = 0;
    qulonglong sources       = 0;
    qulonglong seconds_added = 0;
    qulonglong removed = 0;
};

//Creates and parses the hosts files for food and shelter
class Slave : public QThread {
    Q_OBJECT
public:
    // Copies the data to avoid sharing
    Slave(bool rem_comments, bool rem_dups, bool add_credits, bool add_stats,
          std::vector<std::string> filepaths,
          std::vector<std::string> custom_lines, std::vector<QUrl> urls)
        : rem_comments(rem_comments), rem_dups(rem_dups),
          add_credits(add_credits), add_stats(add_stats),
          filepaths(std::move(filepaths)), custom_lines(std::move(custom_lines)), urls(std::move(urls)) {}
    //Starts working on the data and then exits (don't forget to delete)
    void run() override;

signals:
    void success(std::string);
    void stats(Stats);
    void failure();
    void progress(int);    // signals the percentage of work done
    void message(QString); // signals any messages

private:
    [[nodiscard]] std::pair<bool,std::string> process_line(std::string ) const;
    bool rem_comments, rem_dups, add_credits, add_stats;
    std::vector<std::string> filepaths, custom_lines;
    std::vector<QUrl>        urls;
    DownloadManager dl_mgr;
};

// Manages threads and stores data needed for them to run
class Engine : public QObject {
    Q_OBJECT
public:
    Engine() = default;
    //Starts working. Once finishes, emits ready(data)
    void start_work(bool rem_comments, bool rem_dups,
                                  bool add_credits, bool add_stats);

    void add_custom(QListWidgetItem *item) { custom_lines.push_back(item); }
    void add_file(QListWidgetItem *item) { filepaths.push_back(item); }
    void add_url(QListWidgetItem *item) { urls.push_back(item); }
    void rem_custom(size_t row) {
        custom_lines.erase(custom_lines.begin() + row);
    }
    void rem_file(size_t row) { filepaths.erase(filepaths.begin() + row); }
    void rem_url(size_t row) { urls.erase(urls.begin() + row); }
    ~Engine() override;

public slots:
    //get results from the slave
    void thread_success(const std::string&, const Stats& );
    void thread_failure();

signals:
    void ready(std::string, Stats); // finished work
    void failed(); //couldn't finish
    void stats(Stats);
    void progress(int);             // signals the percentage of work done
    void message(QString);          // signals any messages

private:
    Slave* slave = nullptr;
    std::vector<QListWidgetItem *> filepaths;
    std::vector<QListWidgetItem *> custom_lines;
    std::vector<QListWidgetItem *> urls;
};
