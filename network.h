#pragma once
#include <QtNetwork>
#include <QNetworkAccessManager>
#include <filesystem>
#define DL_FOLDER "download/"
QT_BEGIN_NAMESPACE
class QSslError;
QT_END_NAMESPACE

class DownloadManager : public QObject {
    Q_OBJECT
    QNetworkAccessManager*    manager;
    QVector<QNetworkReply*>  cur_downloads;
public:
    explicit DownloadManager(QObject* parent);
    ~DownloadManager() override;
    [[nodiscard]] static  bool check_url(const QUrl &url);
    [[nodiscard]] bool         finished() const { return is_finished; }
    void stop();
private:
    static QString get_filename(const QUrl &url);
    static bool    save_file(const QString &filename, QIODevice *data);
    static bool    is_redirected(QNetworkReply *reply);
    bool is_finished = false;
public slots:
    void        do_download(const QUrl &url);
    void        on_download_finished(QNetworkReply *reply);
    static void ssl_errors(const QList<QSslError> &errors);
signals:
    void dl_finished(QUrl);
    void all_finished();
};
