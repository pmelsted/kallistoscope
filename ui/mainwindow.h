#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include "openproject.h"
#include "newproject.h"
#include"../logic/calculation.h"
#include "../logic/database.h"
#include "../logic/filedownloads.h"
#include "installation.h"
#include "headerbar.h"
#include <QMainWindow>
#include <QComboBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    calculation calc;
    Database* db;
    filedownloads download;
    QThread* downLoadThread = new QThread(this);
    headerBar* header;
    installation* install;
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
signals:
    void getPathInfo(QString, QString, QString);
public slots:
    void savePathToDatabase(QString, QString);
private slots:
    void displayError(QString);
    void failure(QString);
    void installPaths(QString);
    void on_newProject_clicked();
    void populateProjectList(QStringList);
    void processPathInfo(QString, QString);
    void reactToCancelled();
    void releaseToStatus();
    void success();
    void transferToProject(QString);
    void updatePathInfo(QStringList);
private:
    Ui::MainWindow *ui;
    void accessPathInfo();
    openProject* openProjectDialog;
    QString programPath;
    QComboBox* projectComboBox;
    newProject* newProjectDialog;
    bool redirectEnabled = true;
};

#endif // MAINWINDOW_H
