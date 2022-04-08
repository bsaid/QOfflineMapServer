#include "mapDownloader.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MapDownloader w;
    w.show();
    return a.exec();
}
