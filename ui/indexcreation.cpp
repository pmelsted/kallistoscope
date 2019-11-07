#include "indexcreation.h"
#include "ui_indexcalculation.h"
#include "newproject.h"

#include <QFileDialog>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMessageBox>

static QList< speciesInfo > speciesList;

/**
 * @brief indexCalculation::indexCalculation ui for the providing the index or information neccessary for the application
 * to calculate it for the user
 * @param parent
 * @param dbInstance
 */

indexcreation::indexcreation(QWidget *parent, Database* dbInstance) :
    QMainWindow(parent),
    ui(new Ui::indexCalculation)
{
    ui->setupUi(this);
    this->setWindowTitle("Calculate a new index");

    if(dbInstance == nullptr) {
        db = new Database();
    } else {
        db = dbInstance;
    }
    ancestor = parent;
    connect(db, SIGNAL(pathInfo(QStringList)), this, SLOT(setPathInfo(QStringList)));
    QStringList pathNames;
    pathNames.append("kallisto");
    pathNames.append("bustools");
    db->getPath(pathNames);

    connect(db, SIGNAL(speciesInfoReady(QList< speciesInfo >, QString)), this, SLOT(populateSpecies(QList < speciesInfo >, QString)));
    db->getSpecies();

    calc.moveToThread(calculationThread);

    connect(this, SIGNAL(performIndexCalc(QString, QStringList)), &calc, SLOT(startProcess(QString, QStringList)));
    connect(&calc, SIGNAL(indexCalcFinished()), this, SLOT(updateInfo()));

    connect(db, SIGNAL(error(QString)), this, SLOT(handleError(QString)));

    connect(&calc, SIGNAL(newOutput(QString)), this, SLOT(updateOutputLayout(QString)));
    connect(&calc, SIGNAL(newError(QString)), this, SLOT(updateErrorLayout(QString)));

    this->setFixedSize(this->width(), this->height());

    header = new headerBar(this, db);
    header->collectHeaderInfo("Calculate index");
    ui->header_bar_scroll->setWidget(header);

    connect(header, SIGNAL(newProjectClicked()), this, SLOT(removeReference()));
    connect(header, SIGNAL(landscapeChanged()), this, SLOT(removeReference()));
    connect(header, SIGNAL(indexfileClicked()), this, SLOT(removeReference()));
    connect(header, SIGNAL(performClicked()), this, SLOT(removeReference()));
}

indexcreation::~indexcreation()
{
    delete ui;
}

/**
 * @brief indexCalculation::calculateIndex index calculation for methods that need to wait for a download of the
 * fasta file to finish
 */
void indexcreation::calculateIndex() {
    disconnect(&downloads, SIGNAL(downloadFinished()), this, SLOT(calculateIndex()));
    ui->statusField->setHtml("<h3> Performing calculations.. </h3>");

    QString fastaFileName = downloads.getDestinationFile();

    QString program = kallistoPath;
    QStringList arguments;
    arguments << "index" << "-i" << indexDestination << fastaFileName;

    emit performIndexCalc(program, arguments);
    calculationThread->start(QThread::HighestPriority);
}

/**
 * @brief indexCalculation::checkDirectoryAvailability check if suggested index full path is already a directory
 * @param directories
 */
void indexcreation::checkDirectoryAvailability(QStringList directories) {
    disconnect(db, SIGNAL(indexDirectories(QStringList)), this, SLOT(checkDirectoryAvailability(QStringList)));
    QString indexFileName = ui->indexFileName->toPlainText();
    QString destinationFolder = ui->indexFolderField->toPlainText();

    if(destinationFolder[destinationFolder.length()-1] != "/") {
        destinationFolder = destinationFolder + "/";
    }
    destinationFolder = QDir::toNativeSeparators(destinationFolder);

    downloads.setOutputFolder(destinationFolder);

    if(!indexFileName.contains(".")) {
        indexFileName = indexFileName + ".idx";
    }

    indexDestination = destinationFolder + indexFileName;

    foreach(QString directory, directories) {
        if(indexDestination == directory) {
            errorList.append("The full path " + indexDestination + " already exists");
        }
    }
}

/**
 * @brief indexCalculation::checkNameAvailability check if index name is free
 * @param names
 */
void indexcreation::checkNameAvailability(QStringList names) {
    disconnect(db, SIGNAL(indexInfo(QStringList)), this, SLOT(checkNameAvailability(QStringList)));
    QString suggestedIndexName = ui->indexName->toPlainText();
    foreach(QString name, names) {
         if(suggestedIndexName == name) {
             errorList.append("Please select a unique index name");
         }
    }
}

/**
 * @brief indexCalculation::computeFailure default error window update
 * @param response
 */
void indexcreation::computeFailure(QString response) {
    QString responseHTML = "<p>" + response + "</p>";
    ui->statusField->setStyleSheet({"color:red; font-size: 17px; padding-top:25px; background-color:black"});
    ui->statusField->setHtml(responseHTML);
}

/**
 * @brief indexCalculation::connectionError displays an error if there is no internet connection when fetching the data
 */
void indexcreation::connectionError() {
    ui->statusField->setStyleSheet("color: red; background-color:black;");
    QString connectionStatus = "<h3> Something went wrong when fetching data, check internet connection and try again";
    ui->statusField->setHtml(connectionStatus);
}

/**
 * @brief indexCalculation::errorCheck checks whether all the field that need to be filled are filled
 * @return List of errors that need to fixed by the user
 */
void indexcreation::errorCheck() {
    if(ui->indexFolderField->toPlainText().isEmpty()) {
        errorList.append("Please select an output folder for the fasta and index file");
        return;
    }

    if(ui->indexName->toPlainText().isEmpty()) {
        errorList.append("Please select a index name");
    }

    connect(db, SIGNAL(indexInfo(QStringList)), this, SLOT(checkNameAvailability(QStringList)));
    db->getIndexNames();

    connect(db, SIGNAL(indexDirectories(QStringList)), this, SLOT(checkDirectoryAvailability(QStringList)));
    db->getIndexDirectories();

    QAbstractButton* indexSelection = ui->indexSelectionButtonGroup->checkedButton();

    if(indexSelection == nullptr) {
        errorList.append("Please select an option for how the index file will be provided");
    } else {
        QString indexSelectionName = indexSelection->objectName();
        if(indexSelectionName == "urlChoice") {
            QString urlDirectory = ui->urlDirectory->toPlainText();
            if(urlDirectory.isEmpty()) {
                errorList.append("Please provide an url for fasta file collection");
            }
        } else if(indexSelectionName == "fileChoice") {
            QString fastaFile = ui->fastaFile->toPlainText();
            if(fastaFile.isEmpty()) {
                errorList.append("Please select a fasta file");
            }
        }
    }

    QString indexFileName = ui->indexFileName->toPlainText();

    if(indexFileName.isEmpty()) {
        errorList.append("Please select a file name for the index file");
    }
}

/**
 * @brief indexCalculation::handleError default error handler
 * @param error the error in question
 */
void indexcreation::handleError(QString error) {
    ui->statusField->setHtml("<p>" + error + "</p>");
    ui->statusField->setStyleSheet("color:red; background-color:black");
}

void indexcreation::index_redirect() {
    emit index_created(ui->indexName->toPlainText());
    this->deleteLater();
}

/**
 * @brief indexCalculation::on_calculateIndex_clicked calculating the index with kallisto
 */
void indexcreation::on_calculateIndex_clicked()
{
    this->errorCheck();
    if(errorList.isEmpty()) {
        ui->statusField->setHtml("<h3> Performing calculations...</h3>");
        ui->statusField->setStyleSheet("color:white; background-color:black");

        QAbstractButton* checkedRadioButton = ui->indexSelectionButtonGroup->checkedButton();
        QString indexSelection = checkedRadioButton->objectName();

        if(indexSelection == "speciesChoice") {
            int checkBoxSelect = speciesComboBox->currentIndex();
            speciesInfo selectedPair = speciesList[checkBoxSelect];
            QString speciesIndex = selectedPair.name;
            QString selectedSpecies = selectedPair.displayName;
            QString tempToUpper = speciesIndex;
            tempToUpper[0] = tempToUpper[0].toUpper();

            speciesString = GENERAL_CURRENT_INDEX_DIRECTORY + speciesIndex + GENERAL_CDNA + tempToUpper + "." + selectedPair.assembly + GENERAL_FASTA_FILE_END;

            connect(&downloads, SIGNAL(downloadFinished()), this, SLOT(calculateIndex()));
            connect(&downloads, SIGNAL(downloadFailed(QString)), this, SLOT(computeFailure(QString)));
            ui->statusField->setHtml("<h3> Fetching fasta file...</h3>");
            downloads.setDestinationFile(speciesString);
        } else if(indexSelection == "urlChoice") {
            downloadUrl = ui->urlDirectory->toPlainText();
            connect(&downloads, SIGNAL(downloadFinished()), this, SLOT(calculateIndex()));
            connect(&downloads, SIGNAL(downloadFailed(QString)), this, SLOT(computeFailure(QString)));
            ui->statusField->setHtml("<h3> Fetching fasta file...</h3>");
            downloads.setDestinationFile(downloadUrl);
        }  else {
            QString fastaFileName = ui->fastaFile->toPlainText();

            QString program = kallistoPath;

            QStringList arguments;
            arguments << "index" << "-i" << indexDestination << fastaFileName;

            emit performIndexCalc(program, arguments);
            calculationThread->start(QThread::HighestPriority);
       }
    } else {
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
 * @brief indexCalculation::on_cancel_clicked user decides to exit from the page
 */
void indexcreation::on_cancel_clicked()
{
    QMessageBox::StandardButton reply;
    QString open = "Are you sure, the changes will not be saved?";
    reply = QMessageBox::question(this, "Duplicate", open, QMessageBox::No|QMessageBox::Yes);

    if(reply == QMessageBox::Yes) {
        if(ancestor == nullptr)   {
            newProject* project = new newProject(nullptr, nullptr, "");
            project->show();
        } else {
            emit cancelled();
        }
        delete this;
    }
}

/**
 * @brief indexCalculation::on_fastaFileSelect_clicked select fasta file from file path in users machine
 */
void indexcreation::on_fastaFileSelect_clicked()
{
    QString file_name = QFileDialog::getOpenFileName(this, "Select file", QDir::homePath());
    ui->fastaFile->setText(file_name);
}

/**
 * @brief indexCalculation::on_indexFolderButton_clicked select output folder for fasta file and index
 */
void indexcreation::on_indexFolderButton_clicked()
{
    QString folder_name = QFileDialog::getExistingDirectory(this, "Select output folder", QDir::homePath());
    ui->indexFolderField->setText(folder_name);
}

/**
 * @brief indexCalculation::populateSpecies check whether the species information needs to be updated
 * collects the data if needed and shows it to the user
 * @param info the information collected
 * @param timestamp when the information was collected
 */
void indexcreation::populateSpecies(QList < speciesInfo > info, QString timestamp) {
    qint64 ageOfInfo;
    if(!timestamp.isEmpty()) {
        QDate today = QDate::currentDate();
        QString year = timestamp.mid(0, 4);
        QString month = timestamp.mid(5, 2);
        QString day = timestamp.mid(8, 2);
        QDate saveDate = QDate(year.toInt(), month.toInt(), day.toInt());

        ageOfInfo = saveDate.daysTo(today);

    } else {
        ageOfInfo = 0;
    }
    if(info.isEmpty() || ageOfInfo > 90) {
        downloads.getData(QUrl("http://rest.ensembl.org/info/species?content-type=application/json"), this);
    } else {
        foreach(speciesInfo val, info) {
            speciesList.append(val);
            speciesComboBox->addItem(val.displayName);
        }
        ui->speciesGrid->addWidget(speciesComboBox, 0, 0, Qt::AlignLeft);
    }
}

/**
 * @brief indexCalculation::removeReference added to garbage collection if a breadcrumb in the list, that is not
 * the current page is selected
 */
void indexcreation::removeReference() {
    disconnect(header, SIGNAL(newProjectClicked()), this, SLOT(removeReference()));
    disconnect(header, SIGNAL(landscapeChanged()), this, SLOT(removeReference()));
    disconnect(header, SIGNAL(indexfileClicked()), this, SLOT(removeReference()));
    disconnect(header, SIGNAL(performClicked()), this, SLOT(removeReference()));
    this->hide();
    this->deleteLater();
}

/**
 * @brief indexCalculation::responseReady results from a request sent here and processed for the layout
 * @param response the result from the request sent to filedownloads
 */
void indexcreation::responseReady(QNetworkReply* response)
{
    disconnect(downloads.manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(responseReady(QNetworkReply*)));
    if(response->error() == QNetworkReply::NoError) {
        QList<speciesInfo> temp;
        db->deleteSpeciesInfo();
        speciesComboBox->clear();
        speciesInfo human;
        speciesInfo mouse;
        QByteArray response_data = response->readAll();
        QJsonDocument json = QJsonDocument::fromJson(response_data);

        QJsonObject values = json.object();
        QJsonArray jsonArray = values["species"].toArray();

        foreach(const QJsonValue & value, jsonArray) {
            QJsonObject object = value.toObject();
            speciesInfo info;
            info.name = object["name"].toString();
            info.displayName = object["display_name"].toString();
            info.assembly = object["assembly"].toString();

            if(info.displayName == "Human") {
                human = info;
            } else if(info.displayName == "Mouse") {
                mouse = info;
            } else {
                temp.append(info);
            }
        }
        speciesList.append(human);
        speciesList.append(mouse);
        speciesList.append(temp);
        db->insertSpecies(speciesList);
        foreach(speciesInfo val, speciesList) {
            speciesComboBox->addItem(val.displayName);
        }
        ui->speciesGrid->addWidget(speciesComboBox, 0, 0, Qt::AlignLeft);
    } else {
        this->connectionError();
    }
}

/**
 * @brief indexCalculation::setPathInfo gets the information about the kallisto path
 * @param paths collected from the database
 */
void indexcreation::setPathInfo(QStringList paths) {
     kallistoPath = paths[0];

    if(kallistoPath.isEmpty()) {
        ui->statusField->setHtml("<h3>Kallisto can't be found, please install and try again</h3>");
        ui->statusField->setStyleSheet("color:red; background-color:black");
    }
}

/**
 * @brief indexCalculation::updateErrorLayout a new error signal from the calculations
 * @param message
 */
void indexcreation::updateErrorLayout(QString message) {
    message = "<p>" + message + "</p>";
    ui->statusField->setStyleSheet("color:red;  background-color:black");
    ui->statusField->setHtml(message);
}

/**
 * @brief indexCalculation::updateInfo storing all the connected information
 */
void indexcreation::updateInfo() {
    calculationThread->quit();

    ui->statusField->setHtml("<h3> Index created, saving..</h3>");

    QString fetchUrl;
    QString fastaFileDest;

    QString checkedButton = ui->indexSelectionButtonGroup->checkedButton()->objectName();

    if(checkedButton == "speciesChoice") {
        fetchUrl = speciesString;
        fastaFileDest = downloads.getDestinationFile();
    } else if(checkedButton == "urlChoice") {
        fetchUrl = ui->urlDirectory->toPlainText();
        fastaFileDest = downloads.getDestinationFile();
    } else {
        fetchUrl = ui->fastaFile->toPlainText();
        fastaFileDest = ui->fastaFile->toPlainText();
    }

    delete calculationThread;

    connect(db, SIGNAL(indexAdded()), this, SLOT(index_redirect()));
    db->insertIndexInfo(ui->indexName->toPlainText(), ui->indexDescription->toPlainText(), fastaFileDest, indexDestination, checkedButton, fetchUrl);
}

/**
 * @brief indexCalculation::updateOutputLayout a new output signal from calculations
 * @param message
 */
void indexcreation::updateOutputLayout(QString message) {
    message = "<p>" + message + "</p>";
    allOutput = allOutput + message;
    ui->statusField->setStyleSheet("color:white;  background-color:black");
    ui->statusField->setHtml(allOutput);
}
