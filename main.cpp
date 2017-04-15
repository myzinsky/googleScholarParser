#include "gsp.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    gsp w;
    w.showMaximized();

    return a.exec();
}
