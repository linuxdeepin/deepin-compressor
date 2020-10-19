#ifndef CLIINTERFACE_H
#define CLIINTERFACE_H

#include "archiveinterface.h"
#include "cliproperties.h"

class CliInterface : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit CliInterface(QObject *parent, const QVariantList &args);
    ~CliInterface() override;

    // ReadOnlyArchiveInterface interface
public:
    bool list() override;
    bool testArchive() override;
    bool extractFiles(const QVector<FileEntry> &files, const ExtractionOptions &options) override;

    // ReadWriteArchiveInterface interface
public:
    bool addFiles(const QVector<FileEntry> &files, const CompressOptions &options) override;
    bool moveFiles(const QVector<FileEntry> &files, const CompressOptions &options) override;
    bool copyFiles(const QVector<FileEntry> &files, const CompressOptions &options) override;
    bool deleteFiles(const QVector<FileEntry> &files) override;
    bool addComment(const QString &comment) override;

protected:
    /**
     * @brief runProcess  执行命令
     * @param programName  命令
     * @param arguments  命令参数
     * @return
     */
    bool runProcess(const QString &programName, const QStringList &arguments);

protected slots:
    /**
     * @brief readStdout  读取命令行输出
     * @param handleAll
     */
    virtual void readStdout(bool handleAll = false);

protected:
    CliProperties *m_cliProps = nullptr;
};

#endif // CLIINTERFACE_H
