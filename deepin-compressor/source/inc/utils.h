/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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

#ifndef UTILS_H
#define UTILS_H

#include <QObject>


#define SAFE_DELETE_ELE( ptr ) \
    if (ptr != NULL)      \
    {                     \
        delete ptr;       \
        ptr = NULL;       \
    }

#define SAFE_DELETE_ARRAY( ptr ) \
    if (ptr != NULL)            \
    {                           \
        delete[] ptr;           \
        ptr = NULL;             \
    }

#define SAFE_DELETE_TABLE( ptr ) \
    if (ptr != NULL)            \
    {                           \
        for (i = 0; i < row; i++)\
        {\
            SAFE_DELETE_ARRAY(ptr[i])\
            delete [] ptr;\
        }\
        ptr = NULL; \
    }


class Utils : public QObject
{
    Q_OBJECT

public:
    Utils(QObject *parent = nullptr);
    ~Utils();

    static QString getConfigPath();
    static QString suffixList();
    static QPixmap renderSVG(const QString &filePath, const QSize &size);
    static bool isCompressed_file(const QString &filePath);
    static QString humanReadableSize(const qint64 &size, int precision);
    static qint64 humanReadableToSize(const QString &size);
    static QByteArray detectEncode(const QByteArray &data, const QString &fileName = QString());
    static QString toShortString(QString strSrc, int limitCounts = 16, int left = 8);
    static bool checkAndDeleteDir(const QString &iFilePath);
    static bool deleteDir(const QString &iFilePath);
    static quint64 getAllFileCount(const QString &path);
    static QString readConf();

    /**
     * @brief existMimeType 判断此文件关联类型是否存在
     * @param mimetype      文件类型
     * @return
     */
    static bool existMimeType(QString mimetype);
    static QString judgeFileMime(QString file);

private:
    static QStringList m_associtionlist;
};

#endif
