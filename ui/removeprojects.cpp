#include "removeprojects.h"
#include "ui_removeprojects.h"

#include <QCheckBox>
#include <QGridLayout>
#include <QString>
#include <QMessageBox>
#include <QDebug>

/**
 * @brief removeProjects::removeProjects UI for the user to choose projects he wants to delete
 * @param parent the widget that requested the window
 * @param dbInstance the instance of the database
 */
removeProjects::removeProjects(QWidget *parent, Database* dbInstance) :
    QDialog(parent),
    ui(new Ui::removeProjects)
{
    ui->setupUi(this);

    this->setWindowTitle("Delete projects");

    if(dbInstance == nullptr) {
        db = new Database();
    } else {
        db = dbInstance;
    }
    checkBoxGroup = new QButtonGroup();
    checkBoxGroup->setExclusive(false);
    QGridLayout* grid = new QGridLayout;
    ui->scrollAreaWidgetContents->setLayout(grid);

    connect(db, SIGNAL(dataObtained(QStringList)), this, SLOT(populateCheckBox(QStringList)));
    db->getAllProjects();

    this->setFixedSize(this->width(), this->height());

    connect(db, SIGNAL(deletionFailure()), this, SLOT(displayFailure()));
}

removeProjects::~removeProjects()
{
    delete ui;
}

/**
 * @brief removeProjects::deleteDone informs the parent the delete has been performed and removes itself
 */
void removeProjects::deleteDone() {
    emit deleteComplete(deleteList);
    delete this;
}

/**
 * @brief removeProjects::displayFailure default error show
 */
void removeProjects::displayFailure() {
    QMessageBox::information(this, "Error", "Something went wrong when deleting projects, please try again later");
}


/**
 * @brief removeProjects::on_cancel_delete_clicked if the user cancels the operation the window is deleted
 */
void removeProjects::on_cancel_delete_clicked()
{
    delete this;
}

/**
 * @brief removeProjects::on_delete_projects_clicked collects the information about the projects the
 * user wants to delete and performs the function call to delete them from the database
 */
void removeProjects::on_delete_projects_clicked()
{
    QMessageBox::StandardButton reply;
    QString open = "Are you sure, you want to delete these projects?";
    reply = QMessageBox::question(this, "Duplicate", open, QMessageBox::No|QMessageBox::Yes);

    if(reply == QMessageBox::Yes) {
        QList<QCheckBox *> checkList = ui->scrollAreaWidgetContents->findChildren<QCheckBox *>();

        foreach(auto button, checkList) {
            if(button->isChecked()) {
                deleteList.append(button->text());
            }
        }
        connect(db, SIGNAL(projectsDeleted()), this, SLOT(deleteDone()));
        db->removeProjects(deleteList);
    }
}

/**
 * @brief removeProjects::populateCheckBox create the list of project the user has created
 * @param names
 */
void removeProjects::populateCheckBox(QStringList names) {
    foreach(QString name, names) {
        QCheckBox* box = new QCheckBox(name);
        checkBoxGroup->addButton(box);
        ui->scrollAreaWidgetContents->layout()->addWidget(box);
    }
}
