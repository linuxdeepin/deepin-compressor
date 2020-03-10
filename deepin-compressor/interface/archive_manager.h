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
#ifndef ARCHIVE_H
#define ARCHIVE_H

#include "options.h"

#include "archivejob.h"

#include <QHash>
#include <QMimeType>


class LoadJob;
class BatchExtractJob;
class CreateJob;
class ExtractJob;
class DeleteJob;
class AddJob;
class MoveJob;
class CopyJob;
class CommentJob;
class TestJob;
class OpenJob;
class OpenWithJob;
class Plugin;
class PreviewJob;
class Query;
class ReadOnlyArchiveInterface;

enum ArchiveError {
    NoError = 0,
    NoPlugin,
    FailedPlugin
};

class Archive : public QObject
{
    Q_OBJECT

    /**
     *  Complete base name, without the "tar" extension (if any).
     */
    Q_PROPERTY(QString completeBaseName READ completeBaseName CONSTANT)
    Q_PROPERTY(QString fileName READ fileName CONSTANT)
    Q_PROPERTY(QString comment READ comment CONSTANT)
    Q_PROPERTY(QMimeType mimeType READ mimeType CONSTANT)
    Q_PROPERTY(bool isEmpty READ isEmpty)
    Q_PROPERTY(bool isReadOnly READ isReadOnly CONSTANT)
    Q_PROPERTY(bool isSingleFile READ isSingleFile)
    Q_PROPERTY(bool isSingleFolder MEMBER m_isSingleFolder READ isSingleFolder)
    Q_PROPERTY(bool isMultiVolume READ isMultiVolume WRITE setMultiVolume)
    Q_PROPERTY(bool numberOfVolumes READ numberOfVolumes)
    Q_PROPERTY(EncryptionType encryptionType MEMBER m_encryptionType READ encryptionType)
    Q_PROPERTY(uint numberOfEntries READ numberOfEntries)
    Q_PROPERTY(qulonglong unpackedSize MEMBER m_extractedFilesSize READ unpackedSize)
    Q_PROPERTY(qulonglong packedSize READ packedSize)
    Q_PROPERTY(QString subfolderName MEMBER m_subfolderName READ subfolderName)
    Q_PROPERTY(QString password READ password)
    Q_PROPERTY(QStringList compressionMethods MEMBER m_compressionMethods)
    Q_PROPERTY(QStringList encryptionMethods MEMBER m_encryptionMethods)

public:

    enum EncryptionType {
        Unencrypted,
        Encrypted,
        HeaderEncrypted
    };
    Q_ENUM(EncryptionType)

    class Entry;

    QString completeBaseName() const;
    QString fileName() const;
    QString comment() const;
    QMimeType mimeType();
    bool isEmpty() const;
    bool isReadOnly() const;
    bool isSingleFile() const;
    bool isSingleFolder() const;
    bool isMultiVolume() const;
    void setMultiVolume(bool value);
    bool hasComment() const;
    int numberOfVolumes() const;
    EncryptionType encryptionType() const;
    QString password() const;
    uint numberOfEntries() const;
    qulonglong unpackedSize() const;
    qulonglong packedSize() const;
    QString subfolderName() const;
    QString multiVolumeName() const;
    ReadOnlyArchiveInterface *interface();


    bool hasMultipleTopLevelEntries() const;
    static BatchExtractJob *batchExtract(const QString &fileName, const QString &destination, bool autoSubfolder, bool preservePaths, QObject *parent = nullptr);
    static CreateJob *create(const QString &fileName, const QString &mimeType, const QVector<Archive::Entry *> &entries, const CompressionOptions &options, QObject *parent = nullptr, bool useLibArchive = false);
    static Archive *createEmpty(const QString &fileName, const QString &mimeType, QObject *parent = nullptr);
    static LoadJob *load(const QString &fileName, QObject *parent = nullptr);
    static LoadJob *load(const QString &fileName, const QString &mimeType, QObject *parent = nullptr);
    static LoadJob *load(const QString &fileName, Plugin *plugin, QObject *parent = nullptr);
    static LoadJob *load(const QString &fileName, bool isbatch = false, QObject *parent = nullptr);

    ~Archive() override;

    ArchiveError error() const;
    bool isValid() const;

    DeleteJob *deleteFiles(QVector<Archive::Entry *> &entries);
    CommentJob *addComment(const QString &comment);
    TestJob *testArchive();

    AddJob *addFiles(const QVector<Archive::Entry *> &files, const Archive::Entry *destination, const CompressionOptions &options = CompressionOptions());
    MoveJob *moveFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options = CompressionOptions());
    CopyJob *copyFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options = CompressionOptions());
    ExtractJob *extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDir, const ExtractionOptions &options = ExtractionOptions());
    PreviewJob *preview(Archive::Entry *entry);
    OpenJob *open(Archive::Entry *entry);
    OpenWithJob *openWith(Archive::Entry *entry);
    void encrypt(const QString &password, bool encryptHeader);

private Q_SLOTS:
    void onAddFinished(KJob *);
    void onUserQuery(Query *);
    void onCompressionMethodFound(const QString &method);
    void onEncryptionMethodFound(const QString &method);

private:
    Archive(ReadOnlyArchiveInterface *archiveInterface, bool isReadOnly, QObject *parent = nullptr);
    Archive(ArchiveError errorCode, QObject *parent = nullptr);

    static Archive *create(const QString &fileName, QObject *parent = nullptr);
    static Archive *create(const QString &fileName, const QString &fixedMimeType, QObject *parent = nullptr);
    static Archive *create(const QString &fileName, Plugin *plugin, QObject *parent = nullptr);
    static Archive *create(const QString &fileName, const QString &fixedMimeType, bool write, QObject *parent= nullptr , bool useLibArchive = false);
    ReadOnlyArchiveInterface *m_iface;
    bool m_isReadOnly;
    bool m_isSingleFolder;
    bool m_isMultiVolume;

    QString m_subfolderName;
    qulonglong m_extractedFilesSize;
    ArchiveError m_error;
    EncryptionType m_encryptionType;
    QMimeType m_mimeType;
    QStringList m_compressionMethods;
    QStringList m_encryptionMethods;
};




#endif // ARCHIVE_H
