#ifndef INDEXCALCULATION_H
#define INDEXCALCULATION_H

#include <QMainWindow>
#include <QThread>
#include <QComboBox>
#include "../logic/database.h"
#include "headerbar.h"
#include "../logic/filedownloads.h"
#include "../logic/calculation.h"

namespace Ui {
class indexCalculation;
}

class indexcreation : public QMainWindow
{
    Q_OBJECT
    QString allOutput;
    QWidget* ancestor;
    calculation calc;
    QThread* calculationThread = new QThread(this);
    Database* db;
    filedownloads downloads;
    QString downloadUrl;
    QString GENERAL_CURRENT_INDEX_DIRECTORY = "ftp://ftp.ensembl.org/pub/current_fasta/";
    QString GENERAL_CDNA = "/cdna/";
    QString GENERAL_FASTA_FILE_END = ".cdna.all.fa.gz";
    QString GENERAL_SELECTED_INDEX_DIRECTORY = "ftp://ftp.ensembl.org/pub/";
    headerBar* header;
    QString indexDestination;
    QString kallistoPath;
    QStringList releaseOptions;
    QComboBox *speciesComboBox = new QComboBox;
    QString speciesString;
public:
    explicit indexcreation(QWidget *parent = nullptr, Database* dbInstance=nullptr);
    ~indexcreation();
signals:
    void cancelled();
    void index_created(QString name);
    void performIndexCalc(QString, QStringList);
private slots:
    void calculateIndex();
    void checkDirectoryAvailability(QStringList);
    void checkNameAvailability(QStringList);
    void computeFailure(QString);
    void connectionError();
    void handleError(QString);
    void index_redirect();
    void on_calculateIndex_clicked();
    void on_cancel_clicked();
    void on_fastaFileSelect_clicked();
    void on_indexFolderButton_clicked();
    void populateSpecies(QList < speciesInfo >, QString);
    void removeReference();
    void responseReady(QNetworkReply* response);
    void setPathInfo(QStringList);
    void updateInfo();
    void updateOutputLayout(QString);
    void updateErrorLayout(QString);
private:
    Ui::indexCalculation *ui;
    void errorCheck();
    QStringList errorList;
};

#endif // INDEXCALCULATION_H
