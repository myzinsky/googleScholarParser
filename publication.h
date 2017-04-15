#include <QString>
#include <QUrl>

#ifndef PUBLICATION_H
#define PUBLICATION_H

class publication
{
public:
    publication();
    QString title;
    QString authors;
    QString url;
    unsigned int citations;
    unsigned int year;
    QUrl citationUrl;
};

#endif // PUBLICATION_H
