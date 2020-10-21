#ifndef CLIINTERFACE_H
#define CLIINTERFACE_H

#include "archiveinterface.h"
#include "cliproperties.h"
#include "kprocess.h"

enum WorkType {
    WT_List,
    WT_Extract,
    WT_Add,
    WT_Delete,
    WT_Move,
    WT_Copy,
    WT_Comment,
    WT_Test
};

enum ParseState {
    ParseStateTitle = 0,
    ParseStateHeader,
    ParseStateArchiveInformation,
    ParseStateEntryInformation
};

class CliInterface : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit CliInterface(QObject *parent, const QVariantList &args);
    ~CliInterface() override;

    // ReadOnlyArchiveInterface interface
public:
    PluginFinishType list() override;
    PluginFinishType testArchive() override;
    PluginFinishType extractFiles(const QVector<FileEntry> &files, const ExtractionOptions &options) override;

    virtual bool isPasswordPrompt(const QString &line) = 0;
    virtual bool isWrongPasswordMsg(const QString &line) = 0;
    virtual bool isCorruptArchiveMsg(const QString &line) = 0;
    virtual bool isDiskFullMsg(const QString &line) = 0;
    virtual bool isFileExistsMsg(const QString &line) = 0;
    virtual bool isFileExistsFileName(const QString &line) = 0;

    // ReadWriteArchiveInterface interface
public:
    PluginFinishType addFiles(const QVector<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType moveFiles(const QVector<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType copyFiles(const QVector<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType deleteFiles(const QVector<FileEntry> &files) override;
    PluginFinishType addComment(const QString &comment) override;

protected:
    /**
     * @brief setListEmptyLines  需要解析空的命令行输出(rar格式含有注释的情况)
     * @param emptyLines  默认不需要解析
     */
    void setListEmptyLines(bool emptyLines = false);

    /**
     * @brief runProcess  执行命令
     * @param programName  命令
     * @param arguments  命令参数
     * @return
     */
    bool runProcess(const QString &programName, const QStringList &arguments);

    /**
     * @brief handleLine  处理命令行输出
     * @param line  行内容
     * @param workStatus 当前进程工作类型
     * @return
     */
    virtual bool handleLine(const QString &line, WorkType workStatus) = 0;

    /**
     * @brief deleteProcess 删除进程
     */
    void deleteProcess();

    /**
     * @brief killProcess  结束进程
     * @param emitFinished
     */
    void killProcess(bool emitFinished = true);

protected slots:
    /**
     * @brief readStdout  读取命令行输出
     * @param handleAll
     */
    virtual void readStdout(bool handleAll = false);

    /**
     * @brief processFinished  进程结束
     * @param exitCode   进程退出码
     * @param exitStatus  结束状态
     */
    virtual void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

protected:
    CliProperties *m_cliProps = nullptr;  // 命令属性
    KProcess *m_process = nullptr;  // 工作进程

private:
    bool m_listEmptyLines = false;
    QByteArray m_stdOutData;  // 存储命令行输出数据
//    int m_exitCode = 0;
    WorkType m_workStatus = WT_List;  // 记录当前工作状态（add、list、extract...）
};

#endif // CLIINTERFACE_H
