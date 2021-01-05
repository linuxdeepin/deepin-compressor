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

#ifndef MIMETYPEDISPLAYMANAGER_H
#define MIMETYPEDISPLAYMANAGER_H

#include <QObject>
#include <QMap>


enum FileType {
    // base type
    Directory,
    CharDevice,
    BlockDevice,
    FIFOFile,
    SocketFile,
    RegularFile,
    //
    Executable,
    Documents,
    Images,
    Videos,
    Audios,
    Archives,
    DesktopApplication,
    Backups,
    Unknown,
    CustomType = 0x100
};

#define MIMETYPE_PATH "/usr/share/dde-file-manager/mimetypes"

class MimeTypeDisplayManager : public QObject
{
    Q_OBJECT
public:

    explicit MimeTypeDisplayManager(QObject *parent = nullptr);

    void initData();
    void initConnect();

    QString displayName(const QString &mimeType);
    FileType displayNameToEnum(const QString &mimeType);
    QString defaultIcon(const QString &mimeType);

    QMap<FileType, QString> displayNames();
    static QStringList readlines(const QString &path);
    static void loadSupportMimeTypes();
    static QStringList supportArchiveMimetypes();
    static QStringList supportVideoMimeTypes();

private:
    QMap<FileType, QString> m_displayNames;
    QMap<FileType, QString> m_defaultIconNames;
    static QStringList ArchiveMimeTypes;
    static QStringList AvfsBlackList;
    static QStringList TextMimeTypes;
    static QStringList VideoMimeTypes;
    static QStringList AudioMimeTypes;
    static QStringList ImageMimeTypes;
    static QStringList ExecutableMimeTypes;
    static QStringList BackupMimeTypes;

    bool m_supportLoaded = false;
};

#endif // MIMETYPEDISPLAYMANAGER_H
