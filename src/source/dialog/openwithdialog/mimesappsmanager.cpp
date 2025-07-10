// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwithdialog/mimesappsmanager.h"
#include "openwithdialog/dmimedatabase.h"
#include "openwithdialog/singleton.h"
#include "openwithdialog/desktopfile.h"
#include "openwithdialog/dfmstandardpaths.h"
#include "openwithdialog/durl.h"

#include <QDir>
#include <QSettings>
#include <QMimeType>
#include <QDirIterator>
#include <QDateTime>
#include <QThread>
#include <QStandardPaths>
#include <QDebug>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QApplication>


#undef signals
extern "C" {
#include <gio/gio.h>
#include <gio/gdesktopappinfo.h>
#include <gio/gappinfo.h>
}
#define signals public

QStringList MimesAppsManager::DesktopFiles = {};
QMap<QString, QStringList> MimesAppsManager::MimeApps = {};
QMap<QString, QStringList> MimesAppsManager::DDE_MimeTypes = {};
QMap<QString, DesktopFile> MimesAppsManager::VideoMimeApps = {};
QMap<QString, DesktopFile> MimesAppsManager::ImageMimeApps = {};
QMap<QString, DesktopFile> MimesAppsManager::TextMimeApps = {};
QMap<QString, DesktopFile> MimesAppsManager::AudioMimeApps = {};

QMap<QString, DesktopFile> MimesAppsManager::DesktopObjs = {};

MimeAppsWorker::MimeAppsWorker(QObject *parent): QObject(parent)
{
    qDebug() << "Initializing MimeAppsWorker";
    m_fileSystemWatcher = new QFileSystemWatcher(this);
    m_updateCacheTimer = new QTimer(this);
    m_updateCacheTimer->setInterval(2000);
    m_updateCacheTimer->setSingleShot(true);
    startWatch();
    initConnect();
    qDebug() << "MimeAppsWorker initialized";
}

MimeAppsWorker::~MimeAppsWorker()
{
    qDebug() << "Destroying MimeAppsWorker";
}

void MimeAppsWorker::initConnect()
{
    qDebug() << "Initializing MimeAppsWorker connections";
    connect(m_fileSystemWatcher, &QFileSystemWatcher::directoryChanged, this, &MimeAppsWorker::handleDirectoryChanged);
    connect(m_fileSystemWatcher, &QFileSystemWatcher::fileChanged, this, &MimeAppsWorker::handleFileChanged);
    connect(m_updateCacheTimer, &QTimer::timeout, this, &MimeAppsWorker::updateCache);
    qDebug() << "MimeAppsWorker connections established";
}

void MimeAppsWorker::startWatch()
{
    qDebug() << "Starting file system watcher";
    m_fileSystemWatcher->addPaths(MimesAppsManager::getDesktopFiles());
    m_fileSystemWatcher->addPaths(MimesAppsManager::getApplicationsFolders());
}

void MimeAppsWorker::handleDirectoryChanged(const QString &filePath)
{
    qDebug() << "Directory changed:" << filePath;
    Q_UNUSED(filePath)

    //for 1.4
//    if(QFile::exists(filePath)){
//        m_fileSystemWatcher->addPath(filePath);

//        QMap<QString, QSet<QString>> mimeAppsSet;

//        DesktopFile desktopFile(filePath);
//        MimesAppsManager::DesktopFiles.append(filePath);
//        MimesAppsManager::DesktopObjs.insert(filePath, desktopFile);
//        QStringList mimeTypes = desktopFile.getMimeType();
//        foreach (QString mimeType, mimeTypes) {
//            if (!mimeType.isEmpty()){
//                QSet<QString> apps;
//                if (mimeAppsSet.contains(mimeType)){
//                    apps = mimeAppsSet.value(mimeType);
//                    apps.insert(filePath);
//                }else{
//                    apps.insert(filePath);
//                }
//                mimeAppsSet.insert(mimeType, apps);
//            }
//        }

//        foreach (QString key, mimeAppsSet.keys()) {
//            QSet<QString> apps = mimeAppsSet.value(key);
//            QStringList orderApps;
//            if (apps.count() > 1){
//                QFileInfoList fileInfos;
//                foreach (QString app, apps) {
//                    QFileInfo info(app);
//                    fileInfos.append(info);
//                }

//                std::sort(fileInfos.begin(), fileInfos.end(), MimesAppsManager::lessByDateTime);

//                foreach (QFileInfo info, fileInfos) {
//                    orderApps.append(info.absoluteFilePath());
//                }
//            }else{
//                orderApps.append(apps.toList());
//            }
//            MimesAppsManager::MimeApps.insert(key, orderApps);
//        }

//        foreach (QString desktopFile, MimesAppsManager::DesktopObjs.keys()) {
//            QString iconName = MimesAppsManager::DesktopObjs.value(desktopFile).getIcon();
//            fileIconProvider->getDesktopIcon(iconName, 48);
//        }

//    }else{
//        m_fileSystemWatcher->removePath(filePath);
//        MimesAppsManager::DesktopFiles.removeOne(filePath);
//        MimesAppsManager::DesktopObjs.remove(filePath);
//    }
//    updateCache();
    qDebug() << "Starting cache update timer due to directory change";
    m_updateCacheTimer->start();
}

void MimeAppsWorker::handleFileChanged(const QString &filePath)
{
    qDebug() << "File changed:" << filePath;
    Q_UNUSED(filePath)
//    updateCache();
    m_updateCacheTimer->start();
    //for 1.4
//    DesktopFile desktopFile(filePath);
//    MimesAppsManager::DesktopObjs.remove(filePath);
//    MimesAppsManager::DesktopObjs.insert(filePath, desktopFile);

//    QMap<QString, QSet<QString>> mimeAppsSet;

//    QStringList mimeTypes = desktopFile.getMimeType();
//    foreach (QString mimeType, mimeTypes) {
//        if (!mimeType.isEmpty()){
//            QSet<QString> apps;
//            if (mimeAppsSet.contains(mimeType)){
//                apps = mimeAppsSet.value(mimeType);
//                apps.insert(filePath);
//            }else{
//                apps.insert(filePath);
//            }
//            mimeAppsSet.insert(mimeType, apps);
//        }
//    }

//    foreach (QString key, mimeAppsSet.keys()) {
//        QSet<QString> apps = mimeAppsSet.value(key);
//        QStringList orderApps;
//        if (apps.count() > 1){
//            QFileInfoList fileInfos;
//            foreach (QString app, apps) {
//                QFileInfo info(app);
//                fileInfos.append(info);
//            }

//            std::sort(fileInfos.begin(), fileInfos.end(), MimesAppsManager::lessByDateTime);

//            foreach (QFileInfo info, fileInfos) {
//                orderApps.append(info.absoluteFilePath());
//            }
//        }else{
//            orderApps.append(apps.toList());
//        }
//        MimesAppsManager::MimeApps.insert(key, orderApps);
//    }

//    foreach (QString desktopFile, MimesAppsManager::DesktopObjs.keys()) {
//        QString iconName = MimesAppsManager::DesktopObjs.value(desktopFile).getIcon();
//        fileIconProvider->getDesktopIcon(iconName, 48);
//    }
}

void MimeAppsWorker::updateCache()
{
    qDebug() << "Updating MIME type apps cache";
    MimesAppsManager::initMimeTypeApps();
    qDebug() << "MIME type apps cache updated";
}

bool MimeAppsWorker::writeData(const QString &path, const QByteArray &content)
{
    qDebug() << "Writing data to path:" << path << "size:" << content.size();
    bool bResult = false;
    qInfo() << path;
    QFile file(path);
    if (file.open(QFile::WriteOnly)) {
        file.write(content);
        bResult = true;
        qDebug() << "Successfully wrote data to file";
    } else {
        qWarning() << "Failed to open file for writing:" << file.errorString();
    }

    file.close();
    return bResult;
}

QByteArray MimeAppsWorker::readData(const QString &path)
{
    qDebug() << "Reading data from path:" << path;
    QFile file(path);
    if (!file.open(QFile::ReadOnly)) {
        qInfo() << path << "isn't exists!";
        return QByteArray();
    }

    QByteArray content = file.readAll();
    file.close();
    qDebug() << "Read" << content.size() << "bytes from file";
    return content;
}

MimesAppsManager::MimesAppsManager(QObject *parent): QObject(parent)
{
    qDebug() << "Creating MimesAppsManager";
    m_mimeAppsWorker = new MimeAppsWorker(this);
    connect(this, &MimesAppsManager::requestUpdateCache, m_mimeAppsWorker, &MimeAppsWorker::updateCache);
    QThread *mimeAppsThread = new QThread;
    m_mimeAppsWorker->moveToThread(mimeAppsThread);
    mimeAppsThread->start();
    qDebug() << "MimesAppsManager initialized with worker thread";
}

MimesAppsManager::~MimesAppsManager()
{
    qDebug() << "Destroying MimesAppsManager";
}

QMimeType MimesAppsManager::getMimeType(const QString &fileName)
{
    qDebug() << "Getting MIME type for file:" << fileName;
    DMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(fileName);
    qDebug() << "MIME type:" << mimeType.name();
    return mimeType;
}

QString MimesAppsManager::getMimeTypeByFileName(const QString &fileName)
{
    qDebug() << "Getting MIME type by filename:" << fileName;
    DMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(fileName);
    qDebug() << "MIME type result:" << mimeType.name();
    return mimeType.name();
}

QString MimesAppsManager::getDefaultAppByFileName(const QString &fileName)
{
    qDebug() << "Getting default app by filename:" << fileName;
    DMimeDatabase db;
    QMimeType mimeType = db.mimeTypeForFile(fileName);
    QString result = getDefaultAppByMimeType(mimeType);
    qDebug() << "Default app result:" << result;
    return result;
}

QString MimesAppsManager::getDefaultAppByMimeType(const QMimeType &mimeType)
{
    qDebug() << "Getting default app by QMimeType:" << mimeType.name();
    QString result = getDefaultAppByMimeType(mimeType.name());
    qDebug() << "Default app result:" << result;
    return result;
}

QString MimesAppsManager::getDefaultAppByMimeType(const QString &mimeType)
{
    qDebug() << "Getting default app for MIME type:" << mimeType;
    GAppInfo *defaultApp = g_app_info_get_default_for_type(mimeType.toLocal8Bit().constData(), FALSE);
    QString url = "";
    if (defaultApp) {
        url = g_app_info_get_id(defaultApp);
        qDebug() << "Default app:" << url;
    } else {
        qWarning() << "No default app found for MIME type:" << mimeType;
    }

    return url;
}

QString MimesAppsManager::getDefaultAppDisplayNameByMimeType(const QMimeType &mimeType)
{
    qDebug() << "Getting default app display name by QMimeType:" << mimeType.name();
    QString result = getDefaultAppDisplayNameByGio(mimeType.name());
    qDebug() << "Display name result:" << result;
    return result;
}

QString MimesAppsManager::getDefaultAppDisplayNameByGio(const QString &mimeType)
{
    qDebug() << "Getting default app display name by GIO for MIME type:" << mimeType;
    /*
        *
        * We have the appsForMimeList. Now we need to filter some applications out as per user's choice and get the default value
        * First check mimeapps.list/[Default Associations], then mimeapps.list/[Added Associations]. The entry corresponding to the mimetype in
        * the first case and the first entry in the second case are the user defaults.
        * If the mimetype is not listed, then check mimeinfo.cache
        * Do the same for /usr/local/share/applications and /usr/share/applications
        *
    */

    GAppInfo *defaultApp = g_app_info_get_default_for_type(mimeType.toLocal8Bit().constData(), FALSE);
    QString appDisplayName = "";
    if (defaultApp) {
        appDisplayName = g_app_info_get_name(defaultApp);
        qDebug() << "Found default app display name:" << appDisplayName;
    } else {
        qWarning() << "No default app found for MIME type:" << mimeType;
    }

    g_object_unref(defaultApp);
    return appDisplayName;
}

QString MimesAppsManager::getDefaultAppDesktopFileByMimeType(const QString &mimeType)
{
    qDebug() << "Getting default app desktop file by MIME type:" << mimeType;
    GAppInfo *defaultApp = g_app_info_get_default_for_type(mimeType.toLocal8Bit().constData(), FALSE);
    if (!defaultApp) {
        qWarning() << "No default app found for MIME type:" << mimeType;
        return "";
    }

    const char *desktop_id = g_app_info_get_id(defaultApp);
    GDesktopAppInfo *desktopAppInfo = g_desktop_app_info_new(desktop_id);
    if (!desktopAppInfo) {
        qWarning() << "Failed to create desktop app info for:" << desktop_id;
        g_object_unref(defaultApp);
        return "";
    }
    QString desktopFile = g_desktop_app_info_get_filename(desktopAppInfo);
    qDebug() << "Default app desktop file:" << desktopFile;

    g_object_unref(defaultApp);
    g_object_unref(desktopAppInfo);

    return desktopFile;
}

bool MimesAppsManager::setDefautlAppForTypeByGio(const QString &mimeType, const QString &appPath)
{
    qDebug() << "Setting default app for MIME type:" << mimeType << "app:" << appPath;
    GAppInfo *app = NULL;
    GList *apps = NULL;
    apps = g_app_info_get_all();
    qDebug() << "Found" << g_list_length(apps) << "available apps";

    GList *iterator = apps;
    int checkedApps = 0;

    while (iterator) {
        checkedApps++;
        const char *desktop_id = g_app_info_get_id((GAppInfo *)iterator->data);
        GDesktopAppInfo *desktopAppInfo = g_desktop_app_info_new(desktop_id);

        if (desktopAppInfo) {
            QString filename = g_desktop_app_info_get_filename(desktopAppInfo);
            qDebug() << "Checking app" << checkedApps << ":" << filename;
            
            if (appPath == filename) {
                app = (GAppInfo *)iterator->data;
                g_object_unref(desktopAppInfo);
                qDebug() << "Found matching app:" << filename;
                break;
            }

            g_object_unref(desktopAppInfo);
        }

        if (appPath.endsWith("/" + QString::fromLocal8Bit(desktop_id))) {
            app = (GAppInfo *)iterator->data;
            qDebug() << "Found matching app by ID:" << desktop_id;
            break;
        }

        iterator = iterator->next;
    }

    g_list_free(apps);

    if (!app) {
        qWarning() << "No app found matching:" << appPath;
        return false;
    }

    GError *error = NULL;
    qDebug() << "Setting default app for MIME type";
    g_app_info_set_as_default_for_type(app,
                                       mimeType.toLocal8Bit().constData(),
                                       &error);
    if (error) {
        qCritical() << "Failed to set default app:" << error->message;
        g_free(error);
        return false;
    }

    qInfo() << "Successfully set default app for" << mimeType;
    return true;
}

QStringList MimesAppsManager::getRecommendedAppsByQio(const QMimeType &mimeType)
{
    qDebug() << "Getting recommended apps for MIME type:" << mimeType.name();
    QStringList recommendApps;
    QList<QMimeType> mimeTypeList;
    QMimeDatabase mimeDatabase;

    mimeTypeList.append(mimeType);

    while (recommendApps.isEmpty()) {
        for (const QMimeType &type : mimeTypeList) {
            QStringList type_name_list;

            type_name_list.append(type.name());
            type_name_list.append(type.aliases());

            foreach (const QString &name, type_name_list) {
                foreach (const QString &app, mimeAppsManager->MimeApps.value(name)) {
                    bool app_exist = false;

                    for (const QString &other : recommendApps) {
                        const DesktopFile &app_desktop = mimeAppsManager->DesktopObjs.value(app);
                        const DesktopFile &other_desktop = mimeAppsManager->DesktopObjs.value(other);

                        if (app_desktop.getExec() == other_desktop.getExec() && app_desktop.getLocalName() == other_desktop.getLocalName()) {
                            app_exist = true;
                            break;
                        }
                    }

                    // if desktop file was not existed do not recommend!!
                    if (!QFileInfo::exists(app)) {
                        qWarning() << app << "not exist anymore";
                        continue;
                    }

                    if (!app_exist)
                        recommendApps.append(app);
                }
            }
        }

        if (!recommendApps.isEmpty())
            break;

        QList<QMimeType> newMimeTypeList;

        for (const QMimeType &type : mimeTypeList) {
            for (const QString &name : type.parentMimeTypes())
                newMimeTypeList.append(mimeDatabase.mimeTypeForName(name));
        }

        mimeTypeList = newMimeTypeList;

        if (mimeTypeList.isEmpty())
            break;
    }

    return recommendApps;
}

QStringList MimesAppsManager::getRecommendedAppsByGio(const QString &mimeType)
{
    qDebug() << "Getting recommended apps by GIO for MIME type:" << mimeType;
    QStringList recommendApps;
    GList *recomendAppInfoList = g_app_info_get_recommended_for_type(mimeType.toLocal8Bit().constData());
    GList *iterator = recomendAppInfoList;
    qDebug() << "Found" << g_list_length(recomendAppInfoList) << "recommended apps";

    while (iterator) {
        GAppInfo *appInfo = (GAppInfo *)iterator->data;
        if (appInfo) {
            const char *desktopId = g_app_info_get_id(appInfo);
            qDebug() << "Processing recommended app:" << desktopId;

            GDesktopAppInfo *dekstopAppInfo = g_desktop_app_info_new(desktopId);
            QString app = g_desktop_app_info_get_filename(dekstopAppInfo);
            qDebug() << "App filename:" << app;

            recommendApps << app;
            g_object_unref(dekstopAppInfo);
        }

        iterator = iterator->next;
    }

    g_list_free(recomendAppInfoList);
    qDebug() << "Returning" << recommendApps.size() << "recommended apps";
    return recommendApps;
}

QStringList MimesAppsManager::getApplicationsFolders()
{
    qDebug() << "Getting application folders";
    QStringList desktopFolders;
    desktopFolders << QString("/usr/share/applications/")
                   << QString("/usr/local/share/applications/")
                   << QString("/usr/share/gnome/applications/")
                   << QString("/var/lib/flatpak/exports/share/applications")
                   << QDir::homePath() + QString("/.local/share/flatpak/exports/share/applications")
                   << QDir::homePath() + QString("/.local/share/applications");
    qDebug() << "Application folders:" << desktopFolders;
    return desktopFolders;
}

QString MimesAppsManager::getMimeAppsCacheFile()
{
    QString path = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::CachePath), "MimeApps.json");
    qDebug() << "MIME apps cache file path:" << path;
    return path;
}

QString MimesAppsManager::getMimeInfoCacheFilePath()
{
    qDebug() << "MIME info cache file path: /usr/share/applications/mimeinfo.cache";
    return "/usr/share/applications/mimeinfo.cache";
}

QString MimesAppsManager::getMimeInfoCacheFileRootPath()
{
    qDebug() << "MIME info cache file root path: /usr/share/applications";
    return "/usr/share/applications";
}

QString MimesAppsManager::getDesktopFilesCacheFile()
{
    QString path = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::CachePath), "DesktopFiles.json");
    qDebug() << "Desktop files cache file path:" << path;
    return path;
}

QString MimesAppsManager::getDesktopIconsCacheFile()
{
    QString path = QString("%1/%2").arg(DFMStandardPaths::location(DFMStandardPaths::CachePath), "DesktopIcons.json");
    qDebug() << "Desktop icons cache file path:" << path;
    return path;
}

QStringList MimesAppsManager::getDesktopFiles()
{
    qDebug() << "Getting desktop files from application folders";
    QStringList desktopFiles;

    foreach (QString desktopFolder, getApplicationsFolders()) {
        qDebug() << "Scanning folder:" << desktopFolder;
        QDirIterator it(desktopFolder, QStringList("*.desktop"),
                        QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        int fileCount = 0;
        while (it.hasNext()) {
            it.next();
            desktopFiles.append(it.filePath());
            fileCount++;
        }
        qDebug() << "Found" << fileCount << "desktop files in" << desktopFolder;
    }

    qDebug() << "Total desktop files found:" << desktopFiles.size();
    return desktopFiles;
}

QString MimesAppsManager::getDDEMimeTypeFile()
{
    QString path = QString("%1/%2/%3").arg(getMimeInfoCacheFileRootPath(), "deepin", "dde-mimetype.list");
    qDebug() << "DDE MIME type file path:" << path;
    return path;
}

QMap<QString, DesktopFile> MimesAppsManager::getDesktopObjs()
{
    qDebug() << "Creating desktop objects map";
    QMap<QString, DesktopFile> desktopObjs;
    QStringList appFolders = getApplicationsFolders();
    foreach (QString f, appFolders) {
        // qDebug() << "Creating desktop object for:" << f;
        desktopObjs.insert(f, DesktopFile(f));
    }
    qDebug() << "Created" << desktopObjs.size() << "desktop objects";
    return desktopObjs;
}

void MimesAppsManager::initMimeTypeApps()
{
    qDebug() << "Initializing MIME type apps";
    DesktopFiles.clear();
    DesktopObjs.clear();
    DDE_MimeTypes.clear();

    QMap<QString, QSet<QString>> mimeAppsSet;
    loadDDEMimeTypes();
    foreach (QString desktopFolder, getApplicationsFolders()) {
        QDirIterator it(desktopFolder, QStringList("*.desktop"),
                        QDir::Files | QDir::NoDotAndDotDot,
                        QDirIterator::Subdirectories);
        while (it.hasNext()) {
            it.next();
            QString filePath = it.filePath();
            DesktopFile desktopFile(filePath);
            DesktopFiles.append(filePath);
            DesktopObjs.insert(filePath, desktopFile);
            QStringList mimeTypes = desktopFile.getMimeType();
            QString fileName = QFileInfo(filePath).fileName();
            if (DDE_MimeTypes.contains(fileName)) {
                mimeTypes.append(DDE_MimeTypes.value(fileName));
            }

            foreach (QString mimeType, mimeTypes) {
                if (!mimeType.isEmpty()) {
                    QSet<QString> apps;
                    if (mimeAppsSet.contains(mimeType)) {
                        apps = mimeAppsSet.value(mimeType);
                        apps.insert(filePath);
                    } else {
                        apps.insert(filePath);
                    }
                    mimeAppsSet.insert(mimeType, apps);
                }
            }
        }
    }

    qDebug() << "Processing MIME type apps from cache";
    foreach (QString key, mimeAppsSet.keys()) {
        QSet<QString> apps = mimeAppsSet.value(key);
        QStringList orderApps;
        if (apps.count() > 1) {
            // qDebug() << "Sorting" << apps.count() << "apps for MIME type:" << key;
            QFileInfoList fileInfos;
            foreach (QString app, apps) {
                QFileInfo info(app);
                fileInfos.append(info);
            }

            std::sort(fileInfos.begin(), fileInfos.end(), MimesAppsManager::lessByDateTime);

            foreach (QFileInfo info, fileInfos) {
                orderApps.append(info.absoluteFilePath());
            }
        } else {
            orderApps.append(apps.values());
        }

        MimeApps.insert(key, orderApps);
    }

    //check mime apps from cache
    qDebug() << "Reading MIME info cache file:" << getMimeInfoCacheFilePath();
    QFile f(getMimeInfoCacheFilePath());
    if (!f.open(QIODevice::ReadOnly)) {
        qInfo() << "failed to read mime info cache file:" << f.errorString();
        return;
    }

    QStringList audioDesktopList;
    QStringList imageDeksopList;
    QStringList textDekstopList;
    QStringList videoDesktopList;
    
    int lineCount = 0;
    while (!f.atEnd()) {
        lineCount++;
        QString data = f.readLine();
        QString _desktops = data.split("=").last();
        QString mimeType = data.split("=").first();
        QStringList desktops = _desktops.split(";");

        foreach (const QString desktop, desktops) {
            if (desktop.isEmpty() || audioDesktopList.contains(desktop))
                continue;

            if (mimeType.startsWith("audio")) {
                if (!audioDesktopList.contains(desktop))
                    audioDesktopList << desktop;
            } else if (mimeType.startsWith("image")) {
                if (!imageDeksopList.contains(desktop))
                    imageDeksopList << desktop;
            } else if (mimeType.startsWith("text")) {
                if (!textDekstopList.contains(desktop))
                    textDekstopList << desktop;
            } else if (mimeType.startsWith("video")) {
                if (!videoDesktopList.contains(desktop))
                    videoDesktopList << desktop;
            }
        }
    }
    qDebug() << "Processed" << lineCount << "lines from MIME info cache";
    qDebug() << "Found audio apps:" << audioDesktopList.size() << "image apps:" << imageDeksopList.size() 
             << "text apps:" << textDekstopList.size() << "video apps:" << videoDesktopList.size();

    f.close();

    const QString mimeInfoCacheRootPath = getMimeInfoCacheFileRootPath();
    qDebug() << "Loading audio MIME apps";
    foreach (QString desktop, audioDesktopList) {
        const QString path = QString("%1/%2").arg(mimeInfoCacheRootPath, desktop);
        if (!QFile::exists(path)) {
            qDebug() << "Audio desktop file not found:" << path;
            continue;
        }
        DesktopFile df(path);
        AudioMimeApps.insert(path, df);
    }
    qDebug() << "Loaded" << AudioMimeApps.size() << "audio MIME apps";

    qDebug() << "Loading image MIME apps";
    foreach (QString desktop, imageDeksopList) {
        const QString path = QString("%1/%2").arg(mimeInfoCacheRootPath, desktop);
        if (!QFile::exists(path)) {
            qDebug() << "Image desktop file not found:" << path;
            continue;
        }
        DesktopFile df(path);
        ImageMimeApps.insert(path, df);
    }
    qDebug() << "Loaded" << ImageMimeApps.size() << "image MIME apps";

    qDebug() << "Loading text MIME apps";
    foreach (QString desktop, textDekstopList) {
        const QString path = QString("%1/%2").arg(mimeInfoCacheRootPath, desktop);
        if (!QFile::exists(path)) {
            qDebug() << "Text desktop file not found:" << path;
            continue;
        }
        DesktopFile df(path);
        TextMimeApps.insert(path, df);
    }
    qDebug() << "Loaded" << TextMimeApps.size() << "text MIME apps";

    qDebug() << "Loading video MIME apps";
    foreach (QString desktop, videoDesktopList) {
        const QString path = QString("%1/%2").arg(mimeInfoCacheRootPath, desktop);
        if (!QFile::exists(path)) {
            qDebug() << "Video desktop file not found:" << path;
            continue;
        }
        DesktopFile df(path);
        VideoMimeApps.insert(path, df);
    }
    qDebug() << "Loaded" << VideoMimeApps.size() << "video MIME apps";

    qDebug() << "MIME type apps initialization completed";
    return;
}

void MimesAppsManager::loadDDEMimeTypes()
{
    qDebug() << "Loading DDE MIME types from:" << getDDEMimeTypeFile();
    QSettings settings(getDDEMimeTypeFile(), QSettings::IniFormat);

    QFile file(getDDEMimeTypeFile());
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Failed to open DDE MIME types file:" << file.errorString();
        return;
    }

    // Read propeties
    QTextStream in(&file);
    QString desktopKey;
    while (!in.atEnd()) {
        // Read new line
        QString line = in.readLine();

        // Skip empty line or line with invalid format
        if (line.trimmed().isEmpty()) {
            continue;
        }

        // Read group
        // NOTE: symbols '[' and ']' can be found not only in group names, but
        // only group can start with '['

        if (line.trimmed().startsWith("[") && line.trimmed().endsWith("]")) {
            QString tmp = line.trimmed().replace("[", "").replace("]", "");
            desktopKey = tmp;
            qDebug() << "Found desktop key:" << desktopKey;
            continue;
        }

        // If we are in correct group and line contains assignment then read data
        int first_equal = line.indexOf('=');
        if (!desktopKey.isEmpty() && first_equal >= 0) {
            QString value = line.mid(first_equal + 1);
            QStringList mimetypes = value.split(";");
            DDE_MimeTypes.insert(desktopKey, mimetypes);
            desktopKey.clear();
        }
    }

    file.close();
}

bool MimesAppsManager::lessByDateTime(const QFileInfo &f1, const QFileInfo &f2)
{
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
    bool result = f1.created() < f2.created();
    qDebug() << "Comparing files by creation time:" << f1.fileName() << f1.created() << "<" << f2.fileName() << f2.created() << "=" << result;
    return result;
#else
    bool result = f1.birthTime() < f2.birthTime();
    qDebug() << "Comparing files by birth time:" << f1.fileName() << f1.birthTime() << "<" << f2.fileName() << f2.birthTime() << "=" << result;
    return result;
#endif
}

bool MimesAppsManager::removeOneDupFromList(QStringList &list, const QString desktopFilePath)
{
    qDebug() << "Removing duplicate from list for desktop file:" << desktopFilePath;
    if (list.removeOne(desktopFilePath)) {
        qDebug() << "Removed exact match from list";
        return true;
    }

    const DesktopFile target(desktopFilePath);
    qDebug() << "Searching for duplicate by exec and name matching";

    QMutableStringListIterator iter(list);
    while (iter.hasNext()) {
        const DesktopFile source(iter.next());

        if (source.getExec() == target.getExec() && source.getLocalName() == target.getLocalName()) {
            qDebug() << "Found and removed duplicate based on exec and name";
            iter.remove();
            return true;
        }
    }

    qDebug() << "No duplicate found to remove";
    return false;
}
