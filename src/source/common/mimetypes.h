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
#ifndef MIMETYPES_H
#define MIMETYPES_H

#include <QMimeType>

/**
 * @param filename Absolute path of a file.
 * @return The mimetype of the given file.
 */

/* 对一些未识别出来的类型进行区分
*  zip空压缩包：内容检测为"application/octet-stream"，后缀检测为"application/zip"，file命令探测为"application/zip"
*  谷歌插件zip：内容检测为"application/octet-stream"，后缀检测为"application/zip"，file命令探测为"application/x-chrome-extension"
*  谷歌插件crx：内容检测为"application/octet-stream"，后缀检测为"application/octet-stream"，file命令探测为"application/x-chrome-extension"
*  zip分卷包：内容检测为"application/octet-stream"，后缀检测为"application/zip"，file命令探测为"application/octet-stream"
*/
class CustomMimeType
{
public:
    CustomMimeType() {}
    ~CustomMimeType() {}

    /**
     * @brief name  获取类型名
     * @return
     */
    QString name() const
    {
        if (m_bUnKnown) {
            return m_strTypeName;
        }

        return m_mimeType.name();
    }

    /**
     * @brief inherits      是否继承某个类型
     * @param strMimeType   父类型
     * @return
     */
    bool inherits(const QString &strMimeType) const
    {
        if (m_bUnKnown) {
            if (strMimeType == m_strTypeName)
                return true;

            return false;
        }

        return m_mimeType.inherits(strMimeType);
    }

public:
    bool m_bUnKnown = false;  // 是否自定义扩展类型（内容检测为"application/octet-stream"时使用strTypeName，其余情况使用mimeType）
    QMimeType m_mimeType;
    QString m_strTypeName;
};

CustomMimeType determineMimeType(const QString &filename);

#endif // MIMETYPES_H
