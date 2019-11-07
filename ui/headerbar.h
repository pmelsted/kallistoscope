#ifndef HEADERBAR_H
#define HEADERBAR_H

#include <QWidget>
#include "../logic/database.h"
#include <QHeaderView>
#include <QStandardItemModel>

namespace Ui {
class headerBar;
}

class headerBar : public QWidget
{
    Q_OBJECT
    QString classCalling;
    Database* db;
    QHeaderView* header = new QHeaderView(Qt::Orientation::Horizontal, this);
    QStandardItemModel* model = new QStandardItemModel(this);
signals:
    void indexfileClicked();
    void indexcalcClicked();
    void performClicked();
    void landscapeChanged();
    void newProjectClicked();
private slots:
    void populateHeader(QStringList);
    void react(int);
public:
    explicit headerBar(QWidget *parent = nullptr, Database* db_instance = nullptr);
    ~headerBar();
    void collectHeaderInfo(QString);
    void empty();
    void populateHeaderBar(QStringList);
    QStringList visited;
private:
    Ui::headerBar *ui;
};

#endif // HEADERBAR_H
