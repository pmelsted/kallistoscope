#ifndef OPENPROJECT_H
#define OPENPROJECT_H
#include "../logic/database.h"
#include "headerbar.h"
#include <QMainWindow>
#include <QString>
#include <QComboBox>

namespace Ui {
class openProject;
}

class openProject : public QMainWindow
{
    Q_OBJECT
    headerBar* header;
    Database* db;
    QComboBox* projectList = new QComboBox;
public:
    explicit openProject(QWidget *parent = nullptr, QString name="", Database* databaseInstance=nullptr);
    ~openProject();
private slots:
    void addNewTab(QString);
    void on_actionDelete_projects_triggered();
    void on_actionNew_triggered();
    void on_actionChange_triggered();
    void populateProjectList(QStringList);
    void removeReference();
    void updateTabs(QStringList);
private:
    Ui::openProject *ui;
};

#endif // OPENPROJECT_H
