#ifndef MAP_DOWNLOADER_H
#define MAP_DOWNLOADER_H

#include <QMainWindow>
#include "qhttpserverfwd.h"

#include <QObject>
#include <QScopedPointer>
#include <QFileDialog>
#include <QMainWindow>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QApplication>
#include <QFile>
#include <QDir>
#include <math.h>
#include <iostream>

QT_BEGIN_NAMESPACE
namespace Ui { class MapDownloader; }
QT_END_NAMESPACE

struct Tile{
    int x;
    int y;
    int zoom;
    Tile(int a, int b, int z):x(a),y(b),zoom(z){};
    Tile(const Tile& Other):x(Other.x), y(Other.y), zoom(Other.zoom){};
    ~Tile(){};
};

class MapDownloader : public QMainWindow
{
    Q_OBJECT

public:
    MapDownloader(QWidget *parent = nullptr);
    /* Sets up PushButtons and Widgets. */
    ~MapDownloader();
    /* Destructor */
    bool CheckInput(std::wstring Coordinate);
    /* Checks if input of each coordinate is correct.
       79°58′56″W, N49.2065383N, E16.6030161E, N39.48333571, 39.48333571, 49.2065383N, 41 24.2028
       41°24.2028'N, 40 26 46 N, N40°26′46″, -79°58′56″, 51 -> These are possible types of input format.
       Any other format will most likely not work. If the format is wrong program doesn't continue. Space
       at the end of the input also means wrong input. */

    void ToDecimal(double *Coordinate, std::wstring CoordinatesStr);
    /* Creates a decimal number from input format. 79°58′56″W -> -79.9822. */

    void MapValues(double CoordinateX, double CoordinateY, Tile *TL, int Zoom);
    /* Calculates the values of tiles from Coordinates.*/

    void CreateFigure();
    /* Firstly, two missing corners of rectangle are created and then all the tiles that lies witihin this
       area are created as well. */

    void CreateDir();
    /* Creates a dir called TileMap */

    void Download();
    /* Takes the string of the URL in QLineEdit and for each tile creates a request.
       Each request is sent to network manager until all tiles are downloaded. */

private slots:
    bool DownloadEvent();
    /* Pushbutton for downloading a tile map */

    void ResetEvent();
    /* Sets values to 0 */

    void ChooseDirEvent();
    /* Chooses a dir for downloading a tile map */

    void replyFinished(QNetworkReply* reply);
    /* Sneds signal if one tile was downloaded */

    void handleRequest(QHttpRequest *req, QHttpResponse *resp);
    /* Creates an object of type Responder.*/

    void CreateLocalHost();
    /* Creates an object of type QHttpServer which allows us to connect Request and Respond server. */

    void SelectFolder();
    /* Directs the path to the folder with TileMap. */

private:
    Ui::MapDownloader *ui;
    QImage* img;
    QString DirectoryName;
    QNetworkAccessManager *manager;
    QNetworkReply* reply;
    std::vector<Tile> Figure;
    /* This vector contains two corners of rectangle and then is filled with all the others within the area
       of rectangle */
    QString FolderPath;
};




/// Responder

class Responder : public QObject
{
    Q_OBJECT

public:
    Responder(QHttpRequest *req, QHttpResponse *resp, QString FolderPath);
    /* Creates a LocalHost. Argument QHttpRequest *req is
       evaluated as a regular expression. If the request is correct
       and the file exists LocalHost displays it. Otherwise, it causes error.*/
    ~Responder();

signals:
    void done();

private slots:
    void accumulate(const QByteArray &data);
    void reply();

private:
    QScopedPointer<QHttpRequest> m_req;
    QHttpResponse *m_resp;
};
#endif // MAP_DOWNLOADER_H
