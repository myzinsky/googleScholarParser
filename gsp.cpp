#include "gsp.h"
#include "ui_gsp.h"
#include <QString>
#include <QDebug>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QEventLoop>
#include <QRegExp>
#include <QList>
#include <QTextBrowser>
#include <QStandardPaths>
#include <QObject>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QFile>
#include <QFileInfo>
#include <QProgressDialog>
#include <QProgressBar>

// qcZEIOkAAAAJ

gsp::gsp(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::gsp)
{
    ui->setupUi(this);
    ui->statusBar->showMessage("(c) 2017 Matthias Jung");

    QHeaderView *headerView = ui->tableWidget->horizontalHeader();
    headerView->setSectionResizeMode(0, QHeaderView::Stretch);

    // Estimate Config Path:
    configPath = QStandardPaths::standardLocations(
                QStandardPaths::GenericConfigLocation).at(0);

    qDebug() << configPath;

    QFileInfo check_file(configPath+"/gsp.sqlite");
    if(check_file.exists())
    {
        loadDatabase();
        displayPublications();
        displayStats();
    }
    else
    {
        createDatabase();
    }
}

gsp::~gsp()
{
    delete ui;
}

void gsp::on_pushButton_clicked()
{
    ui->tableWidget->clear();
    ui->tableWidget_2->clear();
    ui->tableWidget->clearContents();
    ui->tableWidget_2->clearContents();
    ui->tableWidget->setRowCount(0);
    ui->tableWidget_2->setRowCount(0);
    publications.clear();

    QProgressDialog progress("Query Google Scholar", "Abort", 0, 0);
    QProgressBar* bar=new QProgressBar;
    progress.setBar(bar);
    bar->setMinimum(0);
    bar->setMaximum(0);
    progress.show();

    unsigned int cstart = 0;

    QString html = getHTML(ui->lineEdit->text(), cstart);

    //TODO check if it worked out!

    unsigned int foundPublications = 0;
    do
    {
        foundPublications = getPublications(html);

        // If there are more than 100 publ.: we query google scholar again:
        if(foundPublications == 100)
        {
            cstart += 100;
            html = getHTML(ui->lineEdit->text(), cstart);
        }
    }
    while(foundPublications == 100);

    // Estimate the Name of the Author:
    getName(html);

    updateDatabase(ui->lineEdit->text());

    displayPublications();
    displayStats();

    progress.close();
}

QString gsp::getHTML(QString id, unsigned int cstart)
{
    QString url = "https://scholar.google.com/citations?user="
                + id + "&cstart="+QString::number(cstart)+"&pagesize=100";

    QNetworkAccessManager manager;
    QNetworkReply *response = manager.get(QNetworkRequest(QUrl(url)));
    QEventLoop event;
    connect(response,SIGNAL(finished()),&event,SLOT(quit()));
    event.exec();
    return response->readAll();
}

void gsp::getName(QString html)
{
    // Extract Name:
    // <div id="gsc_prf_in">.../div>

    QRegExp rx("<div id=\"gsc_prf_in\">([^<]+)</div>");
    rx.setMinimal(true);

    if (rx.indexIn(html) != -1)
    {
        ui->name->setText(rx.cap(1));
    }
}

/// Extracts all publications from an html string and returns the number of
/// found publications
unsigned int gsp::getPublications(QString html)
{
    // The publication list which is to find:
    // <tr class="gsc_a_tr"> ... </tr>

    QRegExp rx("<tr class=\"gsc_a_tr\">(.+)?</tr>");
    rx.setMinimal(true);

    int pos = 0;    // where we are in the string
    unsigned int counter = 0;    // where we are in the string

    // Get all publications:
    while (pos >= 0)
    {
        pos = rx.indexIn(html, pos);
        if (pos >= 0)
        {
            pos++; // move along in html
            counter++; // move along in html
            parsePublication(rx.cap(1));
        }
    }

    return counter;
}

void gsp::parsePublication(QString html)
{
    publication p;

    // Extract Title:
    // <a href="..." class="gsc_a_at">...</a>

    QRegExp rxTitle("<a href=\"(.+)\"\\s*class=\"gsc_a_at\">([^<]+)</a>");
    rxTitle.setMinimal(true);

    if (rxTitle.indexIn(html) != -1)
    {
        p.title = rxTitle.cap(2);
        p.url = rxTitle.cap(1);
    }

    // Extract Authors:
    // <div class="gs_gray">...</div>

    QRegExp rxAuthors("<div class=\"gs_gray\">(.+)</div>");
    rxAuthors.setMinimal(true);

    if (rxAuthors.indexIn(html) != -1)
    {
        p.authors = rxAuthors.cap(1);
    }

    // Extract Citation Count:
    // <a href="..." class="gsc_a_ac">...</a>

    QRegExp rxCitations("<a href=\"(.+)\" class=\"gsc_a_ac\">(.+)</a>");
    rxCitations.setMinimal(true);

    if (rxCitations.indexIn(html) != -1)
    {
        p.citations = rxCitations.cap(2).toInt();
    }

    // Extract Year:
    // <span class="gsc_a_h">2014</span>

    QRegExp rxYear("<span class=\"gsc_a_h\">(.+)</span>");
    rxYear.setMinimal(true);

    if (rxYear.indexIn(html) != -1)
    {
        p.year = rxYear.cap(1).toInt();
    }

    publications.append(p);
}

void gsp::displayPublications()
{
    for(int i=0; i < publications.count(); i++)
    {
        ui->tableWidget->insertRow(ui->tableWidget->rowCount());

        QTextBrowser * te = new QTextBrowser();
        te->setHtml(
           "<b><a href=\"https://scholar.google.com"
           + publications.at(i).url
           + "\" target=\"_blank\">"
           + publications.at(i).title + "</a></b><br><i>"
           + publications.at(i).authors + "</i>"
        );
        te->setOpenExternalLinks(true);
        te->setFrameStyle(QFrame::NoFrame);
        te->setFocusPolicy(Qt::NoFocus);
        //te->setTextInteractionFlags(Qt::LinksAccessibleByMouse);
        te->setStyleSheet("background-color: rgba(255, 255, 255, 0);");
        te->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

        ui->tableWidget->setRowHeight(ui->tableWidget->rowCount()-1, 40);
        ui->tableWidget->setCellWidget(ui->tableWidget->rowCount()-1, 0, te);

        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,
            1,
            new QTableWidgetItem(QString::number(publications.at(i).citations)));

        ui->tableWidget->setItem(ui->tableWidget->rowCount()-1,
            2,
            new QTableWidgetItem(QString::number(publications.at(i).year)));

        //ui->tableWidget->resizeColumnsToContents();
    }
}

void gsp::displayStats()
{
    ui->tableWidget_2->insertRow(ui->tableWidget_2->rowCount());
    ui->tableWidget_2->setItem(ui->tableWidget_2->rowCount()-1,
        0,
        new QTableWidgetItem("Citations:"));
    ui->tableWidget_2->setItem(ui->tableWidget_2->rowCount()-1,
        1,
        new QTableWidgetItem(QString::number(citations())));

    ui->tableWidget_2->insertRow(ui->tableWidget_2->rowCount());
    ui->tableWidget_2->setItem(ui->tableWidget_2->rowCount()-1,
        0,
        new QTableWidgetItem("h-index:"));
    ui->tableWidget_2->setItem(ui->tableWidget_2->rowCount()-1,
        1,
        new QTableWidgetItem(QString::number(hindex())));

    ui->tableWidget_2->insertRow(ui->tableWidget_2->rowCount());
    ui->tableWidget_2->setItem(ui->tableWidget_2->rowCount()-1,
        0,
        new QTableWidgetItem("i10-index:"));
    ui->tableWidget_2->setItem(ui->tableWidget_2->rowCount()-1,
        1,
        new QTableWidgetItem(QString::number(i10index())));
}

unsigned int gsp::citations()
{
    unsigned int sum = 0;
    for(int i=0; i < publications.count(); i++)
    {
        sum += publications.at(i).citations;
    }
    return sum;
}

unsigned int gsp::hindex()
{
    unsigned int h = 0;

    for(int i=0; i < publications.count(); i++)
    {
        if(publications.at(i).citations > (unsigned int)i)
        {
            h++;
        }
    }
    return h;
}

unsigned int gsp::i10index()
{
    unsigned int h = 0;

    for(int i=0; i < publications.count(); i++)
    {
        if(publications.at(i).citations >= 10)
        {
            h++;
        }
    }
    return h;
}

void gsp::createDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(configPath+"/gsp.sqlite");
    db.open();
    QSqlQuery query;
    if(!query.exec("create table publications "
              "(id integer primary key, "
              "title varchar(500), "
              "url varchar(500), "
              "authors varchar(500), "
              "citations integer, "
              "year integer);"))
    {
            qDebug() << "Cannot create publications table";
    }

    if(!query.exec("create table settings "
                   "(scholar varchar(100),"
                   "name varchar(100));"))
    {
            qDebug() << "Cannot create settings table";
    }
}

void gsp::updateDatabase(QString scholar)
{
    // TODO update mechanism!
    // TODO Diff Mechanism!

    for(int i=0; i < publications.count(); i++)
    {
        QSqlQuery query;
        query.prepare("INSERT INTO publications"
                      "(title,url,authors,citations,year)"
                      " VALUES (:title,:url,:authors,:citations,:year)");
        query.bindValue(":title", publications.at(i).title);
        query.bindValue(":url", publications.at(i).url);
        query.bindValue(":authors", publications.at(i).authors);
        query.bindValue(":citations", publications.at(i).citations);
        query.bindValue(":year", publications.at(i).year);

        if(!query.exec())
            qDebug() << "Cannot store publication";

    }

    QSqlQuery query;
    query.prepare("INSERT INTO settings"
                  "(scholar,name)"
                  " VALUES (:scholar,:name)");
    query.bindValue(":scholar", scholar);
    query.bindValue(":name", ui->name->text());

    if(!query.exec())
        qDebug() << "Cannot store config";
}

void gsp::loadDatabase()
{
    db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName(configPath+"/gsp.sqlite");
    db.open();

    QSqlQuery query("SELECT * FROM publications");
    int title = query.record().indexOf("title");
    int url = query.record().indexOf("url");
    int authors = query.record().indexOf("authors");
    int citations = query.record().indexOf("citations");
    int year = query.record().indexOf("year");

    while (query.next())
    {
        publication p;
        p.title = query.value(title).toString();
        p.url = query.value(url).toString();
        p.authors = query.value(authors).toString();
        p.citations = query.value(citations).toUInt();
        p.year = query.value(year).toUInt();

        publications.append(p);
    }

    query.prepare("SELECT * FROM settings");
    query.exec();
    int scholar = query.record().indexOf("scholar");
    int name = query.record().indexOf("name");

    while (query.next())
    {
        ui->lineEdit->setText(query.value(scholar).toString());
        ui->name->setText(query.value(name).toString());
    }
}
