#ifndef DATABASE_H
#define DATABASE_H

#include <QString>
#include <QObject>
#include <QSqlQuery>
#include <QJsonArray>

struct speciesInfo {
    QString name;   //the academic name of the species for example homo_sapiens
    QString displayName;    //the more common name of the species for example Human
    QString assembly;   //a string used in the fasta file on the site
};

struct runInfo {
    QString timestamp;
    QString output;
};

class Database: public QObject
{
    Q_OBJECT
private slots:
    void deletionCompleted();
signals:
    void breadcrumbs(QStringList);
    void dataObtained(QStringList);
    void deletionFailure();
    void error(QString);
    void indexAdded();
    void indexDirectories(QStringList);
    void indexInfo(QStringList);
    void indexInfoCollected(QStringList);
    void indexInfoCollectedThroughDirectory(QStringList);
    void outputUpdateDone();
    void outputReady(QList<runInfo>);
    void pathAdded();
    void pathInfo(QStringList);
    void projectAdded();
    void projectsDeleted();
    void projectInfoCollected(QStringList);
    void projectUpdated();
    void runHistoryDeleted();
    void speciesInfoReady(QList < speciesInfo >, QString);
    void worked();
public:
    Database();
    void addBreadcrumb(QString);
    void addOutput(QString, QString);
    void createNewProject(QString name, QString indexDirectory, QString resultDirectory, QString technology, QString fastaDirectory);
    void deleteHistory(QStringList);
    void deleteSpeciesInfo();
    void getAllProjects();
    void getBreadcrumbs();
    void getIndexDirectories();
    void getIndexNames();
    void getIndexInfo(QString);
    void getIndexInfoThroughDirectory(QString);
    void getOutput(QString);
    void getPath(QStringList names);
    void getProjectInformation(QString name);
    void getSpecies();
    void insertPath(QString name, QString path);
    void insertIndexInfo(QString name, QString description, QString fasta, QString index, QString fastaChoice, QString fetchUrl);
    void insertSpecies(QList< speciesInfo > info);
    void removeBreadcrumbs();
    void removePaths();
    void removeProjects(QStringList names);
    void updateOutputFolder(QString name, QString resultDirectory);
    void updateProject(QString oldName, QString nameUpdate, QString indexDirectory, QString resultDirectory, QString technology, QString fastaDirectory);
};

#endif // DATABASE_H
