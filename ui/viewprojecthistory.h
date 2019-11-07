#ifndef VIEWPROJECTHISTORY_H
#define VIEWPROJECTHISTORY_H

#include <QMainWindow>
#include "../logic/database.h"

namespace Ui {
class ViewProjectHistory;
}

class ViewProjectHistory : public QMainWindow
{
    Q_OBJECT
    Database* db;
    QString message;

public:
    explicit ViewProjectHistory(QWidget *parent = nullptr, QString searchString="", Database* db_instance=nullptr, bool runHistory = true);
    ~ViewProjectHistory();
private slots:
    void on_go_back_clicked();
    void populateInfo(QStringList);
    void populateRunHistory(QList<runInfo>);
private:
    Ui::ViewProjectHistory *ui;
};

#endif // VIEWPROJECTHISTORY_H
