#include "database.h"

#include <QString>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QDate>

/**
 * @brief Database::Database the database class, storing all the functions for database creation, insertion, changes and deletion
 */
Database::Database()
{
    QString databasePath = QDir::homePath() + "/.kallistoscope/application_information.sqlite";
    databasePath = QDir::toNativeSeparators(databasePath);

    QSqlDatabase db;
    if(QSqlDatabase::contains("projectConnection")) {
        db = QSqlDatabase::database("projectConnection");
    } else {
        db = QSqlDatabase::addDatabase("QSQLITE", "projectConnection");

        db.setDatabaseName(databasePath);
    }
    QSqlQuery query;
    query = QSqlQuery(db);
    db.open();

    if(!query.exec("CREATE TABLE IF NOT EXISTS projects ("
                   "name TEXT NOT NULL UNIQUE,"
                   "indexDirectory TEXT NOT NULL,"
                   "resultDirectory TEXT NOT NULL,"
                   "technology TEXT NOT NULL,"
                   "fastaDirectory text"
                  ");")) {
        emit error("Error creating table projects: "+ query.lastError().text());
    } else {
        emit worked();
    }

    QSqlDatabase db_paths;

    if(QSqlDatabase::contains("pathConnection")) {
        db_paths = QSqlDatabase::database("pathConnection");
    } else {
        db_paths = QSqlDatabase::addDatabase("QSQLITE", "pathConnection");
        db_paths.setDatabaseName(databasePath);
    }

    db_paths.open();

    QSqlQuery pathQuery;
    pathQuery = QSqlQuery(db_paths);

    if(!pathQuery.exec("CREATE TABLE IF NOT EXISTS paths ("
                    "name TEXT NOT NULL UNIQUE,"
                    "pathString TEXT NOT NULL"
                    ");")) {
        emit error("Error creating table projects: "+ pathQuery.lastError().text());
    } else {
        emit worked();
    }

    QSqlDatabase db_species;

    if(QSqlDatabase::contains("speciesDatabase")) {
        db_species = QSqlDatabase::database("speciesDatabase");
    } else {
        db_species = QSqlDatabase::addDatabase("QSQLITE", "speciesDatabase");

        db_species.setDatabaseName(databasePath);
    }
    db_species.open();

    QSqlQuery speciesQuery;
    speciesQuery = QSqlQuery(db_species);

    if(!speciesQuery.exec("CREATE TABLE IF NOT EXISTS speciesInformation ("
                    "name TEXT NOT NULL,"
                    "display_name TEXT NOT NULL,"
                    "assembly TEXT NOT NULL,"
                    "timeStamp DATE"
                    ");")) {
        emit error("Error creating table projects: "+ pathQuery.lastError().text());
    } else {
        emit worked();
    }

    QSqlDatabase db_index;
    if(QSqlDatabase::contains("indexDatabase")) {
        db_index = QSqlDatabase::database("indexDatabase");
    } else {
        db_index = QSqlDatabase::addDatabase("QSQLITE", "indexDatabase");

        db_index.setDatabaseName(databasePath);
    }

    db_index.open();

    QSqlQuery indexQuery;
    indexQuery = QSqlQuery(db_index);

    if(!indexQuery.exec("CREATE TABLE IF NOT EXISTS index_information("
                        "name TEXT NOT NULL,"
                        "description TEXT,"
                        "fastaFile TEXT,"
                        "indexDirectory TEXT,"
                        "fastaChoice TEXT,"
                        "fetchUrl TEXT"
                        ");")) {
         emit error("Error creating table index information: "+ pathQuery.lastError().text());
    } else {
         emit worked();
    }

    QSqlDatabase db_runHistory;
    if(QSqlDatabase::contains("runHistoryConnection" )) {
        db_runHistory = QSqlDatabase::database("runHistoryConnection");
    } else {
        db_runHistory =  QSqlDatabase::addDatabase("QSQLITE", "runHistoryConnection");

        db_runHistory.setDatabaseName(databasePath);
    }

    db_runHistory.open();

    QSqlQuery runHistoryQuery;
    runHistoryQuery = QSqlQuery(db_runHistory);

    if(!runHistoryQuery.exec("CREATE TABLE IF NOT EXISTS runHistoryInfo("
                             "projectName TEXT NOT NULL,"
                             "timestamp TEXT,"
                             "outputMessage TEXT"
                             ");")) {
        emit error("Error creating table index information: "+ runHistoryQuery.lastError().text());
    }  else {
        emit worked();
    }

    QSqlDatabase db_breadcrumbs;
    if(QSqlDatabase::contains("breadcrumbsConnection")) {
        db_breadcrumbs = QSqlDatabase::database("breadcrumbsConnection");
    } else {
        db_breadcrumbs = QSqlDatabase::addDatabase("QSQLITE", "breadcrumbsConnection");
        db_breadcrumbs.setDatabaseName(databasePath);
    }
    db_breadcrumbs.open();

    QSqlQuery breadcrumbsQuery;
    breadcrumbsQuery = QSqlQuery(db_breadcrumbs);

    if(!breadcrumbsQuery.exec("CREATE TABLE IF NOT EXISTS breadcrumbs("
                              "breadcrumb TEXT"
                              ");")) {
        emit error("Error creating table breadcrumbs: "+ breadcrumbsQuery.lastError().text());
    } else {
        emit worked();
    }


}

/**
 * @brief Database::addBreadcrumb add a new breadcrumb to the database
 * @param crumb
 */
void Database::addBreadcrumb(QString crumb) {
    QSqlDatabase db = QSqlDatabase::database("breadcrumbsConnection");

    QSqlQuery query;
    query = QSqlQuery(db);

    query.prepare("INSERT INTO breadcrumbs(breadcrumb) VALUES(:info)");
    query.bindValue(":info", crumb);

    if(!query.exec()) {
        emit error("Error adding breadcrumb:" + query.lastError().text());
    }
}

/**
 * @brief Database::addOutput add output to the run history of a project
 * @param projectName which project
 * @param message the output
 */
void Database::addOutput(QString projectName, QString message) {
    QSqlDatabase db = QSqlDatabase::database("runHistoryConnection");

    QString dateTime = QDateTime::currentDateTime().toString();

    QSqlQuery query;
    query = QSqlQuery(db);

    query.prepare("INSERT INTO runHistoryInfo(projectName, timestamp, outputMessage) VALUES(:projectName, :timestamp, :outputMessage)");
    query.bindValue(":projectName", projectName);
    query.bindValue(":timestamp", dateTime);
    query.bindValue(":outputMessage", message);

    if(!query.exec()) {
        emit error("Error creating runHistory instance:" + query.lastError().text());
    } else  {
        //qDebug() << "worked";
    }
}

/**
 * @brief Database::createNewProject adds a new project to the database
 * @param name the project name
 * @param indexDirectory the index from the fasta file calculations
 * @param resultDirectory the output folder for the results
 * @param technology the single cell technology being used
 * @param indexCalcChoice which option was used to calculate the index
 * @param species the species in the fasta file directory
 * @param releaseVersion the release version for the fasta file directory
 * @param fastaUrl the url provided by the user, if he selected that option
 * @param fastaDirectory the directory to the fasta file if he selected that option
 */
void Database::createNewProject(QString name, QString indexDirectory, QString resultDirectory, QString technology, QString fastaDirectory) {
    QSqlDatabase db = QSqlDatabase::database("projectConnection");

    QSqlQuery query;
    query = QSqlQuery(db);
    query.prepare("INSERT INTO projects(name, indexDirectory, resultDirectory, technology, fastaDirectory) VALUES(:name, :indexDirectory, :resultDirectory, :technology, :fastaDirectory)");
    query.bindValue(":name", name);
    query.bindValue(":indexDirectory", indexDirectory);
    query.bindValue(":resultDirectory", resultDirectory);
    query.bindValue(":technology", technology);;
    query.bindValue(":fastaDirectory", fastaDirectory);

    if(!query.exec()) {
        emit error("Error creating new project:" + query.lastError().text());
    } else  {
        emit projectAdded();
    }
}

/**
 * @brief Database::deleteHistory remove references to the run history for this project
 * @param names
 */
void Database::deleteHistory(QStringList names) {
    QSqlDatabase db = QSqlDatabase::database("runHistoryConnection");
    bool success = true;

    QSqlQuery query;
    query = QSqlQuery(db);
    query.prepare("DELETE FROM runHistoryInfo WHERE projectName = :name");

    foreach(QString name, names) {
        query.bindValue(":name", name);
        if(!query.exec()) {
            success = false;
        }
    }

    if(success) {
        emit runHistoryDeleted();
    } else {
        emit deletionFailure();
    }
}

/**
 * @brief Database::deleteSpeciesInfo deletes the project with the name in the parameter
 */
void Database::deleteSpeciesInfo() {
    QSqlDatabase db = QSqlDatabase::database("speciesDatabase");

    QSqlQuery query;
    query = QSqlQuery(db);
    query.prepare("DELETE FROM speciesInformation");

    if(!query.exec()) {
        qDebug() << "Error creating values:"<< query.lastError().text();
    } else {
       // qDebug() << "worked";
    }
}

/**
 * @brief Database::deletionCompleted both the project and the run history were successfully deleted
 */
void Database::deletionCompleted() {
    emit projectsDeleted();

}

/**
 * @brief Database::getAllProjects returns a list of all the projects
 */
void Database::getAllProjects() {
    QSqlDatabase db = QSqlDatabase::database("projectConnection");

    QSqlQuery query;
    query = QSqlQuery(db);
    QString name;
    QStringList projectList;
    query.prepare("SELECT name FROM projects");
    if(query.exec()) {
        if(query.first()) {
            name = query.value(0).toString();
            projectList.append(name);
            while(query.next()) {
                name = query.value(0).toString();
                projectList.append(name);
            }
        }
        emit dataObtained(projectList);
    } else {
        emit error(query.lastError().text());
    }
}

void Database::getBreadcrumbs() {
    QSqlDatabase db = QSqlDatabase::database("breadcrumbsConnection");
    QStringList breadcrumbHistory;

    QSqlQuery query;
    query = QSqlQuery(db);

    query.prepare("SELECT breadcrumb FROM breadcrumbs");

    if(!query.exec()) {
        emit error("Error inserting values:" + query.lastError().text());
    } else {
        while(query.next()) {
            QString result = query.value(0).toString();

            breadcrumbHistory.append(result);
        }
    }
    emit breadcrumbs(breadcrumbHistory);
}

/**
 * @brief Database::getIndexDirectories collect all index directories
 */
void Database::getIndexDirectories() {
    QSqlDatabase db = QSqlDatabase::database("indexDatabase");
    QStringList indexDirectoryList;

    QSqlQuery query;
    query = QSqlQuery(db);

    query.prepare("SELECT indexDirectory FROM index_information");

    if(!query.exec()) {
        emit error("Error inserting values:" + query.lastError().text());
    } else {
        while(query.next()) {
            QString result = query.value(0).toString();

            indexDirectoryList.append(result);
        }
    }
    emit indexDirectories(indexDirectoryList);
}

/**
 * @brief Database::getIndexInfo get information about this index based on its name
 * @param indexName
 */
void Database::getIndexInfo(QString indexName) {
    QSqlDatabase db = QSqlDatabase::database("indexDatabase");

    QSqlQuery query;
    query = QSqlQuery(db);
    QStringList indexInfo;

    query.prepare("SELECT * FROM index_information WHERE name = :name;");
    query.bindValue(":name", indexName);

    if(!query.exec()) {
        indexInfo.append(query.lastError().text());
    } else  {
        while(query.next()) {
            for(int i = 0; i < 6; i++) {
                QString result = query.value(i).toString();
                indexInfo.append(result);
            }
        }
    }
    emit indexInfoCollected(indexInfo);
}

/**
 * @brief Database::getIndexInfoThroughDirectory get information about this index based on its directory
 * @param indexDirectory
 */
void Database::getIndexInfoThroughDirectory(QString indexDirectory) {
    QSqlDatabase db = QSqlDatabase::database("indexDatabase");

    QSqlQuery query;
    query = QSqlQuery(db);
    QStringList indexInfo;

    query.prepare("SELECT * FROM index_information WHERE indexDirectory = :indexDirectory;");
    query.bindValue(":indexDirectory", indexDirectory);

    if(!query.exec()) {
        indexInfo.append(query.lastError().text());
    } else  {
        while(query.next()) {
            for(int i = 0; i < 6; i++) {
                QString result = query.value(i).toString();
                indexInfo.append(result);
            }
        }
    }
    emit indexInfoCollectedThroughDirectory(indexInfo);
}

/**
 * @brief Database::getIndexNames collect all the index file names
 */
void Database::getIndexNames() {
    QSqlDatabase db = QSqlDatabase::database("indexDatabase");
    QStringList indexVersionList;

    QSqlQuery query;
    query = QSqlQuery(db);

    query.prepare("SELECT name FROM index_information");

    if(!query.exec()) {
        emit error("Error getting index information:" + query.lastError().text());
    } else {
        while(query.next()) {
            QString result = query.value(0).toString();
            indexVersionList.append(result);
        }
    }
    emit indexInfo(indexVersionList);
}

/**
 * @brief Database::getOutput collect the output for a specific project
 * @param currProject
 */
void Database::getOutput(QString currProject) {
    QSqlDatabase db = QSqlDatabase::database("runHistoryConnection");
    QList<runInfo> results;

    QSqlQuery query;
    query = QSqlQuery(db);

    query.prepare("SELECT timestamp, outputMessage FROM runHistoryInfo WHERE projectName=:name");
    query.bindValue(":name", currProject);

    if(!query.exec()) {
        emit error("Error creating runHistory instance:" + query.lastError().text());
    } else  {
        while(query.next()) {
            runInfo runInstance;
            runInstance.timestamp = query.value(0).toString();
            runInstance.output = query.value(1).toString();
            results.append(runInstance);
        }
        emit outputReady(results);
    }
}

/**
 * @brief Database::getPath getting the path in the users computer for kallisto and bustools
 * to run the program
 * @param names
 */
void Database::getPath(QStringList names) {
    QSqlDatabase db = QSqlDatabase::database("pathConnection");

    QSqlQuery query;
    query = QSqlQuery(db);
    QStringList pathNameList;
    query.prepare("SELECT pathString FROM paths WHERE name = :name");

    foreach(QString name, names) {
        query.bindValue(":name", name);
        if(!query.exec()) {
            emit error("Error accessing values:" + query.lastError().text());
            return;
        } else  {
            if(query.first()) {
                query.first();
                pathNameList.append(query.value(0).toString());
            } else {
                pathNameList.append("");
            }
        }
    }
    emit pathInfo(pathNameList);
}

/**
 * @brief Database::getProjectInformation get the information of a project based on its name
 * @param name
 */
void Database::getProjectInformation(QString name) {
    QSqlDatabase db = QSqlDatabase::database("projectConnection");

    QSqlQuery query;
    query = QSqlQuery(db);
    QStringList projectList;

    query.prepare("SELECT * FROM projects WHERE name = ?");
    query.addBindValue(name);
    if(!query.exec()) {
        projectList.append(query.lastError().text());
    } else  {
        while(query.next()) {
            for(int i = 0; i < 5; i++) {
                QString result = query.value(i).toString();
                projectList.append(result);
            }
        }
    }
    emit projectInfoCollected(projectList);
}

/**
 * @brief Database::getSpecies collect species information from database along with timestamp for
 * the data collection date, if it is more than three months old it will  be deleted and collected
 * again from ensembl
 */
void Database::getSpecies() {
    QSqlDatabase db = QSqlDatabase::database("speciesDatabase");
    QStringList releaseVersionList;
    QString timestamp;
    QList < speciesInfo > results;
    speciesInfo result;

    QSqlQuery query;
    query = QSqlQuery(db);

    query.prepare("SELECT name, display_name, assembly FROM speciesInformation");

    if(!query.exec()) {
        emit error("Error inserting values:" + query.lastError().text());
    } else {
        while(query.next()) {
            result.name = query.value(0).toString();
            result.displayName = query.value(1).toString();
            result.assembly = query.value(2).toString();
            results.append(result);
        }
    }

    query.prepare("SELECT timeStamp FROM speciesInformation");

    if(!query.exec()) {
        emit error("Error inserting values:" + query.lastError().text());
    } else {
        query.first();
        timestamp = query.value(0).toString();
    }
    emit speciesInfoReady(results, timestamp);
}

/**
 * @brief Database::insertIndexInfo add the information about the new index
 * @param name to identify the index in question
 * @param description user can provide a description about the index
 * @param fasta directory to fasta file
 * @param index the directory to the index files
 */
void Database::insertIndexInfo(QString name, QString description, QString fasta, QString index, QString fastaChoice, QString fetchUrl) {
    QSqlDatabase db = QSqlDatabase::database("indexDatabase");

    QSqlQuery query;
    query = QSqlQuery(db);
    query.prepare("INSERT INTO index_information(name, description, fastaFile, indexDirectory, fastaChoice, fetchUrl) VALUES(:name, :description, :fastaFile, :indexDirectory, :fastaChoice, :fetchUrl)");
    query.bindValue(":name", name);
    query.bindValue(":description", description);
    query.bindValue(":fastaFile", fasta);
    query.bindValue(":indexDirectory", index);
    query.bindValue(":fastaChoice", fastaChoice);
    query.bindValue(":fetchUrl", fetchUrl);

    if(!query.exec()) {
        emit error("Error creating index:" + query.lastError().text());
    } else  {
        emit indexAdded();
    }
}

/**
 * @brief Database::insertPath adding path to executable kallisto and bustools
 * @param name provided by program, kallisto and bustools for easy lookup
 * @param path the path to the executable versions of kallisto and bustools
 */
void Database::insertPath(QString name, QString path) {
    QSqlDatabase db = QSqlDatabase::database("pathConnection");

    QSqlQuery query;
    query = QSqlQuery(db);
    query.prepare("INSERT OR IGNORE INTO paths(name, pathString) VALUES(:name,:path)");

    query.bindValue(":name", name);
    query.bindValue(":path", path);

    if(!query.exec()) {
        emit error("Error inserting values:" + query.lastError().text());
    } else  {
        emit pathAdded();
    }
}

/**
 * @brief Database::insertSpecies add species to database
 * @param info the information collected from ensembl about the species
 */
void Database::insertSpecies(QList< speciesInfo > info) {
    QSqlDatabase db = QSqlDatabase::database("speciesDatabase");
    QStringList releaseVersionList;
    QString timestamp;

    QSqlQuery query;
    query = QSqlQuery(db);

    query.prepare("INSERT INTO speciesInformation(name, display_name, assembly, timestamp) VALUES(:name, :displayName, :assembly, :timestamp)");
    QDate today = QDate::currentDate();

    foreach(speciesInfo val, info) {
        query.bindValue(":name", val.name);
        query.bindValue(":displayName", val.displayName);
        query.bindValue(":assembly", val.assembly);
        query.bindValue(":timestamp", today);

        if(!query.exec()) {
            emit error("Error inserting values:" + query.lastError().text());
        }

    }
}

/**
 * @brief Database::removeBreadcrumbs removes all the breadcrumbs when a new main window is created
 */
void Database::removeBreadcrumbs() {
    QSqlDatabase db = QSqlDatabase::database("breadcrumbsConnection");

    QSqlQuery query;
    query = QSqlQuery(db);
    query.prepare("DELETE FROM breadcrumbs");

    if(!query.exec()) {
        emit error("Error deleting breadcrumbs:" + query.lastError().text());
    }
}

/**
 * @brief Database::removePaths deletes the kallisto and bustools paths stored in the database
 */
void Database::removePaths() {
    QSqlDatabase db = QSqlDatabase::database("pathConnection");

    QSqlQuery query;
    query = QSqlQuery(db);
    query.prepare("DELETE FROM paths");

    if(!query.exec()) {
        emit error("Error deleting paths:" + query.lastError().text());
    }
}

/**
 * @brief Database::removeProjects deletes the project with the name in the parameter
 * @param names
 */
void Database::removeProjects(QStringList names) {
    QSqlDatabase db = QSqlDatabase::database("projectConnection");

    QSqlQuery query;
    query = QSqlQuery(db);
    query.prepare("DELETE FROM projects WHERE name = :name");
    bool worked = true;

    foreach(QString name, names) {
        query.bindValue(":name", name);
        if(!query.exec()) {
            worked = false;
        }
    }
    if(worked) {
        connect(this, SIGNAL(runHistoryDeleted()), SLOT(deletionCompleted()));
        this->deleteHistory(names);
    } else {
        emit deletionFailure();
    }

}

/**
 * @brief Database::updateOutputFolder update the output folder, option on the project window
 * @param name
 * @param resultDirectory
 */
void Database::updateOutputFolder(QString name, QString resultDirectory) {
    QSqlDatabase db = QSqlDatabase::database("projectConnection");

    QSqlQuery query;
    query = QSqlQuery(db);
    query.prepare("UPDATE projects SET resultDirectory = :resultDirectory WHERE name = :name");
    query.bindValue(":name", name);
    query.bindValue(":resultDirectory", resultDirectory);

    if(!query.exec()) {
        emit error("Error creating values:" + query.lastError().text());
    } else  {
        emit outputUpdateDone();
    }

}

/**
 * @brief Database::updateProject update the project information based on the information provided by the users
 * @param oldName the original name of the project
 * other parameters are the same as in create project
 */
void Database::updateProject(QString oldName, QString nameUpdate, QString indexDirectory, QString resultDirectory, QString technology, QString fastaDirectory) {
    QSqlDatabase db = QSqlDatabase::database("projectConnection");

    QSqlQuery query;
    query = QSqlQuery(db);
    query.prepare("UPDATE projects SET name = :nameUpdate, indexDirectory = :indexDirectory, resultDirectory = :resultDirectory, technology = :technology, fastaDirectory = :fastaDirectory WHERE name = :oldName");

    query.bindValue(":nameUpdate", nameUpdate);
    query.bindValue(":indexDirectory", indexDirectory);
    query.bindValue(":resultDirectory", resultDirectory);
    query.bindValue(":technology", technology);
    query.bindValue(":fastaDirectory", fastaDirectory);
    query.bindValue(":oldName", oldName);

    if(!query.exec()) {
        emit error("Error updating project:" + query.lastError().text());
    } else  {
        emit projectUpdated();
    }
}
