#ifndef LIBZIPPLUGIN_H
#define LIBZIPPLUGIN_H

#include "archiveinterface.h"


#include <zip.h>
#include "kpluginfactory.h"


struct FileProgressInfo {
    float fileProgressProportion = 0.0;
    float fileProgressStart;
    QString fileName;
};

class LibzipPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libzip.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit LibzipPluginFactory();
    ~LibzipPluginFactory();
};

class LibzipPlugin : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit LibzipPlugin(QObject *parent, const QVariantList &args);
    ~LibzipPlugin() override;

    bool list(bool isbatch = false) override;
    bool doKill() override;
    bool extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options) override;
    bool addFiles(const QVector<Archive::Entry *> &files, const Archive::Entry *destination, const CompressionOptions &options, uint numberOfEntriesToAdd = 0) override;
    bool deleteFiles(const QVector<Archive::Entry *> &files) override;
    bool moveFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options) override;
    bool copyFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options) override;
    bool addComment(const QString &comment) override;
    bool testArchive() override;
    void cleanIfCanceled()override;
    void watchFileList(QStringList *strList)override;
private:
    bool extractEntry(zip_t *archive, const QString &entry, const QString &rootNode, const QString &destDir, bool preservePaths, bool removeRootNode, FileProgressInfo &pi);
    bool writeEntry(zip_t *archive, const QString &entry, const Archive::Entry *destination, const CompressionOptions &options, bool isDir = false);
    bool emitEntryForIndex(zip_t *archive, qlonglong index);
    void emitProgress(double percentage);
    QString permissionsToString(const mode_t &perm);
    static void progressCallback(zip_t *, double progress, void *that);
    QByteArray detectEncode(const QByteArray &data, const QString &fileName = QString());
    void detectAllfile(zip_t *archive, int num);
    QString  trans2uft8(const char *str);

    QVector<Archive::Entry *> m_emittedEntries;
    bool m_overwriteAll;
    bool m_skipAll;
    bool m_listAfterAdd;
    int m_filesize;
    zip_t *m_addarchive;
    QByteArray m_codecstr;
    QByteArray m_codecname;
    ExtractionOptions m_extractionOptions;
    bool isWrongPassword = false;
    QString m_extractDestDir;
};

#endif // LIBZIPPLUGIN_H
