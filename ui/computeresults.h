#ifndef COMPUTERESULTS_H
#define COMPUTERESULTS_H
#include <QWidget>
#include <QButtonGroup>
#include "../logic/database.h"
#include "../logic/project.h"
#include "../logic/calculation.h"
#include <QDir>
#include <QThread>
#include <QAbstractButton>
#include <QComboBox>

namespace Ui {
class computeResults;
}

class computeResults : public QWidget
{
    Q_OBJECT
    QString allOutput;
    QString bustoolsPath;
    calculation calc;
    QThread* calcThread = new QThread(this);
    project *currProject;
    void createNewFastq();
    QStringList errorList;
    QButtonGroup* fastQFiles = new QButtonGroup(this);
    QString kallistoPath;
    struct projectInfo projectData;
    int numberOfFastQFiles = 1;
public:
    explicit computeResults(QWidget *parent = nullptr, QString name = "", Database* databaseInstance = nullptr);
    ~computeResults();
private slots:
    void button_clicked(int);
    void doCalculations();
    void folder_button_clicked();
    void resetInputLayout(int);
    void on_addFilePair_clicked();
    void on_result_button_clicked();
    void on_snakemake_button_clicked();
    void select_file_clicked(QAbstractButton*);
    void setPathInfo(QStringList);
    void showIndexInfo(QStringList);
    void showIntermediateState(QString);
    void updateLayout(QString);
    void updateStatusLayout();
    void updateOutputLayout(QString);
    void updateErrorLayout(QString);
    void view_clicked(QAbstractButton*);
signals:
    void performCalls(QList <functionValues>);
    void performCorrectCalls(QList <functionValues>);
private:
    Ui::computeResults *ui;
    void checkFastQContent(QString filename1, QString filename2, int index);
    QDir currentPath;
    Database* db;
    void errorCheck();
    void performCorrectFunctionCalls();
    void performFunctionCalls();
    void performQuantFunctionCalls();
    void populateWorkflow();
    void threeFileErrorCheck();
    QComboBox *workflowCombo = new QComboBox;
};

#endif // COMPUTERESULTS_H
