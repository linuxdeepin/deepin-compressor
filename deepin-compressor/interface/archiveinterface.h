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
#ifndef ARCHIVEINTERFACE_H
#define ARCHIVEINTERFACE_H

#include "archive_manager.h"

#include "archiveentry.h"

#include <QObject>
#include <QStringList>
#include <QString>
#include <QVariantList>
#include "kpluginmetadata.h"

class Query;

class  ReadOnlyArchiveInterface: public QObject
{
    Q_OBJECT
public:
    explicit ReadOnlyArchiveInterface(QObject *parent, const QVariantList &args);
    ~ReadOnlyArchiveInterface() override;

    QString filename() const;
    QString comment() const;
    QString password() const;

    bool isMultiVolume() const;
    int numberOfVolumes() const;
    virtual bool isReadOnly() const;
    virtual bool open();
    virtual bool list(bool isbatch = false) = 0;
    virtual bool testArchive() = 0;
    void setPassword(const QString &password);
    void setHeaderEncryptionEnabled(bool enabled);
    virtual bool extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options) = 0;
    bool waitForFinishedSignal();
    virtual int moveRequiredSignals() const;
    virtual int copyRequiredSignals() const;
    static QStringList entryFullPaths(const QVector<Archive::Entry *> &entries, PathFormat format = WithTrailingSlash);
    static QVector<Archive::Entry *> entriesWithoutChildren(const QVector<Archive::Entry *> &entries);
    static QStringList entryPathsFromDestination(QStringList entries, const Archive::Entry *destination, int entriesWithoutChildren);

    virtual bool doKill();

    bool isHeaderEncryptionEnabled() const;
    virtual QString multiVolumeName() const;
    void setMultiVolume(bool value);
    uint numberOfEntries() const;
    QMimeType mimetype() const;

    virtual bool hasBatchExtractionProgress() const;

    virtual bool isLocked() const;

Q_SIGNALS:

    void cancelled();
    void error(const QString &message, const QString &details = QString());
    void entry(Archive::Entry *archiveEntry);
    void progress(double progress);
    void progress_filename(const QString &filename);
    void info(const QString &info);
    void finished(bool result);
    void testSuccess();
    void compressionMethodFound(const QString &method);
    void encryptionMethodFound(const QString &method);
    void sigExtractNeedPassword();
    void userQuery(Query *query);
    void updateDestFileSignal(QString destFile);

protected:
    void setWaitForFinishedSignal(bool value);

    void setCorrupt(bool isCorrupt);
    bool isCorrupt() const;
    void setWrongPassword(bool isWrong);
    bool isWrongPassword() const;
    QString m_comment;
    int m_numberOfVolumes;
    uint m_numberOfEntries;
    KPluginMetaData m_metaData;

private:
    QString m_filename;
    QMimeType m_mimetype;
    QString m_password;
    bool m_waitForFinishedSignal;
    bool m_isHeaderEncryptionEnabled;
    bool m_isCorrupt;
    bool m_isMultiVolume;
    bool m_isWrongPassword;

private Q_SLOTS:
    void onEntry(Archive::Entry *archiveEntry);

};

class  ReadWriteArchiveInterface: public ReadOnlyArchiveInterface
{
    Q_OBJECT
public:
    enum OperationMode  {
        NoOperation,
        List,
        Extract,
        Add,
        Move,
        Copy,
        Delete,
        Comment,
        Test
    };

    explicit ReadWriteArchiveInterface(QObject *parent, const QVariantList &args);
    ~ReadWriteArchiveInterface() override;

    bool isReadOnly() const override;
    virtual bool addFiles(const QVector<Archive::Entry *> &files, const Archive::Entry *destination, const CompressionOptions &options, uint numberOfEntriesToAdd = 0) = 0;
    virtual bool moveFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options) = 0;
    virtual bool copyFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options) = 0;
    virtual bool deleteFiles(const QVector<Archive::Entry *> &files) = 0;
    virtual bool addComment(const QString &comment) = 0;

Q_SIGNALS:
    void entryRemoved(const QString &path);

protected:
    OperationMode m_operationMode = NoOperation;

private Q_SLOTS:
    void onEntryRemoved(const QString &path);


};



#endif // ARCHIVEINTERFACE_H
