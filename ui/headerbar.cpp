#include "headerbar.h"
#include "ui_headerbar.h"
#include "mainwindow.h"
#include "newproject.h"
#include "indexfile.h"
#include "indexcreation.h"
#include "openproject.h"

#include <QVariant>
#include <QDebug>
#include <QStandardItem>
#include <QSizePolicy>

/**
 * @brief headerBar::headerBar a widget for the breadcrumbs on the main pages of the application
 * @param parent
 * @param db_instance
 */
headerBar::headerBar(QWidget *parent, Database* db_instance) :
    QWidget(parent),
    ui(new Ui::headerBar)
{
    if(db_instance == nullptr) {
        db = new Database();
    } else {
        db = db_instance;
    }

    ui->setupUi(this);
    header->setSectionsClickable(true);
    header->setMinimumWidth(parent->width());
    this->setMinimumWidth(parent->width());
    header->setSectionResizeMode(QHeaderView::ResizeToContents);


    QList<QStandardItem* > items;
    QStandardItem* item1 = new QStandardItem();
    item1->setData(QVariant(QString("Welcome")), Qt::DisplayRole);
    items.append(item1);
    model->appendColumn(items);
    model->setHeaderData(0, Qt::Horizontal, QVariant(QString("Welcome")));

    header->setModel(model);
    connect(header, SIGNAL(sectionClicked(int)), this, SLOT(react(int)));
    visited.append("Welcome");

}

headerBar::~headerBar()
{
    delete ui;
}

/**
 * @brief headerBar::collectHeaderInfo calls database to get the current breadcrumb path
 * @param currenctClass
 */
void headerBar::collectHeaderInfo(QString currenctClass) {
    classCalling = currenctClass;
    connect(db, SIGNAL(breadcrumbs(QStringList)), this, SLOT(populateHeader(QStringList)));
    db->getBreadcrumbs();
}

/**
 * @brief headerBar::empty clear out the header for new information
 */
void headerBar::empty() {
    model->clear();

    QList<QStandardItem* > items;
    QStandardItem* item1 = new QStandardItem();
    item1->setData(QVariant(QString("Welcome")), Qt::DisplayRole);
    items.append(item1);
    model->appendColumn(items);
    model->setHeaderData(0, Qt::Horizontal, QVariant(QString("Welcome")));
}

/**
 * @brief headerBar::populateHeader adding the current page to the breadcrumb list
 * @param crumbList
 */
void headerBar::populateHeader(QStringList crumbList) {
    this->empty();
    disconnect(db, SIGNAL(breadcrumbs(QStringList)), this, SLOT(populateHeader(QStringList)));
    crumbList.append(classCalling);
    visited.append(classCalling);
    db->addBreadcrumb(classCalling);

    this->populateHeaderBar(crumbList);

}

/**
 * @brief headerBar::populateHeaderBar creating header in the layout
 * @param crumbs
 */
void headerBar::populateHeaderBar(QStringList crumbs) {
    int temp = 1;

    foreach(QString crumb, crumbs) {
        QList<QStandardItem* > items;
        QStandardItem* item1 = new QStandardItem();
        item1->setData(QVariant(QString(crumb)), Qt::DisplayRole);
        items.append(item1);
        model->appendColumn(items);
        model->setHeaderData(temp, Qt::Horizontal, QVariant(QString(crumb)));
        temp++;
    }

    if(header->length() > header->width()) {
        header->resize(header->length() + 300, header->height());
        this->setFixedWidth(header->length() + 300);
    }
}

/**
 * @brief headerBar::react action to be perform when a section in the header is clicked
 * @param logicalIndex
 */
void headerBar::react(int logicalIndex) {
    QString page = model->headerData(logicalIndex, Qt::Horizontal).toString();
    if(page != visited[visited.length()-1]) {
        if(page.contains("Welcome")) {
            MainWindow* main = new MainWindow();
            main->show();
            visited.clear();
            db->removeBreadcrumbs();
            emit landscapeChanged();
        }
        if(page.contains("Create")) {
            newProject* project = new newProject(nullptr, nullptr, "");
            project->show();
            emit newProjectClicked();
        }
        if(page.contains("Provide")) {
            indexFile* indexfile = new indexFile(nullptr, nullptr);
            indexfile->show();
            emit indexfileClicked();
        }
        if(page.contains("Calculate")) {
            indexcreation* calc = new indexcreation(nullptr, nullptr);
            calc->show();
            emit indexcalcClicked();
        }
        if(page.contains("Perform")) {
            openProject* open = new openProject(nullptr, "", nullptr);
            open->show();
            emit performClicked();
        }
    }
}
