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

#ifndef UITOOLS_H
#define UITOOLS_H

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

// 界面通用工具
class UiTools : public QObject
{
    Q_OBJECT

public:
    UiTools(QObject *parent = nullptr);
    ~UiTools();

    /**
     * @brief getConfigPath 获取配置路径
     * @return
     */
    static QString getConfigPath();

    /**
     * @brief renderSVG     渲染图标
     * @param filePath      图标路径
     * @param size          图标大小
     * @return              新的图标
     */
    static QPixmap renderSVG(const QString &filePath, const QSize &size);

    /**
     * @brief humanReadableSize 分卷计算大小
     * @param size
     * @param precision
     * @return
     */
    static QString humanReadableSize(const qint64 &size, int precision);

private:
    static QStringList m_associtionlist;
};

#endif  // UITOOLS_H
