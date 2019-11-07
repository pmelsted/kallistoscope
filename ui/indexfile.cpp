#include "indexfile.h"
#include "ui_indexfile.h"
#include "newproject.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

/**
 * @brief indexFile::indexFile ui page for providing index file directly
 * @param parent
 * @param databaseInstance
 */
indexFile::indexFile(QWidget *parent, Database* databaseInstance) :
    QMainWindow(parent),
    ui(new Ui::indexFile)
{
    ui->setupUi(this);
    ancestor = parent;
    this->setWindowTitle("Provide an index file");
    if(databaseInstance == nullptr) {
        db = new Database();
    } else {
        db = databaseInstance;
    }

    header = new headerBar(this, db);
    header->collectHeaderInfo("Provide index file");
    ui->header_bar_scroll->setWidget(header);

    connect(header, SIGNAL(newProjectClicked()), this, SLOT(removeReference()));
    connect(header, SIGNAL(landscapeChanged()), this, SLOT(removeReference()));
    connect(header, SIGNAL(indexcalcClicked()), this, SLOT(removeReference()));
    connect(header, SIGNAL(performClicked()), this, SLOT(removeReference()));
}

indexFile::~indexFile()
{
    delete ui;
}

/**
 * @brief indexCalculation::checkDirectoryAvailability check if suggested index full path is already a directory
 * @param directories
 */
void indexFile::checkDirectoryAvailability(QStringList directories) {
    disconnect(db, SIGNAL(indexDirectories(QStringList)), this, SLOT(checkDirectoryAvailability(QStringList)));
    foreach(QString directory, directories) {
        if(ui->index_file_field->toPlainText() == directory) {
            errorList.append("The full path " + ui->index_file_field->toPlainText() + " already exists");
        }
    }
}

/**
 * @brief indexCalculation::checkNameAvailability check if index name is free
 * @param names
 */
void indexFile::checkNameAvailability(QStringList names) {
    disconnect(db, SIGNAL(indexInfo(QStringList)), this, SLOT(checkNameAvailability(QStringList)));
    QString suggestedIndexName = ui->indexName->toPlainText();
    foreach(QString name, names) {
         if(suggestedIndexName == name) {
             errorList.append("Please select a unique index name");
         }
    }
}

/**
 * @brief indexFile::errorCheck checkes whether neccessary fields are filled out and whether the index name or path is already taken.
 * The index name and path need to be unique
 */
void indexFile::errorCheck() {
    if(ui->index_file_field->toPlainText().isEmpty()) {
        errorList.append("Please provide an index file");
        return;
    }

    if(ui->indexName->toPlainText().isEmpty()) {
        errorList.append("Please select a index name");
    }

    connect(db, SIGNAL(indexInfo(QStringList)), this, SLOT(checkNameAvailability(QStringList)));
    db->getIndexNames();

    connect(db, SIGNAL(indexDirectories(QStringList)), this, SLOT(checkDirectoryAvailability(QStringList)));
    db->getIndexDirectories();

}

/**
 * @brief indexFile::index_redirect index has been created and now this ui page can be deleted
 */
void indexFile::index_redirect() {
    emit index_created(ui->indexName->toPlainText());
    delete this;
}

/**
 * @brief indexFile::on_saveIndex_clicked checks for errors, otherwise the new index is saved in the database
 */
void indexFile::on_saveIndex_clicked()
{
    this->errorCheck();
    if(errorList.isEmpty()) {
        connect(db, SIGNAL(indexAdded()), this, SLOT(index_redirect()));
        db->insertIndexInfo(ui->indexName->toPlainText(), ui->indexDescription->toPlainText(), "", ui->index_file_field->toPlainText(), "", "");
    }  else {
        QString result;
        ui->statusField->setStyleSheet({"color:red; font-size: 13px; padding-top:25px; background-color:black"});
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
 * @brief indexFile::on_go_back_clicked option to cancel the adding of a new index
 */
void indexFile::on_go_back_clicked()
{
    QMessageBox::StandardButton reply;
    QString open = "Are you sure, the changes will not be saved?";
    reply = QMessageBox::question(this, "Duplicate", open, QMessageBox::No|QMessageBox::Yes);

    if(reply == QMessageBox::Yes) {

        if(ancestor == nullptr) {
            newProject* project = new newProject(nullptr, nullptr, "");
            project->show();
        } else {
            emit cancelled();
        }
        delete this;
    }
}

/**
 * @brief indexFile::on_select_index_file_clicked file select option in ui selected.
 */
void indexFile::on_select_index_file_clicked()
{
   QString index_file_name = QFileDialog::getOpenFileName(this, "Select index file", QDir::homePath());
   ui->index_file_field->setText(index_file_name);

}

/**
 * @brief indexFile::removeReference added to garbage collection if a breadcrumb in the list, that is not
 * the current page is selected
 */
void indexFile::removeReference() {
    disconnect(header, SIGNAL(newProjectClicked()), this, SLOT(removeReference()));
    disconnect(header, SIGNAL(landscapeChanged()), this, SLOT(removeReference()));
    disconnect(header, SIGNAL(indexcalcClicked()), this, SLOT(removeReference()));
    disconnect(header, SIGNAL(performClicked()), this, SLOT(removeReference()));
    this->hide();
    this->deleteLater();
}
