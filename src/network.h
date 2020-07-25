#pragma once
#include <QtNetwork>
#include <qnetworkaccessmanager.h>
#include <qurl.h>
#define DL_FOLDER "download/"
#include <filesystem>

QT_BEGIN_NAMESPACE
class QSslError;
QT_END_NAMESPACE

class DownloadManager : public QObject {
    Q_OBJECT
    QNetworkAccessManager*    manager;
    QVector<QNetworkReply*>  cur_downloads;
public:
    DownloadManager();
    ~DownloadManager() override;
    void do_download(const QUrl &url);
    [[nodiscard]] static  bool check_url(const QUrl &url);
    [[nodiscard]] bool         finished() const { return is_finished; }
    void stop();
private:
    static QString get_filename(const QUrl &url);
    static bool    save_file(const QString &filename, QIODevice *data);
    static bool    is_redirected(QNetworkReply *reply);
    bool is_finished = false;
public slots:
    void        on_download_finished(QNetworkReply *reply);
    static void ssl_errors(const QList<QSslError> &errors);
signals:
    void dl_finished(QUrl);
};

#include "network.moc"
