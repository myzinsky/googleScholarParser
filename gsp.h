#ifndef GSP_H
#define GSP_H

#include <QMainWindow>
#include <QSqlDatabase>
#include "publication.h"

namespace Ui
{
    class gsp;
}

class gsp : public QMainWindow
{
    Q_OBJECT

public:
    explicit gsp(QWidget *parent = 0);
    ~gsp();

private slots:
    void on_pushButton_clicked();

private:
    Ui::gsp *ui;

    QList<publication> publications;
    QString configPath;

    QString getHTML(QString id, unsigned int cstart);
    unsigned int getPublications(QString html);
    void getName(QString html);
    void parsePublication(QString html);
    void displayPublications();
    void displayStats();
    unsigned int citations();
    unsigned int hindex();
    unsigned int i10index();
    void createDatabase();
    void updateDatabase(QString scholar);
    void loadDatabase();

    QSqlDatabase db;
};

#endif // GSP_H
