#include "computeresults.h"
#include "ui_computeresults.h"
#include "zlib.h"
#include "../logic/kseq.h"
#include "viewprojecthistory.h"
#include <QFileDialog>
#include <QDebug>
#include <QList>

KSEQ_INIT(gzFile, gzread);

/**
 * @brief computeResults::computeResults the ui within the open project ui which runs kallisto bus
 * and bustools results
 * @param parent
 * @param name
 * @param databaseInstance
 */
computeResults::computeResults(QWidget *parent, QString name, Database* databaseInstance) :
    QWidget(parent),
    ui(new Ui::computeResults)
{
    ui->setupUi(this);

    ui->setupUi(this);
    ui->statusField->setReadOnly(true);

    fastQFiles->addButton(ui->pushButton_3, 1);
    fastQFiles->setId(ui->pushButton_3, 1);
    ui->textEdit_4->setObjectName("fileField1");

    currentPath = QDir::homePath();

    if(databaseInstance == nullptr) {
        db = new Database();
    } else {
        db = databaseInstance;
    }

    currProject = new project(name, db);
    projectData = currProject->provideProjectInfo();

    this->createNewFastq();

    if(projectData.technology == "10xv1") {
        this->createNewFastq();
    }

    ui->outputFolderField->setText(projectData.resultDirectory);
    connect(ui->addFilePair, SIGNAL(clicked()), this, SLOT(on_addFilePair_clicked()));

    ui->file_select_button_group->connect(ui->file_select_button_group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(select_file_clicked(QAbstractButton*)));

    connect(ui->result_button, SIGNAL(clicked()), this, SLOT(on_result_button_clicked()));
    connect(ui->resultFolderButton, SIGNAL(clicked()), this, SLOT(folder_button_clicked()));

    connect(ui->snakemake_button, SIGNAL(clicked()), this, SLOT(on_snakemake_button_clicked()));

    connect(db, SIGNAL(pathInfo(QStringList)), this, SLOT(setPathInfo(QStringList)));
    QStringList pathNames;
    pathNames.append("kallisto");
    pathNames.append("bustools");
    db->getPath(pathNames);

    calc.moveToThread(calcThread);

    connect(fastQFiles, SIGNAL(buttonClicked(int)), this, SLOT(button_clicked(int)));

    connect(db, SIGNAL(indexInfoCollectedThroughDirectory(QStringList)), this, SLOT(showIndexInfo(QStringList)));
    db->getIndexInfoThroughDirectory(projectData.indexDirectory);

    connect(&calc, SIGNAL(statusInCalc(QString)), this, SLOT(showIntermediateState(QString)));

    connect(&calc, SIGNAL(newOutput(QString)), this, SLOT(updateOutputLayout(QString)));
    connect(&calc, SIGNAL(newError(QString)), this, SLOT(updateErrorLayout(QString)));

    ui->view_button_group->connect(ui->view_button_group, SIGNAL(buttonClicked(QAbstractButton*)), this, SLOT(view_clicked(QAbstractButton*)));

    this->populateWorkflow();
}

computeResults::~computeResults()
{
    delete ui;
}

/**
 * @brief calculateResults::button_clicked get the fastq file
 * @param id
 */
void computeResults::button_clicked(int id) {
    QString filter = "Fasta File (*.fastq.gz)";
    QString file_name = QFileDialog::getOpenFileName(this, "Select file", currentPath.path(), "*");
    QString file_directory = calc.findOutputField(file_name, true);

    if(currentPath.path() != file_directory) {
        currentPath.setPath(file_directory);
    }

    QString textFieldName = "fileField" + QString::number(id);
    QTextEdit* textField = this->findChild<QTextEdit *>(textFieldName);

    textField->setText(file_name);
}

/**
 * @brief calculateResults::checkFastQContent checks whether the files provided match
 * with the technology format for the files
 * @param filename1 fastq files
 * @param filename2
 * @param index the pair number if there are more than one filepair
 */
void computeResults::checkFastQContent(QString filename1, QString filename2, int index) {
    QString technology = projectData.technology;
    size_t seqLength;

    if(technology == "10xv3") {
        seqLength = 28;
    }
    else if(technology == "10xv2") {
        seqLength = 26;
    }
    else if(technology == "SureCell") {
        seqLength = 59;
    }
    else if(technology == "DropSeq") {
        seqLength = 20;
    }
    else if(technology == "CELSeq2") {
        seqLength = 6;
    }
    else if(technology == "CELSeq") {
        seqLength = 12;
    }
    else if(technology == "inDrops") {
        seqLength = 48;
    }
    else {
        seqLength = 16;
    }

    gzFile fp1;
    gzFile fp2;
    kseq_t *seq1;
    kseq_t *seq2;
    QByteArray temp1 = filename1.toLocal8Bit();
    QByteArray temp2 = filename2.toLocal8Bit();
    const char *c_str1 = temp1.data();
    const char *c_str2 = temp2.data();
    fp1 = gzopen(c_str1, "r");
    fp2 = gzopen(c_str2, "r");
    seq1 = kseq_init(fp1);
    seq2 = kseq_init(fp2);

    for(int i = 0; i < 100; i++) {
        kseq_read(seq1);
        kseq_read(seq2);

        char* instance = seq1->name.s;

        char* instance2 = seq2->name.s;
        char const *compare1 = "/1";
        char const *compare2 = "/2";

        if(strcmp(&instance[(seq1->name.l)-2], compare1) == 0 && strcmp(&instance2[(seq2->name.l)-2], compare2) == 0) {
            char* subinstance = new char[(seq1->name.l)-2]();
            strncpy(subinstance, instance, (seq1->name.l)-3);

            char* subinstance2 = new char[(seq2->name.l)-2]();
            strncpy(subinstance2, instance2, (seq2->name.l)-3);

            if(strcmp(subinstance, subinstance2) != 0) {
                errorList.append("The names of the sequences between files " + QString::number(index) + " and " + QString::number(index+1) + " do not match, check data");
                break;
             }
        } else {
            if(strcmp(instance, instance2) != 0) {
                errorList.append("The names of the sequences between files " + QString::number(index) + " and " + QString::number(index+1) + " do not match, check data");
                break;
             }
        }
        if(seq1->seq.l != seqLength) {
            errorList.append("The sequence length in is not valid in file " + QString::number(index) +", check the data");
            break;
        }
        if(seq2->seq.l < seq1->seq.l) {
            errorList.append("Sequences is shorter than in file: " + QString::number(index) + ", check data");
            break;
        }
    }
}

/**
 * @brief calculateResults::createNewFastq enables the user to add more fastq file fields if they need them
*/
void computeResults::createNewFastq() {
    numberOfFastQFiles++;
    QLabel* fileLabel = new QLabel();
    fileLabel->setText("File " + QString::number(numberOfFastQFiles) + ":");
    QTextEdit* fileField = new QTextEdit();
    QPushButton* fileButton = new QPushButton();
    fileButton->setText("Select file");
    fastQFiles->addButton(fileButton);
    fastQFiles->setId(fileButton, numberOfFastQFiles);
    fileField->setObjectName("fileField" + QString::number(numberOfFastQFiles));
    fileField->setMaximumSize(550, 30);

    ui->scrollAreaWidgetContents_2->layout()->addWidget(fileLabel);
    ui->scrollAreaWidgetContents_2->layout()->addWidget(fileField);
    ui->scrollAreaWidgetContents_2->layout()->addWidget(fileButton);
}

/**
 * @brief calculateResults::doCalculations call kallisto and bustools
 */
void computeResults::doCalculations() {
    if(workflowCombo->currentIndex() == 1) {
        this->performCorrectFunctionCalls();
    }
    if(workflowCombo->currentIndex() == 2) {
        this->performFunctionCalls();
    }
    if(workflowCombo->currentIndex() == 3) {
        //this->performQuantFunctionCalls();
    }
}

/**
 * @brief calculateResults::errorCheck check file fields, first whether they are present
 * and than if their content is legal
 */
void computeResults::errorCheck() {
    bool marked = false;

    for(int i = 1; i < numberOfFastQFiles+1; i = i+2) {
        QString textEditName1 = "fileField" + QString::number(i);
        QTextEdit* textField1 = this->findChild<QTextEdit *>(textEditName1);
        QString fileName1 = textField1->toPlainText();

        QString textEditName2 = "fileField" + QString::number(i+1);
        QTextEdit* textField2 = this->findChild<QTextEdit *>(textEditName2);
        QString fileName2 = textField2->toPlainText();

        if((fileName1.isEmpty() || fileName2.isEmpty()) && !marked) {
            errorList.append("Please select input files for calculation");
            marked = true;
        } else {
            this->checkFastQContent(fileName1, fileName2, i);
        }
    }

    if(workflowCombo->currentIndex() == 0) {
        errorList.append("Please select a bustools workflow method");
    }

    if(workflowCombo->currentIndex() == 1) {
        if((ui->whitelistField->toPlainText()).isEmpty()) {
            errorList.append("Please provide a whitelist for the bustools correct calculations");
        }
    }

}

/**
 * @brief calculateResults::folder_button_clicked opens a file directory to change the
 * current output folder
 */
void computeResults::folder_button_clicked() {
    QString folder_name = QFileDialog::getExistingDirectory(this, "Select output folder", QDir::homePath());
    ui->outputFolderField->setText(folder_name);
}

/**
 * @brief newProject::on_addFilePair_clicked create new fastq file pair
 */
void computeResults::on_addFilePair_clicked()
{
    if(projectData.technology == "10xv1") {
        this->createNewFastq();
        this->createNewFastq();
        this->createNewFastq();
    } else {
        this->createNewFastq();
        this->createNewFastq();
    }
}

void computeResults::on_snakemake_button_clicked() {
    ui->snakemake_button->setEnabled(false);

    if(workflowCombo->currentIndex() == 1) {
        // generate the snakefile by substitutions
        QString snakemake;

        QString snakemake_template = "THREADS = 1\n"
        "BUSTOOLS_BIN = \"%1\"\n"
        "KALLISTO_BIN = \"%2\"\n"
        "INDEX = \"%3\"\n"
        "OUTPUT_DIRECTORY = \"%4\"\n"
        "TR2G = \"%5\"\n"
        "WHITELIST = \"%8\""
        "\n"
        "rule all:\n"
        "    input:\n"
        "        OUTPUT_DIRECTORY + \"/bus_output/genecount/genes.mtx\"\n"
        "\n"
        "rule kallisto_bus:\n"
        "    input:\n"
        "        \"%6\",\n"
        "        \"%7\",\n"
        "        INDEX\n"
        "    output:\n"
        "        OUTPUT_DIRECTORY + \"/bus_output\",\n"
        "        OUTPUT_DIRECTORY + \"/bus_output/output.bus\",\n"
        "        OUTPUT_DIRECTORY + \"/bus_output/matrix.ec\",\n"
        "        OUTPUT_DIRECTORY + \"/bus_output/transcripts.txt\"\n"
        "    shell:\n"
        "        KALLISTO_BIN + \" bus \"\n"
        "        \"-i {INDEX} \"\n"
        "        \"-o {output[0]} \"\n"
        "        \"-x, 10xv2 \"\n"
        "        \"{input[0]} {input[1]}\"\n"
        "\n"
        "rule bus_count:\n"
        "    input:\n"
        "        OUTPUT_DIRECTORY + \"/bus_output\",\n"
        "        OUTPUT_DIRECTORY + \"/bus_output/output.bus\",\n"
        "        OUTPUT_DIRECTORY + \"/bus_output/matrix.ec\",\n"
        "        OUTPUT_DIRECTORY + \"/bus_output/transcripts.txt\"\n"
        "    output:\n"
        "        OUTPUT_DIRECTORY + \"/bus_output/genecount/\",\n"
        "        OUTPUT_DIRECTORY + \"/bus_output/genecount/genes.mtx\"\n"

        "    threads:\n"
        "        int(THREADS)\n"
        "    shell:\n"
        "        BUSTOOLS_BIN + \" correct \"\n"
        "        \"-w {WHITELIST}\"\n"
        "        \"-p {input[1]} | \"\n"
        "        \"\" + BUSTOOLS_BIN + \" sort \"\n"
        "        \"-T {input[0]}/tmp \"\n"
        "        \"-t {threads}\"\n"
        "        \"-p - | \"\n"
        "        \"\" + BUSTOOLS_BIN + \" count \"\n"
        "        \"-o {output[0]} \"\n"
        "        \"-g {TR2G} \"\n"
        "        \"-e {input[2]} \"\n"
        "        \"-t {input[3]} \"\n"
        "        \"--genecounts -\"\n";
        //"bus, kal, index, output, tr2g, r1,r2,whitelist"

        if(projectData.resultDirectory != ui->outputFolderField->toPlainText()) {
            QString newOutputFolder = ui->outputFolderField->toPlainText();

            projectData.resultDirectory = ui->outputFolderField->toPlainText();
            connect(db, SIGNAL(outputUpdateDone()), this, SLOT(doCalculations()));
            db->updateOutputFolder(projectData.name, projectData.resultDirectory);
        }

        QString outputDir = QDir::toNativeSeparators(projectData.resultDirectory);
        QString r1f = QDir::toNativeSeparators(this->findChild<QTextEdit *>("fileField1")->toPlainText().trimmed());
        QString r2f = QDir::toNativeSeparators(this->findChild<QTextEdit *>("fileField2")->toPlainText().trimmed());
        QString whitelist = QDir::toNativeSeparators(ui->whitelistField->toPlainText().trimmed());
        QString tr2g = QDir::toNativeSeparators(ui->transcripts_to_genes_field->toPlainText().trimmed());
        snakemake = snakemake_template.arg(bustoolsPath, kallistoPath,QDir::toNativeSeparators(projectData.indexDirectory),outputDir,tr2g,r1f,r2f,whitelist );

        QString sfn = QDir::toNativeSeparators(outputDir + "/Snakefile");

        QFile sf(sfn);
        if (sf.open(QIODevice::ReadWrite)) {
            QTextStream stream( &sf );
            stream << snakemake << endl;
            sf.close();
            ui->statusField->setStyleSheet("color: white; font-size: 12px; background-color: black;");
            ui->statusField->setHtml("<p>Snakefile written to " + sfn + "</p>");
        } else {
            ui->statusField->setStyleSheet("color: Red; font-size: 13px; background-color: black;");
            ui->statusField->setHtml("<p>Error: could not write to file " + sfn + "</p>");
        }
    } else {
        ui->statusField->setStyleSheet({"color:red; font-size: 13px;background-color:black"});
        ui->statusField->setHtml("<h3>Error: Snakefile generation only supported for the default workflow</h3>");
    }


    ui->snakemake_button->setEnabled(true);
}

/**
 * @brief calculateResults::on_result_button_clicked calculate results from the fastq
 * files provided or returns errors generated
 */
void computeResults::on_result_button_clicked()
{
    ui->result_button->setEnabled(false);
    if(projectData.technology != "10xv1") {
        this->errorCheck();
    } else {
        this->threeFileErrorCheck();
    }
    if(errorList.isEmpty()) {
        if(projectData.resultDirectory != ui->outputFolderField->toPlainText()) {
            QString newOutputFolder = ui->outputFolderField->toPlainText();

            projectData.resultDirectory = ui->outputFolderField->toPlainText();
            db->updateOutputFolder(projectData.name, projectData.resultDirectory);
        } else {
            if(workflowCombo->currentIndex() == 1) {
                this->performCorrectFunctionCalls();
            }
            if(workflowCombo->currentIndex() == 2) {
                this->performFunctionCalls();
            }
            if(workflowCombo->currentIndex() == 3) {
                //this->performQuantFunctionCalls();
            }
        }
    } else {
        QString result;
        ui->statusField->setStyleSheet({"color:red; font-size: 13px;background-color:black"});

        result = "<ul><h3>The following errors where generated:</h3>";
        foreach(QString error, errorList) {
             result = result + "<li>" + error + "</li>";
        }
        result = result + "</ul>";
        ui->statusField->setHtml(result);
        errorList.clear();
    }
    ui->result_button->setEnabled(true);
}

/**
 * @brief calculateResults::select_file_clicked reacting to file button clicked in ui
 * @param button
 */
void computeResults::select_file_clicked(QAbstractButton* button)
{
    QString filter = "Select file (*)";
    QString file_name = QFileDialog::getOpenFileName(this, "Select file", currentPath.path(), "*");

    if(button->objectName() == "whitelist_button") {
        ui->whitelistField->setText(file_name);
    } else {
        ui->transcripts_to_genes_field->setText(file_name);
    }

}

/**
 * @brief calculateResults::performCorrectFunctionCalls performs function calls if bustools correct->sort->count workflow is selected
 */
void computeResults::performCorrectFunctionCalls() {
    ui->statusField->setHtml("<h2> Performing calculation...</h2>");
    ui->statusField->setStyleSheet("color:white; background-color:black");
    QString statusMessage;

    qRegisterMetaType<QList<functionValues>>();
    QList< functionValues > functions;

    QStringList argumentList;
    QString  outputPath = projectData.resultDirectory + "/bus_output";
    outputPath = QDir::toNativeSeparators(outputPath);
    argumentList << "bus" << "-i" << projectData.indexDirectory << "-o" << outputPath << "-x" << projectData.technology;
    for(int i = 1; i < numberOfFastQFiles+1; i++) {
        QString file = "fileField" + QString::number(i);
        QTextEdit* textField = this->findChild<QTextEdit *>(file);
        QString fileName = textField->toPlainText();
        argumentList << QDir::toNativeSeparators(fileName);
    }

    functionValues kallistoCall;
    kallistoCall.program = kallistoPath;
    kallistoCall.arguments = argumentList;
    functions.append(kallistoCall);

    argumentList.clear();

    QString genecount =  projectData.resultDirectory + "/bus_output/genecount/";
    QString temp = projectData.resultDirectory + "/bus_output/tmp/";


    functionValues makeDir;
    makeDir.program = "mkdir";
    argumentList << QDir::toNativeSeparators(genecount) << QDir::toNativeSeparators(temp);
    makeDir.arguments = argumentList;
    functions.append(makeDir);

    functionValues bustoolsCorrect;
    QString pArgument = projectData.resultDirectory + "/bus_output/output.bus";
    pArgument =  QDir::toNativeSeparators(pArgument);
    bustoolsCorrect.program = bustoolsPath + " correct -w " + ui->whitelistField->toPlainText() + " -p " + pArgument;

    bustoolsCorrect.arguments = argumentList;
    functions.append(bustoolsCorrect);

    argumentList.clear();

    functionValues bustoolsSort;
    bustoolsSort.program = bustoolsPath + " sort -T " + temp + " -t 4 -p -";
    bustoolsSort.arguments = argumentList;
    functions.append(bustoolsSort);

    argumentList.clear();

    functionValues bustoolsCount;
    QString genes = projectData.resultDirectory + "/bus_output/genecount/genes";
    genes = QDir::toNativeSeparators(genes);
    QString matrix = projectData.resultDirectory + "/bus_output/matrix.ec";
    matrix = QDir::toNativeSeparators(matrix);
    QString transcripts = projectData.resultDirectory + "/bus_output/transcripts.txt";
    transcripts = QDir::toNativeSeparators(transcripts);
    bustoolsCount.program = bustoolsPath + " count -o " + genes + " -g " + ui->transcripts_to_genes_field->toPlainText() + " -e " + matrix + " -t " + transcripts + " --genecounts " + "-";
    bustoolsCount.arguments = argumentList;
    functions.append(bustoolsCount);


    connect(this, SIGNAL(performCorrectCalls(QList <functionValues>)), &calc, SLOT(startPipingProcesses(QList <functionValues>)));
    connect(&calc, SIGNAL(calcFinished()), this, SLOT(updateStatusLayout()));

    calcThread->start(QThread::HighestPriority);

    emit performCorrectCalls(functions);
}

/**
 * @brief calculateResults::performFunctionCalls calculates the results for the output if bustools sort->text is chosen
 */
void computeResults::performFunctionCalls() {
    ui->statusField->setHtml("<h2> Performing calculation...</h2>");
    ui->statusField->setStyleSheet("color:white; background-color:black; font-family: monospace; font-size: 12pt");

    qRegisterMetaType<QList<functionValues>>();
    QList< functionValues > functions;

    QStringList busArguments;
    QString busOutput = projectData.resultDirectory + "/bus_output";
    busArguments << "bus" << "-i" << projectData.indexDirectory << "-o" << QDir::toNativeSeparators(busOutput) << "-x" << projectData.technology;
    for(int i = 1; i < numberOfFastQFiles+1; i++) {
        QString file = "fileField" + QString::number(i);
        QTextEdit* textField = this->findChild<QTextEdit *>(file);
        QString fileName = textField->toPlainText();
        busArguments << QDir::toNativeSeparators(fileName);
    }
    functionValues kallistoCall;
    kallistoCall.program = kallistoPath;
    kallistoCall.arguments = busArguments;
    functions.append(kallistoCall);

    QStringList bustoolsSortArguments;
    QString outputSortDestination = projectData.resultDirectory + "/bus_output/output.sort.bus";
    outputSortDestination = QDir::toNativeSeparators(outputSortDestination);
    QString inputSortFile = projectData.resultDirectory + "/bus_output/output.bus";
    inputSortFile = QDir::toNativeSeparators(inputSortFile);
    bustoolsSortArguments << "sort" << "-t" << "4" << "-o" <<  outputSortDestination << inputSortFile;

    functionValues bustoolsSortCall;
    bustoolsSortCall.program = bustoolsPath;
    bustoolsSortCall.arguments = bustoolsSortArguments;
    functions.append(bustoolsSortCall);

    QStringList bustoolTextArguments;
    QString outputTextArgument = projectData.resultDirectory + "/bus_output/output.sort.txt";
    outputTextArgument = QDir::toNativeSeparators(outputTextArgument);
    QString inputTextFile = projectData.resultDirectory + "/bus_output/output.sort.bus";
    inputTextFile = QDir::toNativeSeparators(inputTextFile);
    bustoolTextArguments << "text" << "-o" <<  outputTextArgument << inputTextFile;

    functionValues bustoolsTextCall;
    bustoolsTextCall.program = bustoolsPath;
    bustoolsTextCall.arguments = bustoolTextArguments;
    functions.append(bustoolsTextCall);

    connect(this, SIGNAL(performCalls(QList <functionValues>)), &calc, SLOT(startProcesses(QList <functionValues>)));
    connect(&calc, SIGNAL(calcFinished()), this, SLOT(updateStatusLayout()));

    calcThread->start(QThread::HighestPriority);

    emit performCalls(functions);
}

/**
 * @brief calculateResults::performQuantFunctionCalls performs function calls for kallisto quant
 */
void computeResults::performQuantFunctionCalls() {
    ui->statusField->setHtml("<h2> Performing calculation...</h2>");
    ui->statusField->setStyleSheet("color:white; background-color:black");

    qRegisterMetaType<QList<functionValues>>();
    QList< functionValues > functions;

    functionValues quantCall;
    quantCall.program = kallistoPath;

    QStringList quantArguments;
    QString quant = projectData.resultDirectory + "/quant_output";
    quantArguments << "quant" << "-i" << projectData.indexDirectory << "-o" << QDir::toNativeSeparators(quant);
    for(int i = 1; i < numberOfFastQFiles+1; i++) {
        QString file = "fileField" + QString::number(i);
        QTextEdit* textField = this->findChild<QTextEdit *>(file);
        QString fileName = textField->toPlainText();
        quantArguments << fileName;
    }
    quantCall.arguments = quantArguments;

    functions.append(quantCall);

    connect(this, SIGNAL(performCalls(QList <functionValues>)), &calc, SLOT(startProcesses(QList <functionValues>)));
    connect(&calc, SIGNAL(calcFinished()), this, SLOT(updateStatusLayout()));

    calcThread->start(QThread::HighestPriority);

    emit performCalls(functions);
}

/**
 * @brief calculateResults::populateWorkflow creates dropdown list for workflow for the calculations to be selected
 */
void computeResults::populateWorkflow() {
    workflowCombo->addItem("Select workflow");
    workflowCombo->addItem("kallisto bus -> bustools correct->sort->count");
    //workflowCombo->addItem("kallisto bus bustools sort->text");
    //workflowCombo->addItem("kallisto quant");

    workflowCombo->setCurrentIndex(1);
    ui->workflow_grid->addWidget(workflowCombo, 0, 0, Qt::AlignLeft);

    connect(workflowCombo, SIGNAL(activated(int)), this, SLOT(resetInputLayout(int)));

    workflowCombo->setMinimumContentsLength(30);
}

/**
 * @brief calculateResults::resetInputLayout emptying layout field when a different workflow is selected
 * @param index
 */
void computeResults::resetInputLayout(int index) {
    if(index != 0) {
        ui->whitelistField->clear();
        ui->transcripts_to_genes_field->clear();

        for(int i = 1; i < numberOfFastQFiles+1; i++) {
            QString file = "fileField" + QString::number(i);
            QTextEdit* textField = this->findChild<QTextEdit *>(file);
            textField->clear();
        }
    }

    QLayoutItem* child;
    while((child = ui->scrollAreaWidgetContents_2->layout()->takeAt(3)) != nullptr) {
          delete child->widget();
    }
    numberOfFastQFiles = 1;

    this->createNewFastq();

    if(projectData.technology == "10xv1") {
        this->createNewFastq();
    }
}

/**
 * @brief calculateResults::setPathInfo get the information about the kallisto and bustools paths
 * @param paths the information collected from the database
 */
void computeResults::setPathInfo(QStringList paths) {
    kallistoPath = paths[0];
    bustoolsPath = paths[1];

    if(kallistoPath.isEmpty() || bustoolsPath.isEmpty()) {
        ui->statusField->setHtml("<h3>Kallisto can't be found, please install and try again</h3>");
        ui->statusField->setStyleSheet("color:red; background-color:black");
    }
}

/**
 * @brief calculateResults::showIndexInfo general information about the projects index file and other dependencies
 * @param results
 */
void computeResults::showIndexInfo(QStringList results) {
    QString projectOverviewInfo = "<h2> Project overview: </h2>";

    QString indexName = " The name of the index file is: " + results[0];
    QString indexDirectory = " The directory for the index is: " + results[3];
    QString technology = " The technology used in these calculations is: " + projectData.technology;

    projectOverviewInfo = projectOverviewInfo + "<ul><li>"  + indexName + "</li>";
    projectOverviewInfo = projectOverviewInfo + "<li>" + indexDirectory + "</li>";
    projectOverviewInfo = projectOverviewInfo + "<li>" + technology + "</li></ul>";

    ui->projectOverview->setReadOnly(true);
    ui->projectOverview->setHtml(projectOverviewInfo);
}

void computeResults::showIntermediateState(QString message) {
    ui->statusField->setHtml(message);
}

/**
 * @brief calculateResults::threeFileErrorCheck special check for 10xv1 tecnhology,
 * since the file pairs contain three files
 */
void computeResults::threeFileErrorCheck() {
    bool marked = false;
    for(int i = 1; i < numberOfFastQFiles+2; i = i+3) {
        QString textEditName1 = "fileField" + QString::number(i);
        QTextEdit* textField1 = this->findChild<QTextEdit *>(textEditName1);
        QString fileName1 = textField1->toPlainText();

        QString textEditName2 = "fileField" + QString::number(i+1);
        QTextEdit* textField2 = this->findChild<QTextEdit *>(textEditName2);
        QString fileName2 = textField2->toPlainText();

        QString textEditName3 = "fileField" + QString::number(i+2);
        QTextEdit* textField3 = this->findChild<QTextEdit *>(textEditName3);
        QString fileName3 = textField3->toPlainText();

        if((fileName1.isEmpty() || fileName2.isEmpty() || fileName3.isEmpty()) && !marked) {
            errorList.append("Please select input files for calculation");
            marked = true;
        }
    }
}

/**
 * @brief calculateResults::updateErrorLayout display errors during the calculations
 * @param message
 */
void computeResults::updateErrorLayout(QString message) {
    message = "<pre>" + message + "</pre>";
    allOutput = allOutput + message;
    ui->statusField->setStyleSheet("color:red;  background-color:black");
    ui->statusField->setHtml(message);
}

/**
 * @brief calculateResults::updateLayout changes the information in the layout if the project has been changes
 * @param newName
 */
void computeResults::updateLayout(QString newName) {
    connect(db, SIGNAL(projectInfoCollected(QStringList)), currProject, SLOT(generateProjectDetails(QStringList)));
    db->getProjectInformation(newName);

    projectData = currProject->provideProjectInfo();

    ui->textEdit_4->clear();
    ui->statusField->clear();

    QLayoutItem* child;
    while((child = ui->scrollAreaWidgetContents_2->layout()->takeAt(3)) != nullptr) {
          delete child->widget();
    }
    numberOfFastQFiles = 1;

    this->createNewFastq();

    if(projectData.technology == "10xv1") {
        this->createNewFastq();
    }
    ui->outputFolderField->setText(projectData.resultDirectory);

    this->setFixedSize(this->width(), this->height());
}

/**
 * @brief calculateResults::updateOutputLayout show new output when it is ready
 * @param message
 */
void computeResults::updateOutputLayout(QString message) {
    message = "<pre>" + message + "</pre>";
    allOutput = allOutput + message;
    ui->statusField->setStyleSheet("color:white;  background-color:black");
    ui->statusField->setHtml(allOutput);
}

/**
 * @brief calculateResults::updateStatusLayout calculations are finished, signals are disconnected and status field updated
 */
void computeResults::updateStatusLayout() {
    calcThread->quit();
    disconnect(this, SIGNAL(performCalls(QList <functionValues>)), &calc, SLOT(startProcesses(QList <functionValues>)));
    disconnect(&calc, SIGNAL(calcFinished()), this, SLOT(updateStatusLayout()));
    disconnect(this, SIGNAL(performCorrectCalls(QList <functionValues>)), &calc, SLOT(startPipingProcesses(QList <functionValues>)));

    db->addOutput(projectData.name, allOutput);
    allOutput = "";
}

/**
 * @brief calculateResults::on_view_project_history_clicked show the current projects run history
 */
void computeResults::view_clicked(QAbstractButton* button)
{
    ViewProjectHistory* viewProject;
    if(button->objectName() == "view_project_history") {
        viewProject = new ViewProjectHistory(this, projectData.name, db, true);
    } else {
        viewProject = new ViewProjectHistory(this, projectData.indexDirectory, db, false);
    }

    viewProject->show();
}
