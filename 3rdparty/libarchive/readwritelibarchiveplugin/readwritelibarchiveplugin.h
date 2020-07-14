#ifndef READWRITELIBARCHIVEPLUGIN_H
#define READWRITELIBARCHIVEPLUGIN_H

#include "../libarchiveplugin.h"
#include "kpluginfactory.h"

#include <QStringList>
#include <QSaveFile>
#include <QTemporaryDir>


class ReadWriteLibarchivePluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libarchive.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit ReadWriteLibarchivePluginFactory();
    ~ReadWriteLibarchivePluginFactory();
};


class ReadWriteLibarchivePlugin : public LibarchivePlugin
{
    Q_OBJECT

public:
    explicit ReadWriteLibarchivePlugin(QObject *parent, const QVariantList &args);
    ~ReadWriteLibarchivePlugin() override;

    bool addFiles(const QVector<Archive::Entry *> &files, const Archive::Entry *destination, const CompressionOptions &options, uint numberOfEntriesToAdd = 0) override;
    bool moveFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options) override;
    bool copyFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options) override;
    bool deleteFiles(const QVector<Archive::Entry *> &files) override;

protected:
    bool initializeWriter(const bool creatingNewFile = false, const CompressionOptions &options = CompressionOptions());
    bool initializeWriterFilters();
    bool initializeNewFileWriterFilters(const CompressionOptions &options);
    void finish(const bool isSuccessful);

private:
    /**
     * Processes all the existing entries and does manipulations to them
     * based on the OperationMode (Add/Move/Copy/Delete).
     *
     * @param entriesCounter Counter of added/moved/copied/deleted entries.
     *
     * @return bool indicating whether the operation was successful.
     */
    bool processOldEntries(uint &entriesCounter, OperationMode mode, uint totalCount);
    bool processOldEntries_Add(uint &entriesCounter, OperationMode mode, uint totalCount);
    /**
     * @brief deleteEntry
     * @param entriesCounter
     * @param mode
     * @param totalCount
     * @see 删除Entry
     */
    bool deleteEntry(uint &entriesCounter, uint totalCount);
    /**
     * Writes entry being read into memory.
     *
     * @return bool indicating whether the operation was successful.
     */
    bool writeEntry(struct archive_entry *entry);
    bool writeEntry_Add(struct archive_entry *entry, FileProgressInfo &info, bool bInternalDuty);

    /**
     * Writes entry from physical disk.
     *
     * @return bool indicating whether the operation was successful.
     */
    bool writeFile(const QString &relativeName, const QString &destination,  const FileProgressInfo &info, bool partialprogress = false);
    bool writeFileTodestination(const QString &sourceFileFullPath, const QString &destination, const QString &externalPath, const FileProgressInfo &info, bool partialprogress = false);
    bool writeFileFromEntry(const QString &relativeName, const QString destination, Archive::Entry *pEntry, const FileProgressInfo &info, bool bInternalDuty = false);
    QSaveFile m_tempFile;
    ArchiveWrite m_archiveWriter;

    // New added files by addFiles methods. It's assigned to m_filesPaths
    // and then is used by processOldEntries method (in Add mode) for skipping already written entries.
    QStringList m_writtenFiles;

    // Passed argument from job which is used by processOldEntries method，删除的时候用deleteEntry方法.
    QStringList m_filesPaths;
    int m_entriesWithoutChildren = 0;
    const Archive::Entry *m_destination = nullptr;
    QScopedPointer<QTemporaryDir> m_extractTempDir; //added by hsw 20200528
};

#endif // READWRITELIBARCHIVEPLUGIN_H
