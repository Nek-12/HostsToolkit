/*
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
** This file is based on the Download Manager example of the Qt Toolkit.
*/
#include "src/network.h"
#include <filesystem>
//
bool check_url(const QUrl &url) {
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
                    // set Url if 200, 301, or 302 response given assuming that
                    // server will redirect
                    if (buffer.contains("200 OK") ||
                        buffer.contains("302 Found") ||
                        buffer.contains("301 Moved")) {
                        return true;
                    }
                    buffer.remove(0, packetSize);
                    packetSize = buffer.size();
                } // while packet size >0
            }     // while socket.bytes
        }         // socket wait for ready read
    }             // socket write
    return false;
}

DownloadManager::DownloadManager(QObject* parent) : QObject(parent) {
    qDebug() << "Created download manager";
}

void DownloadManager::append(const QUrl& url) {
    downloadQueue.enqueue(url); // add to queue
    ++totalCount;
    qDebug() << "Added to queue: " << url;
}

void DownloadManager::go() const {
    QTimer::singleShot(0, this, &DownloadManager::start_next_dl);
    // try to start the new download ASAP when the timer is called
    qDebug() << "Started download timer";
}

void DownloadManager::start_next_dl() {
    auto msg = QString("%1/%2 files downloaded successfully")
                   .arg(downloadedCount)
                   .arg(totalCount);
    qDebug() << msg;
    emit message(msg);
    if (downloadQueue.isEmpty()) {
        emit all_finished(); // nothing to do
        return;
    }
    QUrl    url      = downloadQueue.dequeue(); // else get the next item
    QString filename = get_filename(url);       // get the future name
    output.setFileName(filename);               // remember filename
    if (!output.open(QIODevice::WriteOnly)) {   // if not writable
        auto s = QString("Problem opening file '%1' for download '%2': %3")
                     .arg(qPrintable(filename))
                     .arg(url.toEncoded().constData())
                     .arg(qPrintable(output.errorString()));
        qDebug() << s;
        emit dl_failed(s);
        start_next_dl(); // remember to start the next download asap
        return;          // skip this download
    }
    QNetworkRequest request(url);
    cur_dl = mgr.get(request); // begin the download
    connect(cur_dl, &QNetworkReply::downloadProgress, this,
            &DownloadManager::dl_progress); // receive progress reports
    connect(cur_dl, &QNetworkReply::finished, this,
            &DownloadManager::on_dl_finished); // receive download reports (no
                                               // matter failed or not)
    connect(cur_dl, &QNetworkReply::readyRead, this,
            &DownloadManager::on_dl_data_ready); // receive data reports

    auto s = QString("Downloading %1...").arg(url.toEncoded().constData());
    emit message(s);
    qDebug() << s; // report some data
    downloadTimer
        .start(); // start this download and then try to download the next ASAP
}

// when our manager reports progress
void DownloadManager::dl_progress(qint64 bytesReceived, qint64 bytesTotal) {
    if (bytesTotal >
        0) { //! bytestotal is -1 if we can't get the total size, handle it
        int pr = int(bytesReceived * 100 / bytesTotal);
        assert(pr >= 0);
        emit progress(pr); // report it
    }
    // calculate the download speed
    double  speed = bytesReceived * 1000.0 / downloadTimer.elapsed();
    QString unit;
    if (speed < 1024) {
        unit = "B/s"; // bytes per second
    } else if (speed < 1024 * 1024) {
        speed /= 1024;
        unit = "kB/s";
    } else {
        speed /= 1024 * 1024;
        unit = "MB/s";
    }
    // save the last speed in case someone asks for it
    dl_speed = QString::fromLatin1("%1 %2").arg(speed, 3, 'f', 1).arg(unit);
    report_speed(dl_speed);
}

// when download ends (no matter failed or not)
void DownloadManager::on_dl_finished() {
    emit progress(100);
    dl_speed.clear();
    output.close(); // at this point we have already either written to the file
                    // or failed.

    if (cur_dl->error()) {
        // download failed
        emit dl_failed(qPrintable(cur_dl->errorString()));
        stop();
    } else {
        // check if it was actually a redirect
        if (isHttpRedirect()) {
            handle_redirect(); // if valid, we'll download it later.
            output.remove();   // for now forget about it
        } else {
            message("Succeeded.");
            ++downloadedCount;
            emit dl_finished(cur_dl->url());
        }
    }
    cur_dl->deleteLater(); // throw it away
    cur_dl = nullptr;
    start_next_dl();       // start the next download
}

// when the data has been received (we're not finished yet, possibly)
void DownloadManager::on_dl_data_ready() {
    output.write(cur_dl->readAll()); // save it
}

void DownloadManager::handle_redirect() {
    // report some data ->
    int statusCode =
        cur_dl->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    QUrl requestUrl = cur_dl->request().url();
    qDebug() << "Request: " << requestUrl.toDisplayString()
             << " was redirected with code: " << statusCode << '\n';
    // <-
    QVariant target = cur_dl->attribute(
        QNetworkRequest::RedirectionTargetAttribute); // get the target
    if (!target.isValid())
        return;                        // if it's some crap ignore it
    QUrl redirectUrl = target.toUrl(); // else get the new url
    if (redirectUrl.isRelative())
        redirectUrl = requestUrl.resolved(redirectUrl); // get the normalized
                                                        // url
    qDebug() << "Redirected to: " << redirectUrl.toDisplayString();
    append(redirectUrl); // try to download it then.
    --totalCount;
}
// append numbers until the filename is available and return it
QString DownloadManager::get_filename(const QUrl & /*url*/) {
    QString basename               = DL_FOLDER DOWNLOADED_HOSTS_PREFIX;
    int                          i = 0;
    while (QFile::exists(basename + QString::number(i)))
        ++i;
    basename += QString::number(i);
    return basename;
}

bool DownloadManager::isHttpRedirect() const {
    int statusCode =
        cur_dl->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    return statusCode == 301 || statusCode == 302 || statusCode == 303 ||
           statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void DownloadManager::stop() {
    if (cur_dl)
        cur_dl->abort(); //! emits finished() on call;
    // starts a recursive chain reaction of slots and signals
    // which ends with all_dls_finished with abort set to true.
    downloadQueue.clear();
    downloadedCount = 0;
    totalCount      = 0;
}
