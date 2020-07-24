#pragma once
#include "./ui_mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QFileDialog>
#include <QInputDialog>
#include <QListWidget>
#include <QListWidgetItem>
#include <QMainWindow>
#include <QMessageBox>
#include <QTranslator>
#include <QtNetwork>
#include <cassert>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <string>
#include <vector>
#ifdef __linux__
#define HOSTS "/etc/hosts"
#endif
#ifdef _WIN32
#define TIME_MULTIPLIER 400
#define HOSTS           "C:/Windows/System32/drivers/etc/hosts"
#endif
#define VERSION            "3.0.0"
#define SOCKET_RETRY_TIMES 5
#define CONFIG_FNAME       "custom.txt"
#define FILEERRORMSG                                                                                                   \
    "Couldn't process your file! Select another location or launch the app "                                           \
    "with admin privileges"
#define DL_FOLDER "download/"
#define CREDITS                                                                                                        \
    "# This file was generated with HostsToolkit: "                                                                    \
    "https://github.com/Nek-12/HostsToolkit \n";

QT_BEGIN_NAMESPACE
class QSslError;
QT_END_NAMESPACE

class DownloadManager : public QObject {
    Q_OBJECT
    QNetworkAccessManager    manager;
    QVector<QNetworkReply *> cur_downloads;

public:
    DownloadManager();
    void do_download(const QUrl &url);
    bool check_url(const QUrl &url);
    [[nodiscard]] bool finished() const {return is_finished;}
private:
    static QString get_filename(const QUrl &url);
    static bool    save_file(const QString &filename, QIODevice *data);
    static bool    is_redirected(QNetworkReply *reply);
    bool is_finished = false;
public slots:
    void        on_download_finished(QNetworkReply *reply);
    static void ssl_errors(const QList<QSslError> &errors);
};

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(const MainWindow &src) = delete;
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;
    std::string prepare_file();
    void        load_file(const std::string &path);
    void        load_custom();
    void        msg(const QString &msg);

public slots:
    void sys_load();
    void update_stats();
    void apply();
    void upd_progress_bar(int);
    void save_to();
    void append_entry();
    void open_file();
    void del_selected_list_entry();
    void display_about();
    void delete_selected_file();
    void add_url();
signals:
    void updated();
    void progress(int);

private:
    void                           closeEvent(QCloseEvent *bar) override;
    bool                           process_line_ip(std::string &); // TODO: Change to return a  pair instead
    std::vector<std::string>       files;
    std::vector<QListWidgetItem *> filepaths;   //->
    std::vector<QListWidgetItem *> customlines; // Maybe unneeded, but the implementation is ugly.
    std::vector<QListWidgetItem *> urls;        //-> Consider removing to save memory.
    bool                           sys_loaded = false;
    bool                           pending    = false;
    Ui::MainWindow *               ui;
    qulonglong                     total_lines = 1;
    DownloadManager                dl_mgr      = DownloadManager();
};

#include "network.moc"
