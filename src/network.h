#pragma once
#include <QtNetwork>
#define DL_FOLDER "download/"
#include <filesystem>

QT_BEGIN_NAMESPACE
class QSslError;
QT_END_NAMESPACE

class DownloadManager : public QObject {
    Q_OBJECT
    QNetworkAccessManager    manager;
    QVector<QNetworkReply*> cur_downloads;

public:
    DownloadManager();
    void do_download(const QUrl &url);
    bool check_url(const QUrl &url, const unsigned);
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

#include "network.moc"
