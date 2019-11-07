#include "project.h"

/**
 * @brief project::project generate new project struct containing all the project information
 * @param name
 * @param db
 */
project::project(QString name, Database* db)
{
    if(db == nullptr) {
        db = new Database();
    }
    connect(db, SIGNAL(projectInfoCollected(QStringList)), this, SLOT(generateProjectDetails(QStringList)));
    db->getProjectInformation(name);
}

/**
 * @brief project::generateProjectDetails fill in a project object and return it
 * @param projectResults the result from the database function call
 */
void project::generateProjectDetails(QStringList projectResults) {
    struct projectInfo project;
    project.name = projectResults[0];
    project.indexDirectory = projectResults[1];
    project.resultDirectory = projectResults[2];
    project.technology = projectResults[3];
    project.fastaDirectory = projectResults[4];
    currProject = project;

}

/**
 * @brief project::provideProjectInfo return the current project
 * @return
 */
projectInfo project::provideProjectInfo() {
    return currProject;
}
