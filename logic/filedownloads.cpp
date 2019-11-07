#include "filedownloads.h"
#include "../ui/mainwindow.h"
#include <QStandardPaths>
#include <QDir>

/**
 * @brief filedownloads::filedownloads performs all of the downloading for the application
 */
filedownloads::filedownloads()
{
    manager = new QNetworkAccessManager(this);
}

/**
 * @brief filedownloads::accessKallistoBustoolsInfo the kallisto and bustools paths where not found in the database
 * or already installed, therefore the latest release will be obtained from the internet and saved on the users machine
 * @param kallistoReleasePath rest github server command to find kallisto latest release
 * @param bustoolsReleasePath rest github server command to find bustools latest release
 */
void filedownloads::accessKallistoBustoolsInfo(QString kallistoReleasePath, QString bustoolsReleasePath, QString operatingSystem) {
    QString kallistoReleaseVersion = this->findReleaseUrl(kallistoReleasePath, operatingSystem);
    QString bustoolsReleaseVersion = this->findReleaseUrl(bustoolsReleasePath, operatingSystem);

    this->getVersion(kallistoReleaseVersion, bustoolsReleaseVersion);

}

/**
 * @brief filedownloads::downloadComplete save the download result
 * @param response
 */
void filedownloads::downloadComplete(QNetworkReply* response) {
    if(response->error() == QNetworkReply::NoError) {
        QByteArray reply = response->readAll();
        if(reply.isEmpty()) {
            emit downloadFailed("Something went wrong, close application and try again");
        } else {
            QFile file(destination);
            file.open(QIODevice::WriteOnly);
            file.write(reply);

            file.close();

            emit downloadFinished();
        }
    } else {
        emit downloadFailed("Something went wrong when retrieving the data" + response->errorString());
    }
}

/**
 * @brief filedownloads::doDownload performs a request and accesses the file to be downloaded
 * and saved in a given directory
 * @param url request url
 * @param directory the users preferred destination for the result
 */
void filedownloads::doDownload(const QUrl &url) {
    QNetworkRequest request(url);

    connect(manager, SIGNAL(finished(QNetworkReply*)), SLOT(downloadComplete(QNetworkReply*)));
    manager->get(request);
}

/**
 * @brief filedownloads::downloadRelease after the release url has been found it is downloaded. The tar file is removed after installation
 * but the path to kallisto or bustools remains
 * @param path the path to the tar file
 * @return
 */
bool filedownloads::downloadRelease(QString path) {
    process->setProcessChannelMode(QProcess::MergedChannels);

    //accessible under the users home path
    QString filePath = QDir::homePath() + "/.kallistoscope/" + calc.findOutputField(path, false);
    process->start("curl", QStringList() << "-L" << "-o" << filePath << path);

    process->waitForFinished();

    QString result(process->readAllStandardOutput());
    QString errors(process->readAllStandardError());

    process->close();

    if(errors.isEmpty()) {
        QString dir = QDir::homePath() + "/.kallistoscope/" + calc.findOutputField(path, false);
        QString resultDirectory = QDir::homePath() + "/.kallistoscope/";
        dir = QDir::toNativeSeparators(dir);
        resultDirectory = QDir::toNativeSeparators(resultDirectory);
        process->start("tar", QStringList() << "-xvzf" << dir << "-C" << resultDirectory);

        process->waitForFinished();

        QString errors(process->readAllStandardError());

        process->close();

        if(errors.isEmpty()) {
            process->start("rm", QStringList() << "-rf" << dir);

            process->waitForFinished();
        } else {
            return false;
        }
        return true;
    } else {
        return false;
    }
}

/**
 * @brief filedownloads::findReleaseUrl access the latest release through github
 * @param releasePath the url to the github api that stores the latest release information
 * @return
 */
QString filedownloads::findReleaseUrl(QString releasePath, QString operatingSystem) {
    process = new QProcess(this);
    process->setProcessChannelMode(QProcess::MergedChannels);

    process->start("curl", QStringList() << releasePath);

    process->waitForFinished();

    QString path(process->readAllStandardOutput());
    process->close();

    QStringList resultStrings = path.split('\n');
    QString browserUrl;

    foreach(QString result, resultStrings) {
        if(result.contains("browser_download_url") && result.contains(operatingSystem)) {
           browserUrl = result;
        }
    }

    QString downloadPath = browserUrl.mid(31, browserUrl.length()-32);

    return downloadPath;
}

/**
 * @brief filedownloads::getData accesses information about research species from the rest service of ensembl.org
 * @param url the request url
 * @param parent the class that requests the information
 */
void filedownloads::getData(const QUrl &url, QObject* parent) {
    connect(manager, SIGNAL(finished(QNetworkReply*)), parent, SLOT(responseReady(QNetworkReply*)));
    manager->get(QNetworkRequest(QUrl(url)));
}

QString filedownloads::getDestinationFile() {
    return destination;
}

/**
 * @brief filedownloads::getVersion wrapper function for accessing the tar file, it signals the path to the executables if successful
 * @param kallistoPath
 * @param bustoolsPath
 */
void filedownloads::getVersion(QString kallistoPath, QString bustoolsPath) {
    if(!kallistoPath.isEmpty() && !bustoolsPath.isEmpty()) {
        emit releaseVersionSuccess();
    } else {
        emit generalError("Unable to fetch release versions, install software manually and restart the application");
        return;
    }
    bool kallistoResult = this->downloadRelease(kallistoPath);
    bool bustoolsResult = this->downloadRelease(bustoolsPath);

    if(kallistoResult && bustoolsResult) {
        QString kallistoLocation = QDir::homePath() + "/.kallistoscope/kallisto/kallisto";
        QString bustoolsLocation = QDir::homePath() + "/.kallistoscope/bustools/bustools";
        kallistoLocation = QDir::toNativeSeparators(kallistoLocation);
        bustoolsLocation = QDir::toNativeSeparators(bustoolsLocation);
        emit pathSuccess(kallistoLocation, bustoolsLocation);
    } else {
        emit generalError("Unable to fetch applications, install software manually and restart the application");
    }

    process->deleteLater();
}

/**
 * @brief filedownloads::setDestinationFile calculate the destination placement and start download
 * @param url
 */
void filedownloads::setDestinationFile(QString url) {
    QString filename = calc.findOutputField(url, false);
    destination = outputFolder + filename;

    this->doDownload(QUrl(url));
}

/**
 * @brief filedownloads::setOutputFolder setter for the directory for the calculation results
 * @param directory
 */
void filedownloads::setOutputFolder(QString directory) {
    outputFolder = directory;
}
