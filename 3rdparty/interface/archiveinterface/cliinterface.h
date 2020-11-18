#ifndef CLIINTERFACE_H
#define CLIINTERFACE_H

//#include "archiveinterface.h"
#include "cliproperties.h"
#include "kptyprocess.h"

#include <QScopedPointer>
#include <QTemporaryDir>

//enum WorkType {
//    WT_List,
//    WT_Extract,
//    WT_Add,
//    WT_Delete,
//    WT_Move,
//    WT_Copy,
//    WT_Comment,
//    WT_Test
//};

enum ParseState {
    ParseStateTitle = 0,
    ParseStateHeader,
    ParseStateArchiveInformation,
    ParseStateEntryInformation
};

class PasswordNeededQuery;

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
    PluginFinishType extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options) override;

    virtual bool isPasswordPrompt(const QString &line) = 0;
    virtual bool isWrongPasswordMsg(const QString &line) = 0;
    virtual bool isCorruptArchiveMsg(const QString &line) = 0;
    virtual bool isDiskFullMsg(const QString &line) = 0;
    virtual bool isFileExistsMsg(const QString &line) = 0;
    virtual bool isFileExistsFileName(const QString &line) = 0;

    /**
     * @brief pauseOperation    暂停操作
     */
    void pauseOperation() override;

    /**
     * @brief continueOperation 继续操作
     */
    void continueOperation() override;

    /**
     * @brief doKill 强行取消
     */
    bool doKill() override;

    // ReadWriteArchiveInterface interface
public:
    PluginFinishType addFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType moveFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType copyFiles(const QList<FileEntry> &files, const CompressOptions &options) override;
    PluginFinishType deleteFiles(const QList<FileEntry> &files) override;
    PluginFinishType addComment(const QString &comment) override;
    PluginFinishType updateArchiveData() override;

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

    /**
     * @brief handleProgress  解析进度并发送进度信号
     * @param line
     */
    void handleProgress(const QString &line);

    /**
     * @brief handlePassword  需要密码是弹出密码框
     */
    void handlePassword();

    /**
     * @brief handleFileExists 处理解压存在同名文件
     * @param line
     * @return
     */
    bool handleFileExists(const QString &line);

private:

    /**
     * @brief writeToProcess 追加命令
     * @param data
     */
    void writeToProcess(const QByteArray &data);

    /**
     * @brief moveDroppedFilesToDest 提取文件后，将文件从临时目录移到目的路径
     * @param files  文件
     * @param options 解压选项
     * @return
     */
    bool moveExtractTempFilesToDest(const QList<FileEntry> &files, const ExtractionOptions &options);

private slots:
    /**
     * @brief readStdout  读取命令行输出
     * @param handleAll  是否读取结束，默认未结束
     */
    virtual void readStdout(bool handleAll = false);

    /**
     * @brief processFinished  进程结束
     * @param exitCode   进程退出码
     * @param exitStatus  结束状态
     */
    virtual void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

    /**
     * @brief extractProcessFinished 解压进程结束
     * @param exitCode   进程退出码
     * @param exitStatus  结束状态
     */
    void extractProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

protected:
    CliProperties *m_cliProps = nullptr;  // 命令属性
    /*KProcess*/KPtyProcess *m_process = nullptr;  // 工作进程

private:
    QList<FileEntry> m_files; // 待解压的文件
    ExtractionOptions m_options; // 解压选项
    bool m_listEmptyLines = false; // true:rar加载list， false:7z加载list
    QByteArray m_stdOutData;  // 存储命令行输出数据
    //WorkType m_workStatus = WT_List;  // 记录当前工作状态（add、list、extract...）
    QString m_parseName;  // 解析后的文件名
    QScopedPointer<QTemporaryDir> m_extractTempDir;  // 提取文件的临时文件夹
    QString m_rootNode = ""; // 待提取文件的节点
};

#endif // CLIINTERFACE_H
