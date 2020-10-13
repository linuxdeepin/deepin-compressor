/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
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

#include "uitools.h"
//#include "mimetypes.h"

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

#include <KEncodingProber>

DCORE_USE_NAMESPACE

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
                                        << "file_association.file_association_type.x-cd-image"
                                        << "file_association.file_association_type.x-iso9660-appimage"
                                        << "file_association.file_association_type.x-source-rpm";

UiTools::UiTools(QObject *parent)
    : QObject(parent)
{
}

UiTools::~UiTools()
{
}

QString UiTools::getConfigPath()
{
    QDir dir(QDir(QStandardPaths::standardLocations(QStandardPaths::ConfigLocation).first())
             .filePath(qApp->organizationName()));

    return dir.filePath(qApp->applicationName());
}

QPixmap UiTools::renderSVG(const QString &filePath, const QSize &size)
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

QString UiTools::humanReadableSize(const qint64 &size, int precision)
{
    if (size == 0) {
        return "-";
    }

    double sizeAsDouble = size;
    static QStringList measures;
    if (measures.isEmpty())
        measures << QCoreApplication::translate("QInstaller", "B")
                 << QCoreApplication::translate("QInstaller", "KB")
                 << QCoreApplication::translate("QInstaller", "MB")
                 << QCoreApplication::translate("QInstaller", "GB")
                 << QCoreApplication::translate("QInstaller", "TB")
                 << QCoreApplication::translate("QInstaller", "PB")
                 << QCoreApplication::translate("QInstaller", "EB")
                 << QCoreApplication::translate("QInstaller", "ZB")
                 << QCoreApplication::translate("QInstaller", "YB");
    QStringListIterator it(measures);
    QString measure(it.next());
    while (sizeAsDouble >= 1024.0 && it.hasNext()) {
        measure = it.next();
        sizeAsDouble /= 1024.0;
    }

    return QString::fromLatin1("%1 %2").arg(sizeAsDouble, 0, 'f', precision).arg(measure);
}
