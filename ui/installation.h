#ifndef INSTALLATION_H
#define INSTALLATION_H

#include <QMainWindow>

namespace Ui {
class installation;
}

class installation : public QMainWindow
{
    Q_OBJECT

public:
    explicit installation(QWidget *parent = nullptr);
    ~installation();
signals:
    void cancelled();
    void installationSignal(QString);
private slots:
    void on_install_clicked();
    void on_cancel_clicked();
private:
    Ui::installation *ui;
};

#endif // INSTALLATION_H
