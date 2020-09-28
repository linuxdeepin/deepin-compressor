#ifndef LIBARCHIVEPLUGIN_H
#define LIBARCHIVEPLUGIN_H

#include "archiveinterface.h"

#include <QScopedPointer>
#include <QProcess>

#include <archive.h>

class Common;

struct FileProgressInfo {
    float fileProgressProportion = 0.0; //内部百分值范围
    float fileProgressStart;            //上次的百分值
    float totalFileSize;
};

class LibarchivePlugin : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit LibarchivePlugin(QObject *parent, const QVariantList &args);
    ~LibarchivePlugin() override;

    bool list(bool isbatch = false) override;
    bool doKill() override;
    bool extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options) override;

    bool addFiles(const QVector<Archive::Entry *> &files, const Archive::Entry *destination, const CompressionOptions &options, uint numberOfEntriesToAdd = 0) override;
    bool moveFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options) override;
    bool copyFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options) override;
    bool deleteFiles(const QVector<Archive::Entry *> &files) override;
    bool addComment(const QString &comment) override;
    bool testArchive() override;
    bool hasBatchExtractionProgress() const override;
    virtual void cleanIfCanceled()override;
    virtual void watchFileList(QStringList *strList)override;

    virtual void showEntryListFirstLevel(const QString &directory) override;
    virtual void RefreshEntryFileCount(Archive::Entry *file) override;

protected:
    struct ArchiveReadCustomDeleter {
        static inline void cleanup(struct archive *a)
        {
            if (a) {
                archive_read_free(a);
            }
        }
    };

    struct ArchiveWriteCustomDeleter {
        static inline void cleanup(struct archive *a)
        {
            if (a) {
                archive_write_free(a);
            }
        }
    };

    typedef QScopedPointer<struct archive, ArchiveReadCustomDeleter> ArchiveRead;
    typedef QScopedPointer<struct archive, ArchiveWriteCustomDeleter> ArchiveWrite;

    bool initializeReader();
    void createEntry(const QString &externalPath, struct archive_entry *entry);
    void setEntryData(/*const */archive_stat &aentry, qlonglong index, const QString &name, bool isMutilFolderFile = false);
    Archive::Entry *setEntryDataA(/*const */archive_stat &aentry/*, qlonglong index*/, const QString &name);
    void setEntryVal(/*const */archive_stat &aentry, int &index, const QString &name, QString &dirRecord);
    virtual void updateListMap(QVector<Archive::Entry *> &files, int type) override;
    void updateListMap(Archive::Entry *entry, int type);

    void emitEntryForIndex(archive_entry *aentry, qlonglong index);
    virtual qint64 extractSize(const QVector<Archive::Entry *> &files) override;
    void emitEntryFromArchiveEntry(struct archive_entry *entry);
    void copyData(const QString &filename, struct archive *dest, const FileProgressInfo &info, bool bInternalDuty = true);
    void copyDataFromSource(const QString &filename, struct archive *source, struct archive *dest, bool bInternalDuty = true);
    void copyDataFromSource_ArchiveEntry(Archive::Entry *pSourceEntry, struct archive *source, struct archive *dest, bool bInternalDuty = true);
    void copyDataFromSourceAdd(const QString &filename, struct archive *source, struct archive *dest, struct archive_entry *sourceEntry, FileProgressInfo &info, bool bInternalDuty = true);
    ArchiveRead m_archiveReader;
    ArchiveRead m_archiveReadDisk;

private Q_SLOTS:
    void slotRestoreWorkingDir();

private:
    int extractionFlags() const;
    QString convertCompressionName(const QString &method);
    bool list_New(bool isbatch = false);
    void deleteTempTarPkg(const QStringList &tars);
    qlonglong calDecompressSize();

    int m_cachedArchiveEntryCount;
    qlonglong m_currentExtractedFilesSize = 0;//当前已经解压出来的文件大小（能展示出来的都已经解压）
    bool m_emitNoEntries;
    qlonglong m_extractedFilesSize;
    QMap<QString, /*QPair<*/archive_stat/*, qlonglong>*/> m_listMap;
    archive_stat m_archiveEntryStat;
    QString m_DirRecord;
    QString m_SigDirRecord;
    int m_indexCount = 0;

    //QVector<Archive::Entry *> m_emittedEntries;
    QString m_oldWorkingDir;
    QString m_extractDestDir;
    QStringList m_tars;

    QString strOldFileName;
    int m_listIndex = 0;

    QString m_strRootNode;
    QStringList m_listFileName;
    Common *m_common = nullptr;
};

#endif // LIBARCHIVEPLUGIN_H
