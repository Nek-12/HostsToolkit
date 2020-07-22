#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "main.h"
#include <QtNetwork>
#include <qdebug.h>

QT_BEGIN_NAMESPACE
class QSslError;
QT_END_NAMESPACE

class DownloadManager : public QObject {
  Q_OBJECT
  QNetworkAccessManager manager;
  QVector<QNetworkReply *> cur_downloads;

public:
  DownloadManager();
  void doDownload(const QUrl &url);
  static QString saveFileName(const QUrl &url);
  static bool saveToDisk(const QString &filename, QIODevice *data);
  static bool isHttpRedirect(QNetworkReply *reply);

public slots:
  // TODO: Define the slot for downloading
  //     QUrl url = QUrl::fromEncoded(arg.toLocal8Bit());
  //     doDownload(url);

  void downloadFinished(QNetworkReply *reply);
  static void sslErrors(const QList<QSslError> &errors);
};

DownloadManager::DownloadManager() {
  connect(&manager, &QNetworkAccessManager::finished, this,
          &DownloadManager::downloadFinished);
}

void DownloadManager::doDownload(const QUrl &url) {
  QNetworkRequest request(url);
  QNetworkReply* reply = manager.get(request);

#if QT_CONFIG(ssl)
  connect(reply, &QNetworkReply::sslErrors, this, &DownloadManager::sslErrors);
#endif

  cur_downloads.append(reply);
}

QString DownloadManager::saveFileName(const QUrl &url) {
  QString path = url.path();
  QString basename = QFileInfo(path).fileName();

  if (basename.isEmpty())
    basename = "hosts";

  if (QFile::exists(basename)) {
    // already exists, don't overwrite
    int i = 0;
    basename += '_';
    while (QFile::exists(basename + QString::number(i)))
      ++i;

    basename += QString::number(i);
  }

  return basename;
}

bool DownloadManager::saveToDisk(const QString &filename, QIODevice *data) {
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly)) {
    qDebug() << "Couldn't open " << filename << "for writing: " << file.errorString()  << '\n';
    return false;
  }

  file.write(data->readAll());
  file.close();

  return true;
}

bool DownloadManager::isHttpRedirect(QNetworkReply *reply) {
  int statusCode =
      reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
  return statusCode == 301 || statusCode == 302 || statusCode == 303 ||
         statusCode == 305 || statusCode == 307 || statusCode == 308;
}

void DownloadManager::sslErrors(const QList<QSslError> &sslErrors) {
#if QT_CONFIG(ssl)
  for (const QSslError &error : sslErrors)
    qDebug() << "SSL error: " << error.errorString() << '\n';
#else
  Q_UNUSED(sslErrors);
#endif
}

void DownloadManager::downloadFinished(QNetworkReply *reply) {
  QUrl url = reply->url();
  if (reply->error()) {
    qDebug() << "Download of: " <<  url.toEncoded().constData() << "failed: " <<
            qPrintable(reply->errorString());
  } else {
    if (isHttpRedirect(reply)) {
      qDebug() << "Request was redirected.\n";
    } else {
      QString filename = saveFileName(url);
      if (saveToDisk(filename, reply)) {
        qDebug() << "Download of "
                 << url.toEncoded().constData()
                 << " succeeded (saved to) \n" << qPrintable(filename);
      }
    }
  }

  cur_downloads.removeAll(reply);
  reply->deleteLater();

  if (cur_downloads.isEmpty()) {
    //TODO: all downloads finished
    //Do something?
  }
}
#include "network.moc"
