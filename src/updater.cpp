/**********************************************
**    This file is inspired by Lorris
**    http://tasssadar.github.com/Lorris/
***********************************************/

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QMessageBox>
#include <QProcess>
#include <QApplication>
#include <QDateTime>
#include <QFile>
#include <QPushButton>
#include <QDesktopWidget>
#include <QFuture>
#include <QFutureWatcher>
#include <QtConcurrentRun>

#include "updater.h"
#include "revision.h"
#include "utils.h"
#include "tooltipwarn.h"

#if defined(__x86_64__) || defined(_WIN64) || defined(Q_PROCESSOR_X86_64)
#define MANIFEST_URL "http://github.com/bsaid/QOfflineMapServer/releases/latest/download/updater_manifest.txt"
#else
#define MANIFEST_URL "http://github.com/bsaid/QOfflineMapServer/releases/latest/download/updater_manifest.txt"
#endif



QNetworkRequest Updater::getNetworkRequest(const QUrl& url)
{
    QNetworkRequest req(url);
    req.setRawHeader( "User-Agent", "Mozilla/5.0 (X11; U; Linux i686 (x86_64); "
                          "en-US; rv:1.9.0.1) Gecko/2008070206 Firefox/3.0.1" );
    req.setRawHeader( "Accept-Charset", "win1251,utf-8;q=0.7,*;q=0.7" );
    req.setRawHeader( "charset", "utf-8" );
    req.setRawHeader( "Connection", "keep-alive" );
    return req;
}

UpdateCheckResult Updater::checkManifest()
{
    QUrl baseUrl(MANIFEST_URL);
    QNetworkAccessManager manager;
    QNetworkRequest request = getNetworkRequest(baseUrl);
    QNetworkReply *rep = manager.get(request);

    while(rep->error() == QNetworkReply::NoError && !rep->isFinished())
        QCoreApplication::processEvents();

    while(true)
    {
        while(!rep->isFinished())
            QCoreApplication::processEvents();

        if(rep->error() != QNetworkReply::NoError)
            return UpdateCheckResult(RES_CHECK_FAILED, rep->error(), rep->errorString());

        QVariant redirect = rep->attribute(QNetworkRequest::RedirectionTargetAttribute);
        if(redirect.type() != QVariant::Url)
            break;

        // redirect
        baseUrl = baseUrl.resolved(redirect.toUrl());
        request = getNetworkRequest(baseUrl);
        rep = manager.get(request);
    }

    if(rep->isFinished() && rep->size() != 0)
    {
        QString s;
        QString ver(VERSION);
        while(!rep->atEnd())
        {
            s = rep->readLine();

            QStringList parts = s.split(' ', QString::SkipEmptyParts);
            if(parts.size() < 3 || !ver.contains(parts[0]))
                continue;

            if(REVISION < parts[1].toInt())
                return UpdateCheckResult(RES_UPDATE_AVAILABLE);
            else
                return UpdateCheckResult(RES_NO_UPDATE);
        }
    }
    return UpdateCheckResult(RES_NO_UPDATE);
}

void Updater::checkForUpdate(bool autoCheck)
{
    UpdateHandler *h = new UpdateHandler(autoCheck);
    QFuture<UpdateCheckResult> f = QtConcurrent::run(&Updater::checkManifest);
    h->createWatcher(f);
}

bool Updater::copyUpdater()
{
    if(QFile::exists("tmp_updater.exe") && !QFile::remove("tmp_updater.exe"))
        return false;

    if(!QFile::copy("updater.exe", "tmp_updater.exe"))
        return false;
    return true;
}

bool Updater::startUpdater()
{
    if(!copyUpdater() ||
       !QProcess::startDetached("tmp_updater.exe", (QStringList() << VERSION << QString::number(REVISION))))
    {
        Utils::showErrorBox(QObject::tr("Could not start updater.exe, you have to download new version manually!\n"
                                 "<a href='http://tasssadar.github.com/Lorris'>http://tasssadar.github.com/Lorris</a>"));
        return false;
    }
    return true;
}

UpdateHandler::UpdateHandler(bool autoCheck, QObject *parent) : QObject(parent)
{
    m_autoCheck = autoCheck;
    if(!autoCheck)
    {
        m_progress = new ToolTipWarn(tr("Checking for updates..."), NULL, NULL, -1);
        m_progress->showSpinner();
        m_progress->toRightBottom();
    }
}

void UpdateHandler::updateBtn()
{
    QPushButton *btn = dynamic_cast<QPushButton*>(sender());
    if(btn) {
        btn->setEnabled(false);
        btn->setText(tr("Starting..."));
        qApp->processEvents();
    }

    if(Updater::startUpdater())
        qApp->closeAllWindows();
    deleteLater();
}

void UpdateHandler::createWatcher(const QFuture<UpdateCheckResult> &f)
{
    m_watcher = new QFutureWatcher<UpdateCheckResult>(this);

    connect(m_watcher, SIGNAL(finished()), SLOT(updateCheckResult()));
    m_watcher->setFuture(f);
}

void UpdateHandler::updateCheckResult()
{
    if(!m_progress.isNull())
        m_progress->deleteLater();

    UpdateCheckResult res = m_watcher->result();
    if(m_autoCheck && (res.code == RES_CHECK_FAILED || res.code == RES_NO_UPDATE))
    {
        deleteLater();
        return;
    }

    static const QString texts[] = {
        tr("Update check has failed: %1 %2!"),
        tr("No update available"),
        tr("New update for Lorris is available")
    };

    static const char *icons[] = {
        ":/icons/warning",
        ":/icons/info",
        ":/icons/update"
    };

    static const int times[] = { 4000, 3000, 30000 };

    Q_ASSERT(res.code >= 0 && res.code < RES_MAX);

    QString text = texts[res.code].arg(int(res.error)).arg(res.errorString);
    ToolTipWarn *w = new ToolTipWarn(text, NULL, NULL, times[res.code], icons[res.code]);

    if(res.code == RES_UPDATE_AVAILABLE) {
        QPushButton *btn = new QPushButton(tr("Download"));
        w->setButton(btn);
        connect(btn, SIGNAL(clicked()), this, SLOT(updateBtn()));
        connect(btn, SIGNAL(clicked()), w, SLOT(deleteLater()));
    } else {
        deleteLater();
    }
    w->toRightBottom();
}


