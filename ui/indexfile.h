#ifndef INDEXFILE_H
#define INDEXFILE_H

#include <QMainWindow>
#include "../logic/database.h"
#include "headerbar.h"

namespace Ui {
class indexFile;
}

class indexFile : public QMainWindow
{
    Q_OBJECT
    QWidget* ancestor;
    Database* db;
    QStringList errorList;
    headerBar* header;
    QString indexDestination;
public:
    explicit indexFile(QWidget *parent = nullptr, Database* databaseInstance = nullptr);
    ~indexFile();
signals:
    void cancelled();
    void index_created(QString);
private slots:
    void checkDirectoryAvailability(QStringList);
    void checkNameAvailability(QStringList);
    void index_redirect();
    void on_go_back_clicked();
    void on_saveIndex_clicked();
    void on_select_index_file_clicked();
    void removeReference();
private:
    Ui::indexFile *ui;
    void errorCheck();
};

#endif // INDEXFILE_H
