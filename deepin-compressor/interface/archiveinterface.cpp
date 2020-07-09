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
#include "archiveinterface.h"
#include "mimetypes.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QTextCodec>

#include <sys/stat.h>

Q_DECLARE_METATYPE(KPluginMetaData)
ReadOnlyArchiveInterface::ReadOnlyArchiveInterface(QObject *parent, const QVariantList &args)
    : QObject(parent)
    , m_numberOfVolumes(0)
    , m_numberOfEntries(0)
    , m_waitForFinishedSignal(false)
    , m_isHeaderEncryptionEnabled(false)
    , m_isCorrupt(false)
    , m_isMultiVolume(false)
    , m_isWrongPassword(false)
{
    Q_ASSERT(args.size() >= 2);

    qDebug() << "Created read-only interface for" << args.first().toString();
    m_filename = args.first().toString();
    m_mimetype = determineMimeType(m_filename);
    connect(this, &ReadOnlyArchiveInterface::entry, this, &ReadOnlyArchiveInterface::onEntry);

    m_metaData = args.at(1).value<KPluginMetaData>();
}

ReadOnlyArchiveInterface::~ReadOnlyArchiveInterface()
{
}

void ReadOnlyArchiveInterface::onEntry(Archive::Entry *archiveEntry)
{
    Q_UNUSED(archiveEntry)
    m_numberOfEntries++;
}

QString ReadOnlyArchiveInterface::filename() const
{
    return m_filename;
}

void ReadOnlyArchiveInterface::setFileName(const QString &name)
{
    m_filename = name;
}

QString ReadOnlyArchiveInterface::comment() const
{
    return m_comment;
}

bool ReadOnlyArchiveInterface::isReadOnly() const
{
    return true;
}

bool ReadOnlyArchiveInterface::open()
{
    return true;
}

void ReadOnlyArchiveInterface::setPassword(const QString &password)
{
    m_password = password;
}

void ReadOnlyArchiveInterface::setHeaderEncryptionEnabled(bool enabled)
{
    m_isHeaderEncryptionEnabled = enabled;
}

QString ReadOnlyArchiveInterface::password() const
{
    return m_password;
}

bool ReadOnlyArchiveInterface::doKill()
{
    //default implementation
    return false;
}

void ReadOnlyArchiveInterface::setCorrupt(bool isCorrupt)
{
    m_isCorrupt = isCorrupt;
}

bool ReadOnlyArchiveInterface::isCorrupt() const
{
    return m_isCorrupt;
}

void ReadOnlyArchiveInterface::setWrongPassword(bool isWrong)
{
    m_isWrongPassword = isWrong;
}

bool ReadOnlyArchiveInterface::isWrongPassword() const
{
    return m_isWrongPassword;
}

bool ReadOnlyArchiveInterface::isMultiVolume() const
{
    return m_isMultiVolume;
}

void ReadOnlyArchiveInterface::setMultiVolume(bool value)
{
    m_isMultiVolume = value;
}

int ReadOnlyArchiveInterface::numberOfVolumes() const
{
    return m_numberOfVolumes;
}

QString ReadOnlyArchiveInterface::multiVolumeName() const
{
    return filename();
}

ReadWriteArchiveInterface::ReadWriteArchiveInterface(QObject *parent, const QVariantList &args)
    : ReadOnlyArchiveInterface(parent, args)
{

    connect(this, &ReadWriteArchiveInterface::entryRemoved, this, &ReadWriteArchiveInterface::onEntryRemoved);
}

ReadWriteArchiveInterface::~ReadWriteArchiveInterface()
{

}

bool ReadOnlyArchiveInterface::waitForFinishedSignal()
{
    return m_waitForFinishedSignal;
}

int ReadOnlyArchiveInterface::moveRequiredSignals() const
{
    return 1;
}

int ReadOnlyArchiveInterface::copyRequiredSignals() const
{
    return 1;
}

void ReadOnlyArchiveInterface::setWaitForFinishedSignal(bool value)
{
    m_waitForFinishedSignal = value;
}

QStringList ReadOnlyArchiveInterface::entryFullPaths(const QVector<Archive::Entry *> &entries, PathFormat format)
{
    QStringList filesList;
    for (const Archive::Entry *file : entries) {
        filesList << file->fullPath(format);
    }

    return filesList;
}

QVector<Archive::Entry *> ReadOnlyArchiveInterface::entriesWithoutChildren(const QVector<Archive::Entry *> &entries)
{
    // QMap is easy way to get entries sorted by their fullPath.
    QMap<QString, Archive::Entry *> sortedEntries;
    for (Archive::Entry *entry : entries) {
        sortedEntries.insert(entry->fullPath(), entry);
    }

    QVector<Archive::Entry *> filteredEntries;
    QString lastFolder;
    for (Archive::Entry *entry : qAsConst(sortedEntries)) {
        if (lastFolder.count() > 0 && entry->fullPath().startsWith(lastFolder)) {
            continue;
        }

        lastFolder = (entry->fullPath().right(1) == QLatin1String("/")) ? entry->fullPath() : QString();
        filteredEntries << entry;
    }

    return filteredEntries;
}

QStringList ReadOnlyArchiveInterface::entryPathsFromDestination(QStringList entries, const Archive::Entry *destination, int entriesWithoutChildren)
{
    QStringList paths = QStringList();
    entries.sort();
    QString lastFolder;
    const QString destinationPath = (destination == nullptr) ? QString() : destination->fullPath();

    QString newPath;
    int nameLength = 0;
    for (const QString &entryPath : qAsConst(entries)) {
        if (lastFolder.count() > 0 && entryPath.startsWith(lastFolder)) {
            // Replace last moved or copied folder path with destination path.
            int charsCount = entryPath.count() - lastFolder.count();
            if (entriesWithoutChildren != 1) {
                charsCount += nameLength;
            }
            newPath = destinationPath + entryPath.right(charsCount);
        } else {
            const QString name = entryPath.split(QLatin1Char('/'), QString::SkipEmptyParts).last();
            if (entriesWithoutChildren != 1) {
                newPath = destinationPath + name;
                if (entryPath.right(1) == QLatin1String("/")) {
                    newPath += QLatin1Char('/');
                }
            } else {
                // If the mode is set to Move and there is only one passed file in the list,
                // we have to use destination as newPath.
                newPath = destinationPath;
            }
            if (entryPath.right(1) == QLatin1String("/")) {
                nameLength = name.count() + 1; // plus slash
                lastFolder = entryPath;
            } else {
                nameLength = 0;
                lastFolder = QString();
            }
        }

        paths << newPath;
    }

    return paths;
}

QFileDevice::Permissions ReadOnlyArchiveInterface::getPermissions(const mode_t &perm)
{
    QFileDevice::Permissions pers = QFileDevice::Permissions();

    if (perm == 0) {
        pers |= (QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ReadGroup | QFileDevice::ReadOther);
        return pers;
    }

    if (perm & S_IRUSR) {
        pers |= QFileDevice::ReadUser;
    }

    if (perm & S_IWUSR) {
        pers |= QFileDevice::WriteUser;
    }

    if (perm & S_IXUSR) {
        pers |= QFileDevice::ExeUser;
    }

    if (perm & S_IRGRP) {
        pers |= QFileDevice::ReadGroup;
    }
    if (perm & S_IWGRP) {
        pers |= QFileDevice::WriteGroup;
    }

    if (perm & S_IXGRP) {
        pers |= QFileDevice::ExeGroup;
    }

    if (perm & S_IROTH) {
        pers |= QFileDevice::ReadOther;
    }

    if (perm & S_IWOTH) {
        pers |= QFileDevice::WriteOther;
    }

    if (perm & S_IXOTH) {
        pers |= QFileDevice::ExeOther;
    }

    return pers;
}

bool ReadOnlyArchiveInterface::isHeaderEncryptionEnabled() const
{
    return m_isHeaderEncryptionEnabled;
}

QMimeType ReadOnlyArchiveInterface::mimetype() const
{
    return m_mimetype;
}

bool ReadOnlyArchiveInterface::hasBatchExtractionProgress() const
{
    return false;
}

bool ReadOnlyArchiveInterface::isLocked() const
{
    return false;
}

bool ReadOnlyArchiveInterface::isUserCancel() const
{
    return userCancel;
}

bool ReadOnlyArchiveInterface::isCheckPsw() const
{
    return m_isckeckpsd;
}

bool ReadOnlyArchiveInterface::isAnyFileExtracted() const
{
    return bAnyFileExtracted;
}

bool ReadWriteArchiveInterface::isReadOnly() const
{
    if (isLocked()) {
        return true;
    }

    // We set corrupt archives to read-only to avoid add/delete actions, that
    // are likely to fail anyway.
    if (isCorrupt()) {
        return true;
    }

    QFileInfo fileInfo(filename());
    if (fileInfo.exists()) {
        return !fileInfo.isWritable();
    } else {
        return !fileInfo.dir().exists(); // TODO: Should also check if we can create a file in that directory
    }
}

uint ReadOnlyArchiveInterface::numberOfEntries() const
{
    return m_numberOfEntries;
}

void ReadWriteArchiveInterface::onEntryRemoved(const QString &path)
{
    Q_UNUSED(path)
    m_numberOfEntries--;
}

void ReadWriteArchiveInterface::clearPath(QString path)
{
    QProcess p;
    QString command = "rm";
    QStringList args;
    args.append("-fr");
    args.append(path);
    p.execute(command, args);
    p.waitForFinished();
}

void ReadWriteArchiveInterface::watchFileList(QStringList */*strList*/)
{

}
