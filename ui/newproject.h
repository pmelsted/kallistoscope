#ifndef NEWPROJECT_H
#define NEWPROJECT_H

#include "../logic/database.h"
#include "../logic/project.h"
#include "headerbar.h"
#include "indexcreation.h"
#include "indexfile.h"
#include <QMainWindow>
#include <QComboBox>
#include <QAbstractButton>
#include <QThread>

namespace Ui {
class newProject;
}

class newProject : public QMainWindow
{
    Q_OBJECT
    QWidget* ancestor;
    headerBar* header;
    project *currProject;
    Database* db;
    indexcreation* index;
    indexFile* indexfile;
    QString indexName;
    struct projectInfo projectData;
    QString projectName;
    QStringList techInfo;
public:
    explicit newProject(QWidget *parent = nullptr, Database* dbInstance=nullptr, QString name = "");
    ~newProject();
signals:
    void updateDone(QString);
private slots:
    void checkNameAvailability(QStringList);
    void folder_button_clicked(QAbstractButton*);
    void handleError(QString);
    void on_cancel_clicked();
    void on_saveProject_clicked();
    void populateIndexField(QStringList);
    void projectCreated();
    void projectUpdate();
    void removeReference();
    void saveProject(QStringList);
    void setDefault(QStringList);
    void updateBreadcrumbs();
    void updateInfoField(QString);
private:
    Ui::newProject *ui;
    void errorCheck();
    QStringList errorList;
    QComboBox *indexComboBox = new QComboBox;
    void populateTech();
    QComboBox *techComboBox = new QComboBox;
};

#endif // NEWPROJECT_H
