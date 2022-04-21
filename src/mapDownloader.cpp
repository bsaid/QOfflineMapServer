#include "mapDownloader.h"
#include "ui_mapDownloader.h"
#include <QCoreApplication>
#include <QRegExp>
#include <QStringList>
#include <QDebug>
#include <QImage>
#include <QBuffer>
#include <QFile>
#include <iostream>
#include <qhttpserver.h>
#include <qhttprequest.h>
#include <qhttpresponse.h>


MapDownloader::MapDownloader(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MapDownloader)
{
    ui->setupUi(this);
    DirectoryName = "C:\\TileMap";
    connect(ui->Download, SIGNAL(released()), this, SLOT(DownloadEvent()));
    connect(ui->ResetPoints, SIGNAL(released()), this, SLOT(ResetEvent()));
    connect(ui->ChangeDirectory, SIGNAL(released()), this, SLOT(ChooseDirEvent()));
    connect(ui->LocalHost, SIGNAL(released()), this, SLOT(CreateLocalHost()));
    connect(ui->SelectFolder, SIGNAL(released()), this, SLOT(SelectFolder()));
}

void MapDownloader::Download()
{
    QStringList lines;
    ui->textBrowser->append("Total tiles " + QString::number(Figure.size()) + " - zoom " + QString::number(Figure[0].zoom));
    ui->textBrowser->append("------------------------------------");
    for (int i = 0; (size_t)i < Figure.size(); ++i) {
        std::string mapStr = ui->MapURL->text().toStdString();
        size_t index = 0;
        index = mapStr.find("${z}", 0);
        if(index == std::string::npos) {
            index = mapStr.find("{z}", 0);
            mapStr.replace(index, 3, std::to_string(Figure[i].zoom));
        } else {
            mapStr.replace(index, 4, std::to_string(Figure[i].zoom));
        }
        index = mapStr.find("${y}", 0);
        if(index == std::string::npos) {
            index = mapStr.find("{y}", 0);
            mapStr.replace(index, 3, std::to_string(Figure[i].y));
        } else {
            mapStr.replace(index, 4, std::to_string(Figure[i].y));
        }
        index = mapStr.find("${x}", 0);
        if(index == std::string::npos) {
            index = mapStr.find("{x}", 0);
            mapStr.replace(index, 3, std::to_string(Figure[i].x));
        } else {
            mapStr.replace(index, 4, std::to_string(Figure[i].x));
        }
        mapStr.append(" " + std::to_string(Figure[i].zoom) + "_" + std::to_string(Figure[i].x) + "_" + std::to_string(Figure[i].y));
        lines.push_back(QString::fromUtf8(mapStr.c_str()));
    }
    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(replyFinished(QNetworkReply*)));
    ui->progressBar->setMaximum(ui->progressBar->maximum()+lines.size());
    for(QString line : lines)
    {
        QString address = line.split(" ")[0];
        QString identifier = line.split(" ")[1];
        qDebug() << address << " : " << identifier;
        QNetworkReply* reply = manager->get(QNetworkRequest(QUrl(address)));
        reply->setProperty("id", identifier);
    }
}

void MapDownloader::CreateDir()
{
    QDir dir;
    dir.mkdir(DirectoryName);
}

void MapDownloader::replyFinished(QNetworkReply* reply)
{
    qDebug() << ui->progressBar->value() << " ";
    if(reply->error())
    {
        qDebug() << "ERROR!";
        qDebug() << reply->errorString();
    }
    else
    {

        qDebug() << reply->header(QNetworkRequest::ContentTypeHeader).toString();
        qDebug() << reply->header(QNetworkRequest::LastModifiedHeader).toDateTime().toString();;
        qDebug() << reply->header(QNetworkRequest::ContentLengthHeader).toULongLong();
        qDebug() << reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
        qDebug() << reply->attribute(QNetworkRequest::HttpReasonPhraseAttribute).toString();
        QFile *file = new QFile(DirectoryName + "\\" + reply->property("id").toString() + ".png");
        if(file->open(QFile::Append))
        {
            file->write(reply->readAll());
            file->flush();
            file->close();
        }
        delete file;
    }
    ui->progressBar->setValue(ui->progressBar->value()+1);
    reply->deleteLater();
}

bool MapDownloader::CheckInput(std::wstring Coordinate)
{
    std::vector<bool> CorrectInput;
    if(Coordinate.size() == 0) return false;
    if(Coordinate.size() == 1 && (Coordinate.front() >= '0' && Coordinate.front() <= '9')) return true;
    CorrectInput.push_back((Coordinate.front() == 'S' || Coordinate.front() == 'W' ||
            Coordinate.front() == 'E' || Coordinate.front() == 'N' || Coordinate.front() == '-'
            || (Coordinate.front() >= '0' && Coordinate.front() <= '9')) ? true : false);

    CorrectInput.push_back((Coordinate.back() == 'S' || Coordinate.back() == 'W' ||
            Coordinate.back() == 'E' || Coordinate.back() == 'N' || Coordinate.back() == 8242 || Coordinate.back() == 8243
            || (Coordinate.back() >= '0' && Coordinate.back() <= '9')) ? true : false);
    Coordinate.pop_back();
    Coordinate.erase(Coordinate.begin());
    for(wchar_t &c : Coordinate)
    {
        CorrectInput.push_back(((c >= '0' && c <= '9') || c == '.' || c == 8242 || c == 8243 || c == L'°' || c == ' ') ? true : false);
    }
    for(bool Bool : CorrectInput)
    {
        if(Bool == false) return false;
    }
    return true;
}

void MapDownloader::ToDecimal(double *Coordinate, std::wstring CoordinatesStr)
{
    double Orientation = 1.0;
    std::wstring Str;
    std::vector<std::wstring> Numbers;

    (CoordinatesStr.back() == 'S' || CoordinatesStr.back() == 'W' || CoordinatesStr.back() == 'E' || CoordinatesStr.back() == 'N')
        ? (CoordinatesStr.insert(0,&CoordinatesStr.back()), CoordinatesStr.pop_back()) : void();
    (CoordinatesStr.back() >= '0' && CoordinatesStr.back() <= '9') ? CoordinatesStr.push_back(' ') : void();
    for(wchar_t& c : CoordinatesStr) {

        if(c == 'S' || c == 'W' || c == 'E' || c == 'N' || c == '-'){

            Orientation = (c == 'S' || c == 'W' || c == '-') ? -1.0 : 1.0;
            continue;
        }
        ((c >= '0' && c <= '9') || c == '.') ? Str.push_back(c) : (Numbers.push_back(Str),  Str.clear());
    }

    for(int i = 0; (size_t)i < Numbers.size(); ++i)
    {
        *Coordinate += std::stod(Numbers[i])/pow(60,i);
    }
    *Coordinate = *Coordinate * Orientation;
}

void MapDownloader::MapValues(double LatitudeNum, double LongitudeNum, Tile* TL, int Zoom)
{
    TL->zoom = Zoom;
    TL->x = (int)(floor((LongitudeNum + 180.0) / 360.0 * (1 << TL->zoom)));
    TL->y = (int)(floor((1.0 - asinh(tan(LatitudeNum * M_PI/180.0)) / M_PI) / 2.0 * (1 << TL->zoom)));
    Figure.push_back(*TL);
    ui->textBrowser->append("Lat,Lon: " + QString::number(LatitudeNum) + ","
         + QString::number(LongitudeNum) + " -> Tile x,y: " + QString::number(TL->x) + "," + QString::number(TL->y));
}

void MapDownloader::CreateFigure()
{
    int LeftX = 0, RightX = 0, LowY = 0, HighY = 0, Zoom = 0;
    (Figure[0].x < Figure[1].x) ? (LeftX = Figure[0].x, RightX = Figure[1].x) : (LeftX = Figure[1].x, RightX = Figure[0].x);
    (Figure[0].y < Figure[1].y) ? (LowY = Figure[0].y, HighY = Figure[1].y) : (LowY = Figure[1].y, HighY = Figure[0].y);
    Zoom = Figure[0].zoom;
    Figure.clear();
    for(int i = LowY; i <= HighY; ++i)
    {
        for(int j = LeftX; j <= RightX; ++j)
        {
            Figure.push_back(Tile(j,i,Zoom));
        }
    }
}

void MapDownloader::ResetEvent()
{
    ui->Zoom->setText("");
    ui->Latitude1->setText("");
    ui->Longitude1->setText("");
    ui->textBrowser->clear();
    ui->Latitude2->setText("");
    ui->Longitude2->setText("");
    Figure.clear();
}

bool MapDownloader::DownloadEvent()
{
    double LatitudeNum1 = 0.0;
    double LongitudeNum1 = 0.0;
    double LatitudeNum2 = 0.0;
    double LongitudeNum2 = 0.0;
    int Zoom1 = 0;
    int Zoom2 = 0;
    Figure.clear();
    Tile TL(0,0,0);
    std::wstring LatitudeStr1 = ui->Latitude1->text().toStdWString();
    std::wstring LongitudeStr1 = ui->Longitude1->text().toStdWString();
    std::wstring LatitudeStr2 = ui->Latitude2->text().toStdWString();
    std::wstring LongitudeStr2 = ui->Longitude2->text().toStdWString();
    QString Zoom = ui->Zoom->text();
    ui->progressBar->setMinimum(0);
    ui->progressBar->setMaximum(0);
    ui->progressBar->setValue(0);
    ui->textBrowser->append("First and last point in rectangle");
    ui->textBrowser->append("-----------------------------------");
    if(CheckInput(LatitudeStr1) && CheckInput(LongitudeStr1) && CheckInput(LatitudeStr2) && CheckInput(LongitudeStr2))
    {
        ToDecimal(&LatitudeNum1, LatitudeStr1);
        ToDecimal(&LongitudeNum1, LongitudeStr1);
        ToDecimal(&LatitudeNum2, LatitudeStr2);
        ToDecimal(&LongitudeNum2, LongitudeStr2);
    }
    else
    {
        ui->textBrowser->setText("Wrong input! Reset your points please.");
        return false;
    }

    if(Zoom.contains("-"))
    {
        Zoom1 = Zoom.split("-")[0].toInt();
        Zoom2 = Zoom.split("-")[1].toInt();
    }
    else
    {
        Zoom1 = Zoom.toInt();
        Zoom2 = Zoom.toInt();
    }

    CreateDir();
    for(int i = Zoom1; i <= Zoom2; ++i)
    {
        MapValues(LatitudeNum1, LongitudeNum1, &TL, i);
        MapValues(LatitudeNum2, LongitudeNum2, &TL, i);
        CreateFigure();
        Download();
        Figure.clear();
    }
    return true;
}

void MapDownloader::ChooseDirEvent()
{
    DirectoryName = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                      "/home",
                                                      QFileDialog::ShowDirsOnly
                                                      | QFileDialog::DontResolveSymlinks);
    //DirectoryName += "\\TileMap";
    ui->lineEditOutDir->setText(DirectoryName);
}

/* LOCALHOST */

void MapDownloader::SelectFolder()
{
    FolderPath = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                           "/home",
                                                           QFileDialog::ShowDirsOnly
                                                           | QFileDialog::DontResolveSymlinks);
    ui->lineEditInDir->setText(FolderPath);
}

void MapDownloader::CreateLocalHost()
{
    QHttpServer *server = new QHttpServer(this);
    connect(server, SIGNAL(newRequest(QHttpRequest*, QHttpResponse*)),
        this, SLOT(handleRequest(QHttpRequest*, QHttpResponse*)));
    server->listen(QHostAddress::Any, ui->serverPortSpinBox->value());
    ui->lineEditStatus->setText("http://127.0.0.1:" + QString::number(ui->serverPortSpinBox->value()) + "/{z}/{x}/{y}.png");
}

void MapDownloader::handleRequest(QHttpRequest *req, QHttpResponse *resp)
{
    new Responder(req, resp, FolderPath);
}

MapDownloader::~MapDownloader()
{
    delete ui;
}

/// Responder

Responder::Responder(QHttpRequest *req, QHttpResponse *resp, QString FolderPath)
    : m_req(req)
    , m_resp(resp)
{

    QRegExp exp("^/[0-9]+/[0-9]+/[0-9]+.png");
    if (exp.indexIn(req->path()) == -1)
    {
        resp->writeHead(403);
        resp->end(QByteArray("You aren't allowed here!"));
        /// @todo There should be a way to tell request to stop streaming data
        return;
    }

    resp->setHeader("Content-Type", "image/png");
    resp->writeHead(200);
    QString FileName;
    for(auto c : exp.capturedTexts()[0])
    {
        (c == '/') ? FileName.push_back('_') : FileName.push_back(c);
    }
    FileName.remove(0,1);
    QFile file(FolderPath + "\\" + FileName);
    file.open(QFile::ReadOnly);
    resp->write(file.readAll());

    connect(req, SIGNAL(data(const QByteArray&)), this, SLOT(accumulate(const QByteArray&)));
    connect(req, SIGNAL(end()), this, SLOT(reply()));
    connect(m_resp, SIGNAL(done()), this, SLOT(deleteLater()));
}

Responder::~Responder(){}

void Responder::accumulate(const QByteArray &data)
{
    m_resp->write(data);
}

void Responder::reply()
{
    m_resp->end(QByteArray("</body></html>"));
}
