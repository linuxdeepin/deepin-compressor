#ifndef SINGLEFILEPLUGIN_H
#define SINGLEFILEPLUGIN_H

#include "archiveinterface.h"

class  LibSingleFileInterface : public ReadOnlyArchiveInterface
{
    Q_OBJECT

public:
    LibSingleFileInterface(QObject *parent, const QVariantList &args);
    ~LibSingleFileInterface() ;

    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;
    void pauseOperation() override;
    void continueOperation() override;
    bool doKill() override;

protected:
    /**
     * @brief uncompressedFileName  获取解压文件名
     * @return
     */
    const QString uncompressedFileName() const;

    QString m_mimeType;
    QStringList m_possibleExtensions;

};

#endif // SINGLEFILEPLUGIN_H
