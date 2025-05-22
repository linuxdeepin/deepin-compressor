// Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "openwithdialog/dmimedatabase.h"
//#include "shutil/fileutils.h"

#include <QFileInfo>
#include <QUrl>
#include <QDebug>

DMimeDatabase::DMimeDatabase()
{
    qDebug() << "DMimeDatabase constructor";
}

QMimeType DMimeDatabase::mimeTypeForFile(const QString &fileName, QMimeDatabase::MatchMode mode) const
{
    qDebug() << "Getting MIME type for file:" << fileName;
    return mimeTypeForFile(QFileInfo(fileName), mode);
}

QMimeType DMimeDatabase::mimeTypeForFile(const QFileInfo &fileInfo, QMimeDatabase::MatchMode mode) const
{
    qDebug() << "Getting MIME type for file info:" << fileInfo.fileName() << "with mode:" << mode;
    QMimeType result = QMimeDatabase::mimeTypeForFile(fileInfo, mode);
    qDebug() << "Initial MIME type:" << result.name();

    // temporary dirty fix, once WPS get installed, the whole mimetype database thing get fscked up.
    // we used to patch our Qt to fix this issue but the patch no longer works, we don't have time to
    // look into this issue ATM.
    // https://bugreports.qt.io/browse/QTBUG-71640
    // https://codereview.qt-project.org/c/qt/qtbase/+/244887
    // `file` command works but libmagic didn't even comes with any pkg-config support..
    static QStringList officeSuffixList {
        "docx", "xlsx", "pptx", "doc", "ppt", "xls"
    };

    static QStringList wrongMimeTypeNames {
        "application/x-ole-storage", "application/zip"
    };

    if (officeSuffixList.contains(fileInfo.suffix()) && wrongMimeTypeNames.contains(result.name())) {
        QList<QMimeType> results = QMimeDatabase::mimeTypesForFileName(fileInfo.fileName());
        if (!results.isEmpty()) {
            qInfo() << "Corrected MIME type from" << result.name() << "to" << results.first().name();
            return results.first();
        } else {
            qWarning() << "No valid MIME types found for office file";
        }
    }

    qDebug() << "Final MIME type:" << result.name() << "for file:" << fileInfo.fileName();
    return result;
}

//QMimeType DMimeDatabase::mimeTypeForUrl(const QUrl &url) const
//{
//    if (url.isLocalFile())
//        return mimeTypeForFile(url.toLocalFile());

//    return QMimeDatabase::mimeTypeForUrl(url);
//}
