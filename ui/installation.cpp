#include "installation.h"
#include "ui_installation.h"

#include <QRadioButton>
#include <QButtonGroup>
#include <QAbstractButton>
#include <QDebug>

/**
 * @brief installation::installation new window to find out which operating system the users machine runs on
 * @param parent
 */
installation::installation(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::installation)
{
    ui->setupUi(this);
    this->setWindowTitle("Installation");

    this->setFixedSize(this->width(), this->height());
}

installation::~installation()
{
    delete ui;
}

/**
 * @brief installation::on_cancel_clicked the user doesn't want the application to access kallisto and bustools
 */
void installation::on_cancel_clicked()
{
    emit cancelled();
    this->deleteLater();
}

/**
 * @brief installation::on_install_clicked get which button is checked and show errors if any
 */
void installation::on_install_clicked()
{
    QAbstractButton* checked = ui->operatingSystems->checkedButton();
    QString error;
    if(checked == 0) {
        error = "Please select an operating system";
    }
    if(error.isEmpty()) {
        QString value;
        if(checked->objectName() == "inux") {
            value = "linux";
            emit installationSignal(value);
        } else {
            emit installationSignal(checked->objectName());
        }
        this->deleteLater();
    } else {
        ui->error_field->setStyleSheet("color:red");
        ui->error_field->setText(error);
    }
}

