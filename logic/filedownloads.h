#ifndef FILEDOWNLOADS_H
#define FILEDOWNLOADS_H
#include "calculation.h"
#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QFile>
#include <QProcess>

class filedownloads: public QObject
{
    Q_OBJECT
    calculation calc;
    QString destination;
    QString outputFolder;
    QProcess* process;

private slots:
    void downloadComplete(QNetworkReply*);
    QString findReleaseUrl(QString, QString);
public slots:
    void accessKallistoBustoolsInfo(QString, QString, QString);
signals:
    void downloadFinished();        //notifies that the files have been successfully downloaded
    void downloadFailed(QString);   //notifies that something went wrong with the file download
    void generalError(QString);
    void pathSuccess(QString, QString);
    void releaseVersionSuccess();
public:
    filedownloads();
    void doDownload(const QUrl &url);
    bool downloadRelease(QString);
    void getData(const QUrl &url, QObject* parent);
    void getVersion(QString, QString);
    QString getDestinationFile();
    QNetworkAccessManager *manager;
    void setDestinationFile(QString url);
    void setOutputFolder(QString directory);
};

#endif // FILEDOWNLOADS_H
