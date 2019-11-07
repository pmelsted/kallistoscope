#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QDebug>
#include <QString>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonArray>
#include <QFile>
#include <QMessageBox>
#include <QDir>
#include <QStandardPaths>

/**
 * @brief MainWindow::MainWindow the main window checkes whether kallisto and bustools are already in the database
 * or installed on the machine, otherwise it installs it for the user. Serves as a redirect to other ui components
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QString applicationFolder = QDir::homePath() + "/.kallistoscope";

    if(!QDir(applicationFolder).exists()) {
        QString database = applicationFolder + "/application_information.sqlite";
        applicationFolder = QDir::toNativeSeparators(applicationFolder);
        database = QDir::toNativeSeparators(database);
        QDir().mkdir(applicationFolder);
        QFile file(database);
        file.open(QIODevice::ReadWrite);
    }

    db = new Database();
    header = new headerBar(this, db);
    ui->header_layout->addWidget(header);

    ui->newProject->setFlat(true);
    this->setWindowTitle("Welcome");

    connect(db, SIGNAL(dataObtained(QStringList)), this, SLOT(populateProjectList(QStringList)));
    db->getAllProjects();

    connect(db, SIGNAL(pathInfo(QStringList)), this, SLOT(updatePathInfo(QStringList)));
    db->getPath(QStringList() << "kallisto" << "bustools");

    db->removeBreadcrumbs();

    connect(&download, SIGNAL(releaseVersionSuccess()), this, SLOT(releaseToStatus()));
    connect(&download, SIGNAL(generalError(QString)), this, SLOT(displayError(QString)));

    this->setFixedSize(this->width(), this->height());

    ui->statusField->setReadOnly(true);

    download.moveToThread(downLoadThread);
}

MainWindow::~MainWindow()
{
    delete ui;
}

/**
 * @brief MainWindow::accessPathInfo creates a message box if kallisto and bustools can't be found. It offers the user the possibility that
 * the application collects the latest release for them
 */
void MainWindow::accessPathInfo() {
    install = new installation(this);
    connect(install, SIGNAL(installationSignal(QString)), this, SLOT(installPaths(QString)));
    connect(install, SIGNAL(cancelled()), this, SLOT(reactToCancelled()));
    install->show();
}

/**
 * @brief MainWindow::displayError general error catcher that displays the error that occur when fetching the data
 * @param message
 */
void MainWindow::displayError(QString message) {
    downLoadThread->quit();

    disconnect(&download, SIGNAL(pathSuccess(QString, QString)), this, SLOT(savePathToDatabase(QString, QString)));
    disconnect(this, SIGNAL(getPathInfo(QString, QString)), &download, SLOT(accessKallistoBustoolsInfo(QString, QString)));

    ui->statusField->setStyleSheet("color:red; background-color:black");
    ui->statusField->setHtml("<h3>" + message + "</h3>");
}

/**
 * @brief MainWindow::failure something went wrong when collecting or installing kallisto/bustools
 * @param error
 */
void MainWindow::failure(QString error) {
    disconnect(db, SIGNAL(error(QString)), this, SLOT(failure(QString)));
    ui->statusField->setHtml("<p>Database error: " + error +  "</p>");
    redirectEnabled=false;
}

void MainWindow::installPaths(QString operatingSystem) {
    disconnect(install, SIGNAL(installationSignal(QString)), this, SLOT(installPaths(QString)));
    disconnect(install, SIGNAL(cancelled()), this, SLOT(reactToCancelled()));
    install->deleteLater();
    connect(&download, SIGNAL(pathSuccess(QString, QString)), this, SLOT(savePathToDatabase(QString, QString)));
    connect(this, SIGNAL(getPathInfo(QString, QString, QString)), &download, SLOT(accessKallistoBustoolsInfo(QString, QString, QString)));

    downLoadThread->start(QThread::HighestPriority);

    ui->statusField->setHtml("<h3> Accessing latest release information...</h3>");

    emit getPathInfo("https://api.github.com/repos/pachterlab/kallisto/releases/latest", "https://api.github.com/repos/BUStools/bustools/releases/latest", operatingSystem);
}

/**
 * @brief MainWindow::on_newProject_clicked opens the new project window for the user to fill out and
 * create a new project
 */
void MainWindow::on_newProject_clicked()
{
    if(redirectEnabled) {
        newProjectDialog = new newProject(this, db);
        newProjectDialog->show();
        this->hide();
    } else {
        this->accessPathInfo();
    }
}

/**
 * @brief MainWindow::populateCheckBox create the list of project the user has created
 * @param names
 */
void MainWindow::populateProjectList(QStringList projectNames) {
    disconnect(db, SIGNAL(dataObtained(QStringList)), this, SLOT(populateProjectList(QStringList)));
    projectComboBox = new QComboBox;
    projectComboBox->addItem("Choose a project");
    foreach(QString name, projectNames) {
        projectComboBox->addItem(name);
    }

    ui->projectLayout->addWidget(projectComboBox, 0, 0, Qt::AlignLeft);
    connect(projectComboBox, SIGNAL(activated(QString)), this, SLOT(transferToProject(QString)));
}

/**
 * @brief MainWindow::processPathInfo the slot after the systems call to find kallisto and bustools on the machine
 * @param kallistoPath
 * @param bustoolsPath
 */
void MainWindow::processPathInfo(QString kallistoPath, QString bustoolsPath) {
    if(kallistoPath.isEmpty() || bustoolsPath.isEmpty()) {
        redirectEnabled=false;
    } else {
        if(kallistoPath.contains("\n")) {
            kallistoPath = kallistoPath.mid(0, kallistoPath.length()-1);
        }
        if(bustoolsPath.contains("\n")) {
            bustoolsPath = bustoolsPath.mid(0, bustoolsPath.length()-1);
        }
        programPath = calc.findOutputField(kallistoPath, true);

        connect(db, SIGNAL(pathAdded()), this, SLOT(success()));
        connect(db, SIGNAL(error(QString)), this, SLOT(failure(QString)));
        db->insertPath("kallisto", kallistoPath);
        db->insertPath("bustools", bustoolsPath);
    }
}

void MainWindow::reactToCancelled() {
    disconnect(install, SIGNAL(installationSignal(QString)), this, SLOT(installPaths(QString)));
    disconnect(install, SIGNAL(cancelled()), this, SLOT(reactToCancelled()));
    install->deleteLater();

    ui->statusField->setHtml("<h3> Kallisto and bustools couldn't be found, please install and restart application");

}

/**
 * @brief MainWindow::releaseToStatus updates the status of the installation when the latest release of kallisto and bustools
 * is installed
 */
void MainWindow::releaseToStatus()  {
    ui->statusField->setHtml("<h3> Release version information collected, downloading applications... </h3>");
}

/**
 * @brief MainWindow::savePathToDatabase stores the directory to the kallisto and bustools paths in the database
 * @param kallistoPath
 * @param bustoolsPath
 */
void MainWindow::savePathToDatabase(QString kallistoPath, QString bustoolsPath) {
    downLoadThread->quit();

    if(kallistoPath.contains(QDir::homePath())) {
        QString tempPath = calc.findOutputField(kallistoPath, true);
        programPath = calc.findOutputField(tempPath, true);
    } else {
        programPath = calc.findOutputField(kallistoPath, true);
    }

    disconnect(&download, SIGNAL(pathSuccess(QString, QString)), this, SLOT(savePathToDatabase(QString, QString)));
    disconnect(this, SIGNAL(getPathInfo(QString, QString, QString)), &download, SLOT(accessKallistoBustoolsInfo(QString, QString, QString)));

    ui->statusField->setHtml("<h3> Saving to database... </h3>");

    connect(db, SIGNAL(pathAdded()), this, SLOT(success()));
    connect(db, SIGNAL(error(QString)), this, SLOT(failure(QString)));
    db->insertPath("kallisto", kallistoPath);
    db->insertPath("bustools", bustoolsPath);

    delete downLoadThread;
}

/**
 * @brief MainWindow::success the installation, or data collection, was successful
 */
void MainWindow::success() {
    disconnect(db, SIGNAL(pathAdded()), this, SLOT(success()));
    ui->statusField->setHtml("<p> Kallisto and bustools successfully found at " + programPath +  "</p>");
    redirectEnabled=true;
}

/**
 * @brief MainWindow::transferToProject opens the project window if the user chooses a project from the list
 * @param name
 */
void MainWindow::transferToProject(QString name) {
    if(projectComboBox->currentIndex() != 0) {
        if(redirectEnabled) {
            openProjectDialog = new openProject(this, name, db);
            openProjectDialog->show();
            this->hide();
        } else {
            this->accessPathInfo();
        }
    }
}

/**
 * @brief MainWindow::updatePathInfo checks if paths are already in the database, otherwise checks the machine
 * @param results
 */
void MainWindow::updatePathInfo(QStringList results) {
    if(results[0].isEmpty() || results[1].isEmpty()) {
        connect(&calc, SIGNAL(pathInfoReady(QString, QString)), this, SLOT(processPathInfo(QString, QString)));
        calc.pathProcess();
    } else {
        if(results[0].contains(QDir::homePath())) {
            QString tempPath = calc.findOutputField(results[0], true);
            programPath = calc.findOutputField(tempPath, true);
        } else {
            programPath = calc.findOutputField(results[0], true);
        }
        ui->statusField->setHtml("<p> Kallisto and bustools successfully found at " + programPath);
        redirectEnabled=true;
    }
}
