#include "network.h"
#include "src/app.h"


DownloadManager::DownloadManager() {
    connect(&manager, &QNetworkAccessManager::finished, this, &DownloadManager::on_download_finished);
}

void DownloadManager::do_download(const QUrl &url) {
    QNetworkRequest request(url);
    QNetworkReply * reply = manager.get(request);
    is_finished = false;

#if QT_CONFIG(ssl)
    connect(reply, &QNetworkReply::sslErrors, this, &DownloadManager::ssl_errors);
#endif

    cur_downloads.append(reply);
}

QString DownloadManager::get_filename(const QUrl & /*url*/) {
    QString basename = "hosts";
    int     i        = 0;
    basename += '_';
    while (QFile::exists(basename + QString::number(i)))
        ++i;
    basename += QString::number(i);
    return basename;
}

bool DownloadManager::save_file(const QString &filename, QIODevice *data) {
    if (!std::filesystem::is_directory(DL_FOLDER) || !std::filesystem::exists(DL_FOLDER)) {
        std::filesystem::create_directory(DL_FOLDER);
    }
    QFile file(DL_FOLDER + filename);
    if (!file.open(QIODevice::ReadWrite)) {
        qDebug() << "Couldn't open " << filename << "for writing: " << file.errorString() << '\n';
        return false;
    }

    file.write(data->readAll());
    file.close();

    return true;
}

bool DownloadManager::is_redirected(QNetworkReply *reply) {
    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303 || statusCode == 305 || statusCode == 307 ||
           statusCode == 308;
}

void DownloadManager::ssl_errors(const QList<QSslError> &sslErrors) {
#if QT_CONFIG(ssl)
    for (const QSslError &error : sslErrors)
        qDebug() << "SSL error: " << error.errorString() << '\n';
#else
    Q_UNUSED(sslErrors);
#endif
}

void DownloadManager::on_download_finished(QNetworkReply *reply) {
    QUrl url = reply->url();
    if (reply->error()) {
        qDebug() << "Download of: " << url.toEncoded().constData() << "failed: " << qPrintable(reply->errorString());
    } else {
        if (is_redirected(reply)) {
            qDebug() << "Request was redirected.\n";
        } else {
            QString filename = get_filename(url);
            if (save_file(filename, reply)) {
                qDebug() << "Download of " << url.toEncoded().constData() << " succeeded (saved to) \n"
                         << qPrintable(filename);
            }
        }
    }

    cur_downloads.removeAll(reply);
    reply->deleteLater();

    if (cur_downloads.isEmpty()) {
        is_finished = true;
    }
}

bool DownloadManager::check_url(const QUrl &url, const unsigned retry_times = 3) {
    for (unsigned i = 0; i != retry_times; ++i) { // try several times
        QTcpSocket socket;
        QByteArray buffer;
        socket.connectToHost(url.host(), 80);
        if (socket.waitForConnected()) {
            // Standard http request
            socket.write("GET / HTTP/1.1\r\n"
                         "host: " +
                         url.host().toUtf8() + "\r\n\r\n");
            if (socket.waitForReadyRead()) {
                while (socket.bytesAvailable()) {
                    buffer.append(socket.readAll());
                    int packetSize = buffer.size();
                    while (packetSize > 0) {
                        // Output server response for debugging
                        qDebug() << "[" << buffer.data() << "]";
                        // set Url if 200, 301, or 302 response given assuming that server
                        // will redirect
                        if (buffer.contains("200 OK") || buffer.contains("302 Found") || buffer.contains("301 Moved")) {
                            return true;
                        }
                        buffer.remove(0, packetSize);
                        packetSize = buffer.size();

                    } // while packet size >0
                } // while socket.bytes
            } // socket wait for ready read
        } // socket write
    } // after all trials failed
    return false;
}
