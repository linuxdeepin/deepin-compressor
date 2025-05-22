// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwithdialog/dfmstandardpaths.h"
#include "openwithdialog/durl.h"

#include <QDir>
#include <QCoreApplication>
#include <QStandardPaths>
#include <QMap>
#include <QDBusInterface>
#include <QDBusMessage>
#include <QDebug>

QString DFMStandardPaths::location(DFMStandardPaths::StandardLocation type)
{
    qDebug() << "Getting standard path for type:" << type;
    switch (type) {
    case TrashPath:
        return QDir::homePath() + "/.local/share/Trash";
    case TrashFilesPath:
        return QDir::homePath() + "/.local/share/Trash/files";
    case TrashInfosPath:
        return QDir::homePath() + "/.local/share/Trash/info";
#ifdef APPSHAREDIR
    case TranslationPath: {
        QString path = APPSHAREDIR"/translations";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/translations";
        }
        return path;
    }
    case TemplatesPath: {
        QString path = APPSHAREDIR"/templates";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/templates";
        }
        return path;
    }
    case MimeTypePath: {
        QString path = APPSHAREDIR"/mimetypes";
        if (!QDir(path).exists()) {
            path = qApp->applicationDirPath() + "/mimetypes";
        }
        return path;
    }
#endif
#ifdef PLUGINDIR
    case PluginsPath: {
        QString path = PLUGINDIR;
        if (!QDir(path).exists()) {
            path = QString::fromLocal8Bit(PLUGINDIR).split(':').last();
        }
        return path;
    }
#endif
#ifdef QMAKE_TARGET
    case ApplicationConfigPath:
        return getConfigPath();
#endif
    case ThumbnailPath:
        return QDir::homePath() + "/.cache/thumbnails";
    case ThumbnailFailPath:
        return location(ThumbnailPath) + "/fail";
    case ThumbnailLargePath:
        return location(ThumbnailPath) + "/large";
    case ThumbnailNormalPath:
        return location(ThumbnailPath) + "/normal";
    case ThumbnailSmallPath:
        return location(ThumbnailPath) + "/small";
#ifdef APPSHAREDIR
    case ApplicationSharePath:
        return APPSHAREDIR;
#endif
    case RecentPath:
        return "recent:///";
    case HomePath:
        return QStandardPaths::standardLocations(QStandardPaths::HomeLocation).first();
    case DesktopPath:
        return QStandardPaths::standardLocations(QStandardPaths::DesktopLocation).first();
    case VideosPath:
        return QStandardPaths::standardLocations(QStandardPaths::MoviesLocation).first();
    case MusicPath:
        return QStandardPaths::standardLocations(QStandardPaths::MusicLocation).first();
    case PicturesPath:
        return QStandardPaths::standardLocations(QStandardPaths::PicturesLocation).first();
    case DocumentsPath:
        return QStandardPaths::standardLocations(QStandardPaths::DocumentsLocation).first();
    case DownloadsPath:
        return QStandardPaths::standardLocations(QStandardPaths::DownloadLocation).first();
    case CachePath:
        return getCachePath();
    case DiskPath:
        return QDir::rootPath();
#ifdef NETWORK_ROOT
    case NetworkRootPath:
        return NETWORK_ROOT;
#endif
#ifdef USERSHARE_ROOT
    case UserShareRootPath:
        return USERSHARE_ROOT;
#endif
#ifdef COMPUTER_ROOT
    case ComputerRootPath:
        return COMPUTER_ROOT;
#endif
    case Root:
        return "/";
    default:
        return QStringLiteral("bug://dde-file-manager-lib/interface/dfmstandardpaths.cpp#") + QT_STRINGIFY(type);
    }
}
bool DFMStandardPaths::pathControl(const QString &sPath)
{
    qDebug() << "Checking path control for:" << sPath;
    QString docPath = DFMStandardPaths::location(DFMStandardPaths::DocumentsPath);
    QString picPath = DFMStandardPaths::location(DFMStandardPaths::PicturesPath);
    qDebug() << "Documents path:" << docPath << "Pictures path:" << picPath;
    
    QDBusMessage reply;
    QDBusInterface iface("com.deepin.FileArmor1", "/com/deepin/FileArmor1", "com.deepin.FileArmor1",QDBusConnection::systemBus());
    
    if (iface.isValid()) {
        qDebug() << "DBus interface is valid";
        if(sPath.startsWith(docPath)) {
            qDebug() << "Checking document path control";
            reply = iface.call("GetApps", docPath);
        } else if(sPath.startsWith(picPath)) {
            qDebug() << "Checking picture path control";
            reply = iface.call("GetApps", picPath);
        }
    } else {
        qWarning() << "DBus interface is invalid";
    }
    
    if(reply.type() == QDBusMessage::ReplyMessage) {
        qDebug() << "Got valid DBus reply";
        QList<QString> lValue = reply.arguments().takeFirst().toStringList();
        QString strApp = QStandardPaths::findExecutable("deepin-compressor");
        qDebug() << "Found executable:" << strApp;
        
        if(lValue.contains(strApp)) {
            qInfo() << "Path control check passed";
            return true;
        }
    }

    qDebug() << "Path control final result false!";
    return false;
}
//QString DFMStandardPaths::fromStandardUrl(const DUrl &standardUrl)
//{
//    if (standardUrl.scheme() != "standard")
//        return QString();

//    static QMap<QString, QString> path_convert {
//        {"home", location(HomePath)},
//        {"desktop", location(DesktopPath)},
//        {"videos", location(VideosPath)},
//        {"music", location(MusicPath)},
//        {"pictures", location(PicturesPath)},
//        {"documents", location(DocumentsPath)},
//        {"downloads", location(DownloadsPath)}
//    };

//    const QString &path = path_convert.value(standardUrl.host());

//    if (path.isEmpty())
//        return path;

//    const QString &url_path = standardUrl.path();

//    if (url_path.isEmpty() || url_path == "/")
//        return path;

//    return path + standardUrl.path();
//}

//DUrl DFMStandardPaths::toStandardUrl(const QString &localPath)
//{
//    static QList<QPair<QString, QString>> path_convert {
//        {location(DesktopPath), "desktop"},
//        {location(VideosPath), "videos"},
//        {location(MusicPath), "music"},
//        {location(PicturesPath), "pictures"},
//        {location(DocumentsPath), "documents"},
//        {location(DownloadsPath), "downloads"},
//        {location(HomePath), "home"}
//    };

//    for (auto begin : path_convert) {
//        if (localPath.startsWith(begin.first)) {
//            const QString &path = localPath.mid(begin.first.size());

//            if (!path.isEmpty() && !path.startsWith("/"))
//                continue;

//            DUrl url;

//            url.setScheme("standard");
//            url.setHost(begin.second);

//            if (!path.isEmpty() && path != "/")
//                url.setPath(path);

//            return url;
//        }
//    }

//    return DUrl();
//}

#ifdef QMAKE_TARGET
QString DFMStandardPaths::getConfigPath()
{
    qDebug() << "Getting config path";
    QString projectName = QMAKE_TARGET;
    qDebug() << "Project name:" << projectName;
    
    bool mkdirSuccess = QDir::home().mkpath(".config");
    qDebug() << "Created .config directory:" << mkdirSuccess;
    
    QString deepinPath = QString("%1/deepin/%2/").arg(".config", projectName);
    mkdirSuccess = QDir::home().mkpath(deepinPath);
    qDebug() << "Created deepin config directory:" << mkdirSuccess;
    
    QString defaultPath = QString("%1/%2/deepin/%3").arg(QDir::homePath(), ".config", projectName);
    qDebug() << "Config path:" << defaultPath;
    
    return defaultPath;
}
#endif

QString DFMStandardPaths::getCachePath()
{
    qDebug() << "Getting cache path";
    QString projectName = qApp->applicationName();
    QDir::home().mkpath(".cache");
    QDir::home().mkpath(QString("%1/deepin/%2/").arg(".cache", projectName));
    QString defaultPath = QString("%1/%2/deepin/%3").arg(QDir::homePath(), ".cache", projectName);
    qDebug() << "Cache path:" << defaultPath;

    return defaultPath;
}

DFMStandardPaths::DFMStandardPaths()
{
    qDebug() << "DFMStandardPaths constructor";
}
