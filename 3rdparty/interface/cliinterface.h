#ifndef CLIINTERFACE_H
#define CLIINTERFACE_H

#include "archiveinterface.h"
#include "cliproperties.h"
#include "kprocess.h"

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

    virtual bool readListLine(const QString &line) = 0;
    virtual bool isPasswordPrompt(const QString &line) = 0;
    virtual bool isWrongPasswordMsg(const QString &line) = 0;
    virtual bool isCorruptArchiveMsg(const QString &line) = 0;
    virtual bool isDiskFullMsg(const QString &line) = 0;
    virtual bool isFileExistsMsg(const QString &line) = 0;
    virtual bool isFileExistsFileName(const QString &line) = 0;

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

    /**
     * Handles the given @p line.
     * @return True if the line is ok. False if the line contains/triggers a "fatal" error
     * or a canceled user query. If false is returned, the caller is supposed to call killProcess().
     */
    virtual bool handleLine(const QString &line, WorkType workStatus) = 0;

    void deleteProcess();

    /**
     * Kill the running process. The finished signal is emitted according to @p emitFinished.
     */
    void killProcess(bool emitFinished = true);

protected slots:
    /**
     * @brief readStdout  读取命令行输出
     * @param handleAll
     */
    virtual void readStdout(bool handleAll = false);

    virtual void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

protected:
    CliProperties *m_cliProps = nullptr;
    KProcess *m_process = nullptr;

private:
    QByteArray m_stdOutData;
    int m_exitCode = 0;
    WorkType m_workStatus = WT_List;
    QString m_extractDestionPath;
};

#endif // CLIINTERFACE_H
