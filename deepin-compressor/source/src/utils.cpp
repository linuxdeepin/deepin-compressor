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

bool Utils::isFontMimeType(const QString &filePath)
{
    const QString mimeName = QMimeDatabase().mimeTypeForFile(filePath).name();;

    if (mimeName.startsWith("font/") ||
        mimeName.startsWith("application/x-font")) {
        return true;
    }

    return false;
}

QString Utils::suffixList()
{
    return QString("Font Files (*.ttf *.ttc *.otf)");
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
    QString file_suffix ;
    QFileInfo fileinfo;
    bool ret = true;
    fileinfo = QFileInfo(filePath);
    file_suffix = fileinfo.suffix();
    qDebug()<<file_suffix;
    if(file_suffix.compare("7z") && file_suffix.compare("ar") && file_suffix.compare("cbz") && file_suffix.compare("cpio")
            && file_suffix.compare("exe")&& file_suffix.compare("iso")&& file_suffix.compare("tar")
            && file_suffix.compare("Z")&& file_suffix.compare("bz2")&&file_suffix.compare("gz")&& file_suffix.compare("lz")
            && file_suffix.compare("lzma")&&file_suffix.compare("lzo")&&file_suffix.compare("xz")&&file_suffix.compare("zip")
            && file_suffix.compare("rar"))
    {
        ret = false;
    }
    qDebug()<<ret;
    return ret;
}
