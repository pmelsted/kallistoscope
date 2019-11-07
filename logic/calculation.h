#ifndef CALCULATION_H
#define CALCULATION_H
#include <QProcess>
#include "project.h"
#include <QThread>

Q_DECLARE_METATYPE(QList<functionValues>);

class calculation: public QObject
{
    Q_OBJECT
    QProcess* process;
    QProcess* correctProcess;
    QProcess* sortProcess;
    QProcess* countProcess;
signals:
    void calcFinished();
    void indexCalcFinished();
    void newOutput(QString);
    void newError(QString);
    void pathInfoReady(QString, QString);
    void statusInCalc(QString);
public slots:
    void debugCorrectError();
    void debugSortError();
    void debugCountError();
    void errorAdded();
    void outputChanged();
    void startPipingProcesses(QList <functionValues>);
    void startProcesses(QList <functionValues>);
    void startProcess(QString, QStringList);
public:
    calculation();
    QString findOutputField(QString fileName, bool dir);
    void pathProcess();
};

#endif // CALCULATION_H
