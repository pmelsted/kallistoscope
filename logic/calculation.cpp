#include "calculation.h"
#include "../ui/mainwindow.h"
#include <QProcess>
#include <QDebug>
#include <QDir>

struct outputInfo {
   QString output; //the standard output from the command line call
   bool errors;    //provides information whether it is standard output or standard errors
};

static outputInfo output;

/**
 * @brief calculation::calculation performs most of the work concerning function calls for kallisto and bustools
 * and file placement string manipulation
 */
calculation::calculation()
{
    process = new QProcess(this);
    connect(process, SIGNAL(readyReadStandardOutput()), SLOT(outputChanged()));
    connect(process, SIGNAL(readyReadStandardError()), SLOT(errorAdded()));
}

/**
 * @brief calculation::errorAdded new error in output
 */
void calculation::errorAdded() {
    QString error(process->readAllStandardError());
    emit newError(error);
}

/**
 * @brief calculation::findOutputField searches for the filename inside the directory provided
 * @param fileName
 * @return
 */
QString calculation::findOutputField(QString fileName, bool dir) {
    QString splitValue;
    int splitIndex = 0;
    for(int i = 0; i < fileName.length(); i++) {
        if(fileName[i] == "/") {
            splitValue = "";
            splitIndex = i;
        } else {
            splitValue = splitValue + fileName[i];
        }
    }
    QString indexDir = fileName.mid(0, splitIndex);
    if(dir) {
        return indexDir;
    } else {
        return splitValue;
    }
}

void calculation::outputChanged() {
    QString output(process->readAllStandardOutput());
    emit newOutput(output);
}

/**
 * @brief calculation::startProcess call the command line and request a calculation
 * @param program
 * @param arguments
 */
void calculation::pathProcess() {
    QProcess shProcess;
    process->setProcessChannelMode(QProcess::MergedChannels);
    // TODO: port to windows
    shProcess.start("sh", QStringList()<<"-c" << "PATH=/usr/local/bin:$PATH; IFS=':'; find $PATH -name kallisto 2>/dev/null;");
    shProcess.waitForFinished(-1);
    QString kallistoPath(shProcess.readAllStandardOutput());
    QString error(shProcess.readAllStandardError());
    shProcess.close();

    shProcess.start("sh", QStringList()<< "-c" << "PATH=/usr/local/bin:$PATH; IFS=':'; find $PATH -name bustools 2>/dev/null");
    shProcess.waitForFinished(-1);
    QString bustoolsPath(shProcess.readAllStandardOutput());
    QString bustoolsError(shProcess.readAllStandardError());
    shProcess.close();

    emit pathInfoReady(kallistoPath, bustoolsPath);

    shProcess.deleteLater();
}
/**
 * @brief calculation::debugCorrectError errors from the bustools correct call
 */
void calculation::debugCorrectError() {
    QString output = correctProcess->readAllStandardError();
    emit newOutput(output);
}

/**
 * @brief calculation::debugSortError errors from the bustools sort call
 */
void calculation::debugSortError() {
    QString output = sortProcess->readAllStandardError();
    emit newOutput(output);
}

/**
 * @brief calculation::debugCountError errors from the bustools count call
 */
void calculation::debugCountError() {
    QString output = countProcess->readAllStandardError();
    emit newOutput(output);
}

/**
 * @brief calculation::startPipingProcesses bustools correct->sort->count for piping output as input
 * to the next one process
 * @param processes
 */
void calculation::startPipingProcesses(QList<functionValues> processes) {
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start(processes[0].program, processes[0].arguments);
    process->waitForFinished(-1);

    process->close();

    QString genecount =  processes[1].arguments[0];
    QString temp = processes[1].arguments[1];

    QDir().mkdir(genecount);
    QDir().mkdir(temp);

    correctProcess = new QProcess(this);
    sortProcess = new QProcess(this);
    countProcess = new QProcess(this);

    correctProcess->setStandardOutputProcess(sortProcess);
    sortProcess->setStandardOutputProcess(countProcess);

    connect(correctProcess, SIGNAL(readyReadStandardError()), SLOT(debugCorrectError()));
    connect(sortProcess, SIGNAL(readyReadStandardError()), SLOT(debugSortError()));
    connect(countProcess, SIGNAL(readyReadStandardError()), SLOT(debugCountError()));

    correctProcess->start(processes[2].program);
    sortProcess->start(processes[3].program);
    countProcess->start(processes[4].program);
    correctProcess->waitForFinished(-1);
    sortProcess->waitForFinished(-1);
    countProcess->waitForFinished(-1);

    delete correctProcess;
    delete sortProcess;
    delete countProcess;

    emit calcFinished();
}

/**
 * @brief calculation::startProcess call the command line and request a calculation
 * @param program
 * @param arguments
 */
void calculation::startProcess(QString program, QStringList arguments) {
    process->setProcessChannelMode(QProcess::MergedChannels);
    process->start(program, arguments);
    process->waitForFinished(-1);

    emit indexCalcFinished();
}

/**
 * @brief calculation::startProcesses performs bus command and bustools sorting/text
 * Performed on a seperate thread for optimization
 * @param functionCalls the program and arguments pairs to be performed
 */
void calculation::startProcesses(QList <functionValues> functionCalls) {
    QStringList resultValues;
    resultValues.append(QDateTime::currentDateTime().toString());

    foreach(functionValues val, functionCalls) {
        process->setProcessChannelMode(QProcess::MergedChannels);
        process->start(val.program, val.arguments);
        process->waitForFinished(-1);

        QString output(process->readAllStandardOutput());
        QString errors(process->readAllStandardError());

        if(errors.isEmpty()) {
            resultValues.append(output);
        } else {
            resultValues.append(errors);
        }

        process->close();
    }
    emit calcFinished();
}
