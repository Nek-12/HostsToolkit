/* Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
** This file is based on the Download Manager example of the Qt Toolkit.
*/
#pragma once
#include <QTimer>
#include <QtCore>
#include <QtNetwork>
#include <filesystem>
#include "src/const.h"

bool check_url(const QUrl &url);

class DownloadManager : public QObject {
    Q_OBJECT
public:
    explicit DownloadManager(QObject *parent = nullptr); //parent allows us to not worry about memory management
    static QString     get_filename(const QUrl &url);//determines the next available filename
    void                      append(const QUrl &url); //adds a download, but does not start it
    [[nodiscard]] bool        is_idle() const { return totalCount == downloadedCount; }
    [[nodiscard]] int         get_downloaded() const { return downloadedCount; }
    [[nodiscard]] int         get_total() const { return totalCount; }
    [[nodiscard]] QString     get_speed() const { return dl_speed; }

signals:
    void dl_finished(QUrl);
    void all_finished();
    void dl_failed(QString msg);
    void progress(int percentage);
    void message(QString);

public slots:
    void go() const; // start all downloads
    void stop(); //abort all downloads
    void on_dl_finished(); //when everything about one download is done
    void on_dl_data_ready(); //when we are ready to write the file
    void dl_progress(qint64 bytesReceived, qint64 bytesTotal); //when our manager reports any progress

private:
    void start_next_dl();
    [[nodiscard]] bool isHttpRedirect() const;
    void               handle_redirect();

    QNetworkAccessManager mgr;
    QQueue<QUrl>          downloadQueue;
    QNetworkReply *       cur_dl = nullptr;
    QFile                 output;
    QElapsedTimer         downloadTimer;

    int  downloadedCount = 0;
    int  totalCount      = 0;
    QString dl_speed;
};
