#include "mapDownloader.h"

#include <QApplication>

#ifdef Q_OS_WIN
 #include "updater.h"
#endif

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MapDownloader w;
    w.show();
#ifdef Q_OS_WIN
    Updater::checkForUpdate(true);
#endif
    return a.exec();
}
