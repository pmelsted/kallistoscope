#include "newproject.h"
#include "ui_newproject.h"
#include "openproject.h"
#include "mainwindow.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QDebug>
#include <QAbstractItemView>
#include <QAbstractScrollArea>

/**
 * @brief newProject::newProject constructor for a new project or when a project needs to be changed
 * @param parent is the parent of the new window
 * @param dbInstance the current instance of the database
 * @param name the name of the project to be changed, if the project is new this field is empty
 */
newProject::newProject(QWidget *parent, Database* dbInstance, QString name) :
    QMainWindow(parent),
    ui(new Ui::newProject)
{
    ui->setupUi(this);
    ancestor = parent;

    if(name.isEmpty()) {
        this->setWindowTitle("Create new project");
    } else {
        this->setWindowTitle("Change project");
    }

    projectName = name;
    ui->statusField->setReadOnly(true);


    ui->folderButtonGroup->connect(ui->folderButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(folder_button_clicked(QAbstractButton*)));
    if(dbInstance == nullptr) {
        db = new Database();
    } else {
        db = dbInstance;
    }

    this->populateTech();

    connect(db, SIGNAL(error(QString)), this, SLOT(handleError(QString)));

    if(!projectName.isEmpty()) {
        currProject = new project(name, db);
        projectData = currProject->provideProjectInfo();

        ui->projectNameField->setText(projectData.name);
        ui->outputFolderField->setText(projectData.resultDirectory);
        QString tech = projectData.technology;
        int index = techComboBox->findText(tech);
        techComboBox->setCurrentIndex(index);

        ui->listChoice->setChecked(true);

    }

    connect(db, SIGNAL(indexInfo(QStringList)), this, SLOT(populateIndexField(QStringList)));
    db->getIndexNames();

    this->setFixedSize(this->width(), this->height());

    if(projectName.isEmpty()) {
        header = new headerBar(this, db);
        header->collectHeaderInfo("Create project");

        ui->header_bar_scroll->setWidget(header);

        connect(header, SIGNAL(landscapeChanged()), this, SLOT(removeReference()));
        connect(header, SIGNAL(indexfileClicked()), this, SLOT(removeReference()));
        connect(header, SIGNAL(indexcalcClicked()), this, SLOT(removeReference()));
        connect(header, SIGNAL(performClicked()), this, SLOT(removeReference()));
    }

}

newProject::~newProject()
{
    delete ui;
}

/**
 * @brief newProject::checkNameAvailability checks if the project name provided does already exist in the database.
 * @param projectNames
 */
void newProject::checkNameAvailability(QStringList projectNames) {
    disconnect(db, SIGNAL(dataObtained(QStringList)), this, SLOT(checkNameAvailability(QStringList)));
    QString suggestedProjectName = ui->projectNameField->toPlainText();
    if(!projectNames.empty()) {
        foreach(QString projectName, projectNames) {
            if(suggestedProjectName == projectName) {
                errorList.append("Please select a unique project name");
            }
        }
    }
}

/**
 * @brief newProject::errorCheck checks whether all the field that need to be filled are filled
 * @return List of errors that need to fixed by the user
 */
void newProject::errorCheck() {
    QString newProjectName = ui->projectNameField->toPlainText();

    if(newProjectName.isEmpty()) {
        errorList.append("Please select a project name");
    } else {
        if(newProjectName != projectName) {
            connect(db, SIGNAL(dataObtained(QStringList)), this, SLOT(checkNameAvailability(QStringList)));
            db->getAllProjects();
        }
    }

    QString outputFolder = ui->outputFolderField->toPlainText();
    if(outputFolder.isEmpty()) {
        errorList.append("Please select an output folder for results");
    }

    QAbstractButton* currButton = ui->index_calc_group->checkedButton();

    if(currButton == nullptr) {
        errorList.append("Please select a method to provide an index file");
    }

    if(currButton->objectName() == "listChoice") {
        if(indexComboBox->currentIndex() == 0) {
            errorList.append("Please select an index option from the list");
        }
    }
}

/**
 * @brief newProject::folder_button_clicked selecting a folder, both for the fasta and index files and results
 * @param button
 */
void newProject::folder_button_clicked(QAbstractButton* button)
{
    QString folder_name = QFileDialog::getExistingDirectory(this, "Select output folder", QDir::homePath());
    if(button->objectName() == "outputFolderButton") {
        ui->outputFolderField->setText(folder_name);
    }
}

/**
 * @brief newProject::handleError default error handler
 * @param error the error in question
 */
void newProject::handleError(QString error) {
    ui->statusField->setHtml("<p>" + error + "</p>");
    ui->statusField->setStyleSheet("color:red; background-color:black");
}


/**
 * @brief newProject::on_cancel_clicked option to cancel the change or creating a new project
 */
void newProject::on_cancel_clicked()
{
    QMessageBox::StandardButton reply;
    QString open = "Are you sure, the changes will not be saved?";
    reply = QMessageBox::question(this, "Duplicate", open, QMessageBox::No|QMessageBox::Yes);

    if(reply == QMessageBox::Yes) {
        if(ancestor == nullptr) {
             openProject* openCurrProject = new openProject(nullptr, "", db);
             openCurrProject->show();
        } else if(projectName.isEmpty() && ancestor->objectName() == "MainWindow") {
            MainWindow* newMain = new MainWindow();
            newMain->show();
        } else {
        }
        delete this;
    }
}

/**
 * @brief newProject::on_saveProject_clicked either generate the instance and save it in the
 * database or update the fields that have been changed
 */
void newProject::on_saveProject_clicked()
{
    this->errorCheck();
    if(errorList.isEmpty()) {
        if(ui->index_calc_group->checkedButton()->objectName() == "calcChoice") {
            index = new indexcreation(this, db);
            index->show();
            this->hide();

            connect(index, SIGNAL(index_created(QString)), this, SLOT(updateInfoField(QString)));
            connect(index, SIGNAL(cancelled()), this, SLOT(updateBreadcrumbs()));
         } else if(ui->index_calc_group->checkedButton()->objectName() == "fileChoice") {
             indexfile = new indexFile(this, db);
             indexfile->show();
             this->hide();

            connect(indexfile, SIGNAL(index_created(QString)), SLOT(updateInfoField(QString)));
            connect(indexfile, SIGNAL(cancelled()), this, SLOT(updateBreadcrumbs()));
          } else {
            ui->statusField->setHtml("<h3> Saving project...</h3>");
            ui->statusField->setStyleSheet("color:white; background-color:black");

            connect(db, SIGNAL(indexInfoCollected(QStringList)), this, SLOT(saveProject(QStringList)));
            if(!indexName.isEmpty()) {
                db->getIndexInfo(indexName);
            } else {
                db->getIndexInfo(indexComboBox->currentText());
            }
          }
    } else {
        QString result;
        ui->statusField->setStyleSheet({"color:red; font-size: 13px; background-color:black"});
        result = "<ul><h3>The following errors where generated:</h3>";
        foreach(QString error, errorList) {
            result = result + "<li>" + error + "</li>";
        }
        result = result + "</ul>";
        ui->statusField->setHtml(result);
        errorList.clear();
    }

}

/**
 * @brief newProject::projectAdded the new project is saved and a window is open displaying it
 */
void newProject::projectCreated() {
    disconnect(db, SIGNAL(projectAdded()), this, SLOT(projectCreated()));
    openProject* openCurrProject = new openProject(this, ui->projectNameField->toPlainText(), db);
    openCurrProject->show();
    this->hide();
}

/**
 * @brief newProject::populateIndexField add information about the already existing indexes
 * @param names
 */
void newProject::populateIndexField(QStringList names) {
    disconnect(db, SIGNAL(indexInfo(QStringList)), this, SLOT(populateIndexField(QStringList)));
    indexComboBox->addItem("Select index");
    indexComboBox->addItems(names);

    indexComboBox->setMinimumContentsLength(20);

    indexComboBox->setSizeAdjustPolicy(QComboBox::AdjustToMinimumContentsLength);
    ui->indexNameGrid->addWidget(indexComboBox, 0, 0, Qt::AlignLeft);

    if(!projectName.isEmpty()) {
        connect(db, SIGNAL(indexInfoCollectedThroughDirectory(QStringList)), this, SLOT(setDefault(QStringList)));
        db->getIndexInfoThroughDirectory(projectData.indexDirectory);
    }
    if(!indexName.isEmpty()) {
        ui->listChoice->setChecked(true);
        int index = indexComboBox->findText(indexName);
        indexComboBox->setCurrentIndex(index);
    }
}

/**
 * @brief newProject::populateTech create a list of the technologies on offer
 */
void newProject::populateTech() {

    techComboBox->clear();
    techInfo.empty();
    techInfo.append("10xv1");
    techInfo.append("10xv2");
    techInfo.append("10xv3");
    techInfo.append("CEL-Seq");
    techInfo.append("CELSeq2");
    techInfo.append("DropSeq");
    techInfo.append("inDrops");
    techInfo.append("SCRB-Seq");
    techInfo.append("SureCell");

    techComboBox->addItems(techInfo);

    ui->techGrid->addWidget(techComboBox, 0, 0, Qt::AlignLeft);
}

/**
 * @brief newProject::projectUpdated the changes to the project have been saved in the database
 */
void newProject::projectUpdate() {
    disconnect(db, SIGNAL(projectUpdated()), this, SLOT(projectUpdate()));
    emit updateDone(ui->projectNameField->toPlainText());
    delete this;
}

void newProject::removeReference() {
    disconnect(header, SIGNAL(landscapeChanged()), this, SLOT(removeReference()));
    disconnect(header, SIGNAL(indexfileClicked()), this, SLOT(removeReference()));
    disconnect(header, SIGNAL(indexcalcClicked()), this, SLOT(removeReference()));
    disconnect(header, SIGNAL(performClicked()), this, SLOT(removeReference()));
    this->hide();
    this->deleteLater();
}

/**
 * @brief newProject::saveProject save the project if all the data is acceptable
 * @param indexInfo
 */
void newProject::saveProject(QStringList indexInfo) {
    if(projectName.isEmpty()) {
        connect(db, SIGNAL(projectAdded()), this, SLOT(projectCreated()));
        db->createNewProject(ui->projectNameField->toPlainText(), indexInfo[3], ui->outputFolderField->toPlainText(), techComboBox->currentText(), indexInfo[2]);
    } else {
        connect(db, SIGNAL(projectUpdated()), this, SLOT(projectUpdate()));
        db->updateProject(projectName, ui->projectNameField->toPlainText(), indexInfo[3], ui->outputFolderField->toPlainText(), techComboBox->currentText(), indexInfo[2]);
    }
}

/**
 * @brief newProject::setDefault access the index list and sets the new index as the default
 * @param results
 */
void newProject::setDefault(QStringList results) {
    int index = indexComboBox->findText(results[0]);
    indexComboBox->setCurrentIndex(index);
}

/**
 * @brief newProject::updateBreadcrumbs slot when the calculations or saveing of the index is cancelled. New project window reappears and
 * updates the header
 */
void newProject::updateBreadcrumbs() {
    this->show();
    header->collectHeaderInfo("Create project");
    ui->statusField->clear();
}

/**
 * @brief newProject::updateInfoField when a new index is created
 * @param indexValue
 */
void newProject::updateInfoField(QString indexValue) {
    this->show();
    header->collectHeaderInfo("Create project");
    QString message = "<h4> Index " + indexValue + " was created and will be saved with this project </h4>";
    ui->statusField->setStyleSheet("color:white; background-color:black");
    ui->statusField->setHtml(message);
    indexName = indexValue;

    indexComboBox->clear();
    connect(db, SIGNAL(indexInfo(QStringList)), this, SLOT(populateIndexField(QStringList)));
    db->getIndexNames();
}
