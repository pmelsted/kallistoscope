#ifndef PROJECT_H
#define PROJECT_H
#include "database.h"
#include <QString>
#include <QObject>

struct projectInfo {
    QString name;
    QString indexDirectory;
    QString resultDirectory;
    QString technology;
    QString fastaDirectory;
};

struct functionValues {
    QString program;
    QStringList arguments;
};

Q_DECLARE_METATYPE(functionValues);

class project: public QObject
{
    Q_OBJECT
    struct projectInfo currProject;
public:
    project(QString name, Database* db);
    projectInfo provideProjectInfo();

private slots:
   void generateProjectDetails(QStringList);
};

#endif // PROJECT_H
