/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
 * Maintainer: rekols <rekols@foxmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "utils.h"

#include <QUrl>
#include <QDir>
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

Utils::Utils(QObject *parent)
    : QObject(parent)
{
}

Utils::~Utils()
{
}



QString Utils::getConfigPath()
{
    QDir dir(QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first())
             .filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}


QString Utils::suffixList()
{
    return QString("");
}

QPixmap Utils::renderSVG(const QString &filePath, const QSize &size)
{
    QImageReader reader;
    QPixmap pixmap;

    reader.setFileName(filePath);

    if (reader.canRead()) {
        const qreal ratio = qApp->devicePixelRatio();
        reader.setScaledSize(size * ratio);
        pixmap = QPixmap::fromImage(reader.read());
        pixmap.setDevicePixelRatio(ratio);
    } else {
        pixmap.load(filePath);
    }

    return pixmap;
}

bool Utils::isCompressed_file(const QString &filePath)
{
    QMimeType mimetype = determineMimeType(filePath);
    qDebug()<< mimetype.name();
    QString filetype = mimetype.name();
    bool ret = true;


    if(filetype.compare("application/x-7z-compressed") && filetype.compare("application/zip") && filetype.compare("application/vnd.rar") && filetype.compare("application/x-rar")
            && filetype.compare("application/x-java-archive")&& filetype.compare("application/x-deb")&& filetype.compare("application/x-cd-image")
            && filetype.compare("application/x-bcpio")&& filetype.compare("application/x-cpio")&&filetype.compare("application/x-cpio-compressed")&& filetype.compare("application/x-sv4cpio")
            && filetype.compare("application/x-sv4crc")&&filetype.compare("application/x-rpm")&&filetype.compare("application/x-source-rpm")&&filetype.compare("application/vnd.debian.binary-package")
            && filetype.compare("application/vnd.ms-cab-compressed")&& filetype.compare("application/x-xar")&& filetype.compare("application/x-iso9660-appimage")
            && filetype.compare("application/x-tarz")&& filetype.compare("application/x-tar")&& filetype.compare("application/x-compressed-tar")&& filetype.compare("application/x-bzip-compressed-tar")
            && filetype.compare("application/x-xz-compressed-tar")&& filetype.compare("application/x-lzma-compressed-tar")&& filetype.compare("application/x-lzip-compressed-tar")
            && filetype.compare("application/x-tzo")&& filetype.compare("application/x-lrzip-compressed-tar")&& filetype.compare("application/x-lz4-compressed-tar")
            && filetype.compare("application/x-zstd-compressed-tar")&& filetype.compare("application/x-bzip")&& filetype.compare("application/gzip")&& filetype.compare("application/x-lzma")
            && filetype.compare("application/x-xz")&& filetype.compare("application/zip"))
    {
        ret = false;
    }
    qDebug()<<ret;
    return ret;
}

QString Utils::humanReadableSize(const qint64 &size, int precision)
{
    double sizeAsDouble = size;
    static QStringList measures;
    if (measures.isEmpty())
        measures << QCoreApplication::translate("QInstaller", "bytes")
                 << QCoreApplication::translate("QInstaller", "KiB")
                 << QCoreApplication::translate("QInstaller", "MiB")
                 << QCoreApplication::translate("QInstaller", "GiB")
                 << QCoreApplication::translate("QInstaller", "TiB")
                 << QCoreApplication::translate("QInstaller", "PiB")
                 << QCoreApplication::translate("QInstaller", "EiB")
                 << QCoreApplication::translate("QInstaller", "ZiB")
                 << QCoreApplication::translate("QInstaller", "YiB");
    QStringListIterator it(measures);
    QString measure(it.next());
    while (sizeAsDouble >= 1024.0 && it.hasNext()) {
        measure = it.next();
        sizeAsDouble /= 1024.0;
    }
    return QString::fromLatin1("%1 %2").arg(sizeAsDouble, 0, 'f', precision).arg(measure);
}
