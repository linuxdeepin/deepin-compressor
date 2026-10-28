// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "uitools.h"
#include "mimetypes.h"
#include "kpluginfactory.h"
#include "kpluginloader.h"
#include "pluginmanager.h"
#include "qtcompat.h"
#include "common/ddlog.h"

#include <DStandardPaths>

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QDebug>
#include <QFileInfo>
#include <QFontInfo>
#include <QMimeType>
#include <QApplication>
#include <QMimeDatabase>
#include <QStandardPaths>
#include <QImageReader>
#include <QPixmap>
#include <QTextCodec>
#include <QRegularExpression>
#include <QUuid>
#include <QStorageInfo>
#include <QProcessEnvironment>

#include <KEncodingProber>

DCORE_USE_NAMESPACE
Q_DECLARE_METATYPE(KPluginMetaData)

QStringList UiTools::m_associtionlist = QStringList() << "file_association.file_association_type.x-7z-compressed"
                                        << "file_association.file_association_type.x-archive"
                                        << "file_association.file_association_type.x-bcpio"
                                        << "file_association.file_association_type.x-bzip"
                                        << "file_association.file_association_type.x-cpio"
                                        << "file_association.file_association_type.x-cpio-compressed"
                                        << "file_association.file_association_type.vnd.debian.binary-package"
                                        << "file_association.file_association_type.gzip"
                                        << "file_association.file_association_type.x-java-archive"
                                        << "file_association.file_association_type.x-lzma"
                                        << "file_association.file_association_type.vnd.ms-cab-compressed"
                                        << "file_association.file_association_type.vnd.rar"
                                        << "file_association.file_association_type.x-rpm"
                                        << "file_association.file_association_type.x-sv4cpio"
                                        << "file_association.file_association_type.x-sv4crc"
                                        << "file_association.file_association_type.x-tar"
                                        << "file_association.file_association_type.x-bzip-compressed-tar"
                                        << "file_association.file_association_type.x-compressed-tar"
                                        << "file_association.file_association_type.x-lzip-compressed-tar"
                                        << "file_association.file_association_type.x-lzma-compressed-tar"
                                        << "file_association.file_association_type.x-tzo"
                                        << "file_association.file_association_type.x-xz-compressed-tar"
                                        << "file_association.file_association_type.x-tarz"
                                        << "file_association.file_association_type.x-xar"
                                        << "file_association.file_association_type.x-xz"
                                        << "file_association.file_association_type.zip"
                                        << "file_association.file_association_type.x-iso9660-image"
                                        << "file_association.file_association_type.x-iso9660-appimage"
                                        << "file_association.file_association_type.x-source-rpm"
                                        << "file_association.file_association_type.x-chrome-extension";

UiTools::UiTools(QObject *parent)
    : QObject(parent)
{
    qCDebug(appLog) << "UiTools instance created";
}

UiTools::~UiTools()
{
    qCDebug(appLog) << "UiTools instance destroyed";
}

QString UiTools::getConfigPath()
{
    qCDebug(appLog) << "Getting config path";
    QDir dir(QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first())
             .filePath(qApp->organizationName()));

    QString path = dir.filePath(qApp->applicationName());
    qCDebug(appLog) << "Config path:" << path;
    return path;
}

QPixmap UiTools::renderSVG(const QString &filePath, const QSize &size)
{
    qCDebug(appLog) << "Rendering SVG:" << filePath << "size:" << size;
    QImageReader reader;
    QPixmap pixmap;

    reader.setFileName(filePath);

    if (reader.canRead()) {
        qCDebug(appLog) << "SVG can be read, scaling";
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        qCDebug(appLog) << "SVG cannot be read, loading as regular image";
        pixmap.load(filePath);
    }

    return pixmap;
}

QString UiTools::humanReadableSize(const qint64 &size, int precision)
{
    qCDebug(appLog) << "Converting size to human readable:" << size;
    if (0 == size) {
        qCDebug(appLog) << "Size is 0, returning dash";
        return "-";
    }

    double sizeAsDouble = size;
    static QStringList measures;
    if (measures.isEmpty()) {
        measures <<  "B" << "KB" << "MB" << "GB" << "TB" << "PB" << "EB" << "ZB" << "YB";
    }
    QStringListIterator it(measures);
    QString measure(it.next());
    while (sizeAsDouble >= 1024.0 && it.hasNext()) {
        measure = it.next();
        sizeAsDouble /= 1024.0;
    }

    QString result = QString::fromLatin1("%1 %2").arg(sizeAsDouble, 0, 'f', precision).arg(measure);
    qCDebug(appLog) << "Human readable size:" << result;
    return result;
}

bool UiTools::isArchiveFile(const QString &strFileName)
{
    qCDebug(appLog) << "Checking if file is archive:" << strFileName;
    QString strTransFileName = strFileName;
    UnCompressParameter::SplitType type;
    UiTools::transSplitFileName(strTransFileName, type);

    CustomMimeType mimeType = determineMimeType(strTransFileName);
    QString mime;
    if (mimeType.name().contains("application/"))
        mime = mimeType.name().remove("application/");

    bool ret = false;

    if (mime.size() > 0) {
        bool bArchive = false;
        ret = isExistMimeType(mime, bArchive); // 判断是否是归档管理器支持的压缩文件格式
        qCDebug(appLog) << "MIME type check result:" << mime << "is" << (ret ? "supported" : "not supported");
    } else {
        qCDebug(appLog) << "No valid MIME type found";
        ret = false;
    }

    if (strTransFileName.endsWith(".deb")) {    // 对deb文件识别为普通文件
        qCDebug(appLog) << "DEB file detected, treating as non-archive";
        ret = false;
    }

    if (strTransFileName.endsWith(".crx") || strTransFileName.endsWith(".apk")) {    // 对crx、apk文件识别为压缩包
        qCDebug(appLog) << "CRX/APK file detected, treating as archive";
        ret = true;
    }

    qCDebug(appLog) << "Final archive check result:" << ret;
    return ret;
}

bool UiTools::isExistMimeType(const QString &strMimeType, bool &bArchive)
{
    qCDebug(appLog) << "Checking if MIME type exists:" << strMimeType;
    QString conf = readConf();
    QStringList confList = conf.split("\n", SKIP_EMPTY_PARTS);

    bool exist = false;
    for (int i = 0; i < confList.count(); i++) {
        if (confList.at(i).contains("." + strMimeType + ":")) {
            bArchive = true;
            if (confList.at(i).contains("true")) {
                qCDebug(appLog) << "MIME type found and enabled:" << strMimeType;
                exist = true;
                break;
            } else {
                qCDebug(appLog) << "MIME type found but disabled:" << strMimeType;
                exist = false;
                continue;
            }
        }
    }

    qCDebug(appLog) << "MIME type check result:" << exist;
    return exist;
}

QString UiTools::readConf()
{
    qCDebug(appLog) << "Reading configuration file";
    const QString confDir = DStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir dir;
    if (!dir.exists(confDir + QDir::separator())) {
        qCDebug(appLog) << "Config directory does not exist, creating:" << confDir;
        dir.mkpath(confDir + QDir::separator());
    }

    const QString confPath = confDir + QDir::separator() + "deepin-compressor.confbf";
    qCDebug(appLog) << "Config file path:" << confPath;
    QFile confFile(confPath);

    // default settings
    if (!confFile.exists()) {
        qCDebug(appLog) << "Config file does not exist, creating with defaults";
        confFile.open(QIODevice::WriteOnly | QIODevice::Text);

        foreach (QString key, m_associtionlist) {
            QString content = key + ":" + "true" + "\n";
            confFile.write(content.toUtf8());
        }

        confFile.close();
    }

    QString confValue;
    bool readStatus = confFile.open(QIODevice::ReadOnly | QIODevice::Text);
    if (readStatus) {
        qCDebug(appLog) << "Successfully opened config file for reading";
        confValue = confFile.readAll();
    } else {
        qCWarning(appLog) << "Failed to open config file for reading";
    }

    confFile.close();
    qCDebug(appLog) << "Config file read, size:" << confValue.size();
    return confValue;
}

QString UiTools::toShortString(QString strSrc, int limitCounts, int left)
{
    qCDebug(appLog) << "Converting string to short format:" << strSrc << "limit:" << limitCounts;
    left = (left >= limitCounts || left <= 0) ? limitCounts / 2 : left;
    int right = limitCounts - left;
    QString displayName = "";
    displayName = strSrc.length() > limitCounts ? strSrc.left(left) + "..." + strSrc.right(right) : strSrc;
    qCDebug(appLog) << "Short string result:" << displayName;
    return displayName;
}

ReadOnlyArchiveInterface *UiTools::createInterface(const QString &fileName, bool bWrite, AssignPluginType eType/*, bool bUseLibArchive*/)
{
    qCDebug(appLog) << "Creating interface for file:" << fileName << "write mode:" << bWrite << "plugin type:" << eType;
    const CustomMimeType mimeType = determineMimeType(fileName);
    qCDebug(appLog) << "Determined MIME type:" << mimeType.name();

    QVector<Plugin *> offers;
    if (bWrite) {
        offers = PluginManager::get_instance().preferredWritePluginsFor(mimeType);
        qCDebug(appLog) << "Found" << offers.size() << "write plugins for MIME type";
    } else {
        offers = PluginManager::get_instance().preferredPluginsFor(mimeType);
        qCDebug(appLog) << "Found" << offers.size() << "read plugins for MIME type";
    }

    if (offers.isEmpty()) {
        qCWarning(appLog) << "Could not find a plugin to handle" << fileName;
        return nullptr;
    }

    //tar.lzo格式 由P7zip插件压缩mimeFromContent为"application/x-7z-compressed"，由Libarchive插件压缩mimeFromContent为"application/x-lzop"
    // 若未指定使用cli7z插件，则需要对zip和tar格式的解压屏蔽7z，选用libzip或libarchive插件
    //删除P7zip插件处理 mimeFromContent为"application/x-lzop" 的情况
    QMimeDatabase db;
    QMimeType mimeFromContent = db.mimeTypeForFile(fileName, QMimeDatabase::MatchContent);
    bool remove7zFlag = false;

    if (eType != APT_Cli7z) {
        if (((!bWrite) && (mimeType.name() == QString("application/zip") || mimeType.name() == QString("application/x-tar")))
                || ("application/x-tzo" == mimeType.name() && "application/x-lzop" == mimeFromContent.name())) {
            qCDebug(appLog) << "Setting flag to remove 7zip plugin for zip/tar/tzo formats";
            remove7zFlag = true;
        }
    }

    // pzip 插件只用于压缩，不用于读取/解压；ZIP 追加时也需要排除 pzip，回到旧版自动选择链路
    bool removePzipFlag = (mimeType.name() == QString("application/zip"))
                          && ((!bWrite) || eType == APT_AutoWithoutPzip);
    if (removePzipFlag) {
        qCDebug(appLog) << "Setting flag to remove pzip plugin for zip";
    }

    // 创建插件
    ReadOnlyArchiveInterface *pIface = nullptr;
    qCDebug(appLog) << "Iterating through" << offers.size() << "plugins";
    for (Plugin *plugin : offers) {
        //删除P7zip插件
        if (remove7zFlag && plugin->metaData().name().contains("7zip", Qt::CaseInsensitive)) {
            qCDebug(appLog) << "Skipping 7zip plugin due to remove flag";
            continue;
        }

        // 读取 ZIP 时跳过 pzip 插件（pzip 只用于压缩）
        if (removePzipFlag && plugin->metaData().name().contains("pzip", Qt::CaseInsensitive)) {
            qCDebug(appLog) << "Skipping pzip plugin for reading (pzip is write-only)";
            continue;
        }

        switch (eType) {
        // 自动识别
        case APT_Auto:
        case APT_AutoWithoutPzip:
            qCDebug(appLog) << "Using auto-detect plugin type";
            pIface = createInterface(fileName, mimeType, plugin);
            break;
        // cli7zplugin
        case APT_Cli7z:
            if (plugin->metaData().name().contains("7zip", Qt::CaseInsensitive)) {
                qCDebug(appLog) << "Using 7zip plugin";
                pIface = createInterface(fileName, mimeType, plugin);
            }
            break;
        // libarchive
        case APT_Libarchive:
            if (plugin->metaData().name().contains("libarchive", Qt::CaseInsensitive)) {
                qCDebug(appLog) << "Using libarchive plugin";
                pIface = createInterface(fileName, mimeType, plugin);
            }
            break;
        // libzipplugin
        case APT_Libzip:
            if (plugin->metaData().name().contains("libzip", Qt::CaseInsensitive)) {
                qCDebug(appLog) << "Using libzip plugin";
                pIface = createInterface(fileName, mimeType, plugin);
            }
            break;
        // libpigzplugin
        case APT_Libpigz:
            if (plugin->metaData().name().contains("pigz", Qt::CaseInsensitive)) {
                qCDebug(appLog) << "Using pigz plugin";
                pIface = createInterface(fileName, mimeType, plugin);
            }
            break;
        }

        // Use the first valid plugin, according to the priority sorting.
        if (pIface) {
            qCInfo(appLog) << "选用插件：" << plugin->metaData().pluginId();
            qCDebug(appLog) << "Successfully created interface with plugin";
            break;
        } else {
            qCDebug(appLog) << "Failed to create interface with plugin:" << plugin->metaData().pluginId();
        }
    }

    if (!pIface) {
        qCWarning(appLog) << "No suitable plugin found for file:" << fileName;
    }
    return pIface;
}

ReadOnlyArchiveInterface *UiTools::createInterface(const QString &fileName, const CustomMimeType &mimeType, Plugin *plugin)
{
    qCDebug(appLog) << "Creating interface with plugin:" << plugin->metaData().pluginId() << "for file:" << fileName;
    Q_ASSERT(plugin);

    KPluginFactory *factory = KPluginLoader(plugin->metaData().fileName()).factory();
    if (!factory) {
        qCWarning(appLog) << "Failed to load plugin factory for:" << plugin->metaData().fileName();
        return nullptr;
    }

    const QVariantList args = {QVariant(QFileInfo(fileName).absoluteFilePath()),
                               QVariant().fromValue(plugin->metaData()),
                               QVariant::fromValue(mimeType)
                              };

    ReadOnlyArchiveInterface *iface = factory->create<ReadOnlyArchiveInterface>(nullptr, args);
    if (iface) {
        qCDebug(appLog) << "Successfully created interface";
    } else {
        qCWarning(appLog) << "Failed to create interface";
    }
    return iface;
}

void UiTools::transSplitFileName(QString &fileName, UnCompressParameter::SplitType &eSplitType)
{
    qCDebug(appLog) << "Processing split filename:" << fileName;
    if (fileName.contains(".7z.")) {
        qCDebug(appLog) << "Processing 7z split archive";
        // 7z分卷处理
        REG_EXP reg("^([\\s\\S]*\\.)7z\\.[0-9]{3}$"); // QRegExp reg("[*.]part\\d+.rar$"); //rar分卷不匹配
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (reg.exactMatch(fileName)) {
            qCDebug(appLog) << "7z split pattern matched, converting to .001";
            fileName = reg.cap(1) + "7z.001"; //例如: *.7z.003 -> *.7z.001
#else
        QRegularExpressionMatch match = reg.match(fileName);
        if (match.hasMatch()) {
            qCDebug(appLog) << "7z split pattern matched, converting to .001";
            fileName = match.captured(1) + "7z.001"; //例如: *.7z.003 -> *.7z.001
#endif
            eSplitType = UnCompressParameter::ST_Other;
        }
    } else if (fileName.contains(".part") && fileName.endsWith(".rar")) {
        // rar分卷处理
        qCDebug(appLog) << "Processing RAR split archive";
        int x = fileName.lastIndexOf("part");
        int y = fileName.lastIndexOf(".");

        if ((y - x) > 5) {
            qCDebug(appLog) << "Converting RAR split to part01 format";
            fileName.replace(x, y - x, "part01");
        } else {
            qCDebug(appLog) << "Converting RAR split to part1 format";
            fileName.replace(x, y - x, "part1");
        }

        eSplitType = UnCompressParameter::ST_Other;
    } else if (fileName.contains(".zip.")) { // 1.zip.001格式
        qCDebug(appLog) << "Processing ZIP split archive";
        REG_EXP reg("^([\\s\\S]*\\.)zip\\.[0-9]{3}$");
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (reg.exactMatch(fileName)) {
            QFileInfo fi(reg.cap(1) + "zip.001");
            if (fi.exists() == true) {
                qCDebug(appLog) << "ZIP split .001 file exists, converting";
                fileName = reg.cap(1) + "zip.001";
                eSplitType = UnCompressParameter::ST_Zip;
            }
        }
#else
        QRegularExpressionMatch match = reg.match(fileName);
        if (match.hasMatch()) {
            QFileInfo fi(match.captured(1) + "zip.001");
            if (fi.exists() == true) {
                qCDebug(appLog) << "ZIP split .001 file exists, converting";
                fileName = match.captured(1) + "zip.001";
                eSplitType = UnCompressParameter::ST_Zip;
            }
        }
#endif
    } else if (fileName.endsWith(".zip")) { //1.zip 1.01格式
        /**
         * 例如123.zip文件，检测123.z01文件是否存在
         * 如果存在，则认定123.zip是分卷包
         */
        qCDebug(appLog) << "Checking for ZIP split format (.z01)";
        QFileInfo tmp(fileName.left(fileName.length() - 2) + "01");
        if (tmp.exists()) {
            qCDebug(appLog) << "Found corresponding .z01 file, marking as split";
            eSplitType = UnCompressParameter::ST_Zip;
        }
    } else if (fileName.contains(".z")) {  //1.zip 1.01格式
        /**
         * 例如123.z01文件，检测123.zip文件是否存在
         * 如果存在，则认定123.z01是分卷包
         */
        qCDebug(appLog) << "Processing .z## split file";
        REG_EXP reg("^([\\s\\S]*\\.)z[0-9]+$"); //
#if QT_VERSION < QT_VERSION_CHECK(6, 0, 0)
        if (reg.exactMatch(fileName)) {
            qCDebug(appLog) << "Converting .z## to .zip format";
            fileName = reg.cap(1) + "zip";
#else
        QRegularExpressionMatch match = reg.match(fileName);
        if (match.hasMatch()) {
            qCDebug(appLog) << "Converting .z## to .zip format";
            fileName = match.captured(1) + "zip";
#endif
            QFileInfo fi(fileName);
            if (fi.exists()) {
                qCDebug(appLog) << "Corresponding .zip file exists, marking as split";
                eSplitType = UnCompressParameter::ST_Zip;
            }
        }
    }

    // 对软链接文件，需要找到源文件进行处理，否则某些操作会改变软链接属性
    QFileInfo info(fileName);
    if (info.isSymLink()) {
        qCDebug(appLog) << "File is symlink, resolving target:" << info.symLinkTarget();
        fileName = info.symLinkTarget();
    }
    qCDebug(appLog) << "Final processed filename:" << fileName;
}

QString UiTools::handleFileName(const QString &strFileName)
{
    qCDebug(appLog) << "Handling filename:" << strFileName;
    QFileInfo info(strFileName);
    QString strName = info.completeBaseName();     // 文件夹和文件获取的方式不一样

    if (info.filePath().contains(".tar.")) {
        qCDebug(appLog) << "Removing .tar from filename";
        strName = strName.remove(".tar"); // 类似tar.gz压缩文件，创建文件夹的时候移除.tar
    } else if (info.filePath().contains(".7z.")) {
        qCDebug(appLog) << "Removing .7z from filename";
        strName = strName.remove(".7z"); // 7z分卷文件，创建文件夹的时候移除.7z
    } else if (info.filePath().contains(".part01.rar")) {
        qCDebug(appLog) << "Removing .part01 from filename";
        strName = strName.remove(".part01"); // rar分卷文件，创建文件夹的时候移除part01
    } else if (info.filePath().contains(".part1.rar")) {
        qCDebug(appLog) << "Removing .part1 from filename";
        strName = strName.remove(".part1"); // rar分卷文件，创建文件夹的时候移除.part1
    } else if (info.filePath().contains(".zip.")) {
        qCDebug(appLog) << "Removing .zip from filename";
        strName = strName.remove(".zip"); // zip分卷文件，创建文件夹的时候移除.zip
    }

    qCDebug(appLog) << "Final handled filename:" << strName;
    return strName;
}

bool UiTools::isLocalDeviceFile(const QString &strFileName)
{
    qCDebug(appLog) << "Checking if file is on local device:" << strFileName;
    QStorageInfo info(strFileName);
    QString sDevice = info.device();
    QString sFileSystemType = info.fileSystemType();

    // 检查传统的本地设备（以 /dev/ 开头）
    if (sDevice.startsWith("/dev/")) {
        return true;
    }

    // 检查长文件名文件系统类型
    if (sFileSystemType == "fuse.dlnfs" || sFileSystemType == "ulnfs") {
        return true;
    }

    return false;
}

QStringList UiTools::removeSameFileName(const QStringList &listFiles)
{
    qCDebug(appLog) << "Removing duplicate filenames from list of" << listFiles.count() << "files";
    QStringList listResult;
    QStringList listName;

    for (int i = 0; i < listFiles.count(); ++i) {
        QFileInfo info(listFiles[i]);
        if (listName.contains(info.fileName())) {
            qCDebug(appLog) << "Skipping duplicate filename:" << info.fileName();
            continue;
        } else {
            listResult << listFiles[i];
            listName << info.fileName();
        }
    }

    qCDebug(appLog) << "Removed" << (listFiles.count() - listResult.count()) << "duplicates, result count:" << listResult.count();
    return listResult;
}

bool UiTools::isWayland()
{
    qCDebug(appLog) << "Checking if running on Wayland";
    auto e = QProcessEnvironment::systemEnvironment();
    QString XDG_SESSION_TYPE = e.value(QStringLiteral("XDG_SESSION_TYPE"));
    QString WAYLAND_DISPLAY = e.value(QStringLiteral("WAYLAND_DISPLAY"));
    qCDebug(appLog) << "XDG_SESSION_TYPE:" << XDG_SESSION_TYPE << "WAYLAND_DISPLAY:" << WAYLAND_DISPLAY;

    if (XDG_SESSION_TYPE == QLatin1String("wayland") || WAYLAND_DISPLAY.contains(QLatin1String("wayland"), Qt::CaseInsensitive)) {
        qCDebug(appLog) << "Running on Wayland";
        return true;
    } else {
        qCDebug(appLog) << "Not running on Wayland";
        return false;
    }
}
