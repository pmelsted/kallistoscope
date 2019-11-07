#include "openproject.h"
#include "ui_openproject.h"
#include "computeresults.h"
#include "mainwindow.h"
#include "removeprojects.h"
#include <QDebug>
#include <QMessageBox>

/**
 * @brief openProject::openProject window for calculation results based on an already calculated index
 * @param parent
 * @param name the name of the current project
 * @param databaseInstance the database instance being used
 */
openProject::openProject(QWidget *parent, QString name, Database* databaseInstance):
    QMainWindow(parent),
    ui(new Ui::openProject)
{
    ui->setupUi(this);
    this->setWindowTitle("Run computations");

    if(databaseInstance == nullptr) {
        db = new Database();
    } else {
        db = databaseInstance;
    }

    ui->tabWidget->removeTab(0);
    if(!name.isEmpty()) {
        computeResults* tabWidget = new computeResults(this, name, db);
        ui->tabWidget->insertTab(0, tabWidget, name);
    }

    ui->tabWidget->setTabText(ui->tabWidget->count()-1, "+");
    ui->tabWidget->setCurrentIndex(0);

    connect(db, SIGNAL(dataObtained(QStringList)), this, SLOT(populateProjectList(QStringList)));
    db->getAllProjects();

    this->setFixedSize(this->width(), this->height());

    header = new headerBar(this, db);
    header->collectHeaderInfo("Run computation");

    ui->header_bar_scroll->setWidget(header);

    connect(header, SIGNAL(landscapeChanged()), this, SLOT(removeReference()));
    connect(header, SIGNAL(indexfileClicked()), this, SLOT(removeReference()));
    connect(header, SIGNAL(indexcalcClicked()), this, SLOT(removeReference()));
    connect(header, SIGNAL(newProjectClicked()), this, SLOT(removeReference()));
}

openProject::~openProject()
{
    delete ui;
}

/**
 * @brief openProject::addNewTab opens a project in a new tab if the user requests it
 * @param projectName
 */
void openProject::addNewTab(QString projectName) {
    if(projectList->currentIndex() != 0) {
        bool alreadyOpen = false;
        for(int i = 0; i < ui->tabWidget->count(); i++) {
            QString tabName = ui->tabWidget->tabText(i);
            if(tabName == projectName) {
                alreadyOpen = true;
            }
        }
        if(!alreadyOpen) {
            computeResults* tabWidget = new computeResults(this, projectName, db);
            ui->tabWidget->insertTab(ui->tabWidget->count()-1, tabWidget, projectName);
            ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-2);
        } else {
            QMessageBox::StandardButton reply;
            QString open = "This project is already open, do you want to open it in another window?";
            reply = QMessageBox::question(this, "Duplicate", open, QMessageBox::No|QMessageBox::Yes);

            if(reply == QMessageBox::Yes) {
                computeResults* tabWidget = new computeResults(this, projectName, db);
                ui->tabWidget->insertTab(ui->tabWidget->count()-1, tabWidget, projectName);
                ui->tabWidget->setCurrentIndex(ui->tabWidget->count()-2);
            }
        }
    }
}

/**
 * @brief openProject::on_actionChange_triggered changes have been requested for the current project displayed
 */
void openProject::on_actionChange_triggered()
{
    int index = ui->tabWidget->currentIndex();
    QString name = ui->tabWidget->tabText(index);
    newProject* changeProject = new newProject(this, db, name);
    connect(changeProject, SIGNAL(updateDone(QString)), ui->tabWidget->currentWidget(), SLOT(updateLayout(QString)));
    changeProject->show();
}

/**
 * @brief openProject::on_actionDelete_projects_triggered option for the user to delete projects he no longer uses
 */
void openProject::on_actionDelete_projects_triggered()
{
    removeProjects* remove = new removeProjects(this, db);
    connect(remove, SIGNAL(deleteComplete(QStringList)), this, SLOT(updateTabs(QStringList)));
    remove->show();
}

/**
 * @brief openProject::on_actionNew_triggered enables the user to open another project
*/
void openProject::on_actionNew_triggered()
{
    newProject* newP = new newProject(nullptr, db, "");
    newP->show();
    delete this;
}

/**
 * @brief openProject::populateProjectList create a list og the users projects
 * @param results
 */
void openProject::populateProjectList(QStringList results) {
    disconnect(projectList, SIGNAL(currentIndexChanged(QString)), this, SLOT(addNewTab(QString)));
    projectList->clear();
    QStringList projects;
    projects.append("Choose a project");
    foreach(QString name, results) {
        projects.append(name);
    }
    projectList->addItems(projects);
    ui->projectListCombo->addWidget(projectList, 0, 0, Qt::AlignLeft);

    connect(projectList, SIGNAL(activated(QString)), this, SLOT(addNewTab(QString)));
}

/**
 * @brief openProject::removeReference added to garbage collection if a breadcrumb in the list, that is not
 * the current page is selected
 */
void openProject::removeReference() {
    disconnect(header, SIGNAL(landscapeChanged()), this, SLOT(removeReference()));
    disconnect(header, SIGNAL(indexfileClicked()), this, SLOT(removeReference()));
    disconnect(header, SIGNAL(indexcalcClicked()), this, SLOT(removeReference()));
    disconnect(header, SIGNAL(performClicked()), this, SLOT(removeReference()));
    this->hide();
    this->deleteLater();
}

/**
 * @brief openProject::updateTabs if one of the open projects is deleted the tab has to be removed as well
 * @param values
 */
void openProject::updateTabs(QStringList values) {
    db->getAllProjects();

    for(int i = ui->tabWidget->count()-1; i > -1; i--) {
        QString name = ui->tabWidget->tabText(i);
        for(int j = 0; j < values.length(); j++) {
            if(values[j] == name) {
                ui->tabWidget->removeTab(i);
            }
        }
    }
    ui->tabWidget->setCurrentIndex(0);
}
