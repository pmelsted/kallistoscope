#ifndef REMOVEPROJECTS_H
#define REMOVEPROJECTS_H

#include <QDialog>
#include <QButtonGroup>
#include "../logic/database.h"

namespace Ui {
class removeProjects;
}

class removeProjects : public QDialog
{
    Q_OBJECT
    QButtonGroup* checkBoxGroup;
    Database* db;
    QStringList deleteList;
public:
    explicit removeProjects(QWidget *parent = nullptr, Database* dbInstance = nullptr);
    ~removeProjects();
signals:
    void deleteComplete(QStringList);
private slots:
    void deleteDone();
    void displayFailure();
    void on_delete_projects_clicked();
    void on_cancel_delete_clicked();
    void populateCheckBox(QStringList);
private:
    Ui::removeProjects *ui;
};

#endif // REMOVEPROJECTS_H
