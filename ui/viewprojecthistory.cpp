#include "viewprojecthistory.h"
#include "ui_viewprojecthistory.h"

#include <QDebug>

/**
 * @brief ViewProjectHistory::ViewProjectHistory  a ui for viewing the projects run history or index file information
 * @param parent
 * @param searchString
 * @param db_instance
 * @param runHistory
 */
ViewProjectHistory::ViewProjectHistory(QWidget *parent, QString searchString, Database* db_instance, bool runHistory) :
    QMainWindow(parent),
    ui(new Ui::ViewProjectHistory)
{
    ui->setupUi(this);

    ui->currentProjectName->setReadOnly(true);
    ui->runHistory->setReadOnly(true);

    if(db_instance == nullptr) {
        db = new Database();
    } else {
        db = db_instance;
    }

    if(runHistory) {
        ui->currentProjectName->setHtml("<h1> Current project: " + searchString + "</h1>");
        this->setWindowTitle("View project run history");
        connect(db, SIGNAL(outputReady(QList<runInfo>)), this, SLOT(populateRunHistory(QList<runInfo>)));
        db->getOutput(searchString);
    } else {
        this->setWindowTitle("View index information");
        connect(db, SIGNAL(indexInfoCollectedThroughDirectory(QStringList)), this, SLOT(populateInfo(QStringList)));
        db->getIndexInfoThroughDirectory(searchString);
    }

    this->setFixedSize(this->width(), this->height());
}

ViewProjectHistory::~ViewProjectHistory()
{
    delete ui;
}

/**
 * @brief ViewProjectHistory::on_go_back_clicked deletes ui object when the user has read the history
 */
void ViewProjectHistory::on_go_back_clicked()
{
    delete this;
}

/**
 * @brief ViewProjectHistory::populateInfo show the user the index file information for this project
 * @param results
 */
void ViewProjectHistory::populateInfo(QStringList results) {
    ui->currentProjectName->setHtml("<h1>Current index: " + results[0] + "</h1>");

    QString indexInfo = "<h3>Index description</h3>";
    if(results[1].isEmpty()) {
        indexInfo = indexInfo + "<p>There is no description available</p>";
    } else {
        indexInfo = indexInfo + "<p>" + results[1] + "<p>";
    }
    QString choiceInfo;

    indexInfo = indexInfo + "<h3>Current index file</h3>";
    indexInfo = indexInfo + "<p>" + results[3] + "</p>";

    indexInfo = indexInfo + "<h3>Fasta file information</h3>";

    if(results[4] == "speciesChoice") {
        choiceInfo = "<ul><li>File was collected througt the ftp ensembl webpage</li>";
        indexInfo = indexInfo + choiceInfo;
        indexInfo = indexInfo + "<li>The url: " + results[5] +  "</li>";
        indexInfo = indexInfo + "<li>The current filepath: " + results[2] + "</li></ul>";
    } else if(results[4] == "urlChoice") {
        choiceInfo = "<ul><li>File was collected through url provided</li>";
        indexInfo = indexInfo + choiceInfo;
        indexInfo = indexInfo + "<li>The url: " + results[5] +  "</li>";
        indexInfo = indexInfo + "<li>The current filepath: " + results[2] + "</li></ul>";
    } else {
        choiceInfo = "<ul><li>File was provided by the user</li>";
        indexInfo = indexInfo + choiceInfo;
        indexInfo = indexInfo + "<li>The filepath: " + results[5] + "</li></ul>";
    }

    ui->runHistory->setHtml(indexInfo);

}

/**
 * @brief ViewProjectHistory::populateRunHistory show the run history for this project
 * @param results
 */
void ViewProjectHistory::populateRunHistory(QList<runInfo> results) {
    if(results.isEmpty()) {
        ui->runHistory->setHtml("<h2> No history to display yet...</h2>");
    } else {
        foreach(runInfo result, results)  {
            message = message + "<h3>" + result.timestamp + "</h3>";
            message = message + result.output;
        }

        ui->runHistory->setHtml(message);
    }
}
