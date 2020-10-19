#ifndef QUERIES_H
#define QUERIES_H

#include <DApplicationHelper>
#include <DDialog>

#include <QMutex>
#include <QVariant>
#include <QWaitCondition>

DWIDGET_USE_NAMESPACE

/**
 * @brief The OverwriteQuery_Result enum   处理文件已存在时的选项
 */
enum OverwriteQuery_Result {
    Result_Cancel = 0,            // 取消
    Result_Skip = 1,              // 跳过
    Result_SkipAll = 2,           // 全部跳过
    Result_Overwrite = 3,         // 替换
    Result_OverwriteAll = 4,      // 全部替换
    Result_Resume = 5,
    Result_ResumeAll = 6,
    Result_Rename = 7,
    Result_RenameAll = 8,
    Result_Retry = 9
};

class Query : public QObject
{
    Q_OBJECT
public:
    virtual void execute() = 0;
    //    int execDialog();

    void setParent(QWidget *pParent);
    QWidget *getParent();

    /**
     * @brief waitForResponse   等待对话框做出选择
     */
    void waitForResponse();

    /**
     * @brief setResponse   对话框做出的选择
     * @param response      对话框的选择
     */
    void setResponse(const QVariant &response);

    /**
     * @brief getResponse   获取对话框做出的选择
     * @return
     */
    QVariant getResponse() const;

    /**
     * @brief toShortString     字符串过长时保留前后各8位，省略号替换未显示的字符
     * @param strSrc            字符串
     * @param limitCounts       保留字符长度最长为16
     * @param left              左侧字符长度为8
     * @return
     */
    static QString toShortString(QString strSrc, int limitCounts = 16, int left = 8);

protected:
    Query();
    virtual ~Query() {}

//    void colorRoleChange(QWidget *widget, DPalette::ColorRole ct, double alphaF);
//    void colorTypeChange(QWidget *widget, DPalette::ColorType ct, double alphaF);

    QWidget *m_pParent = nullptr;
    QHash<QString, QVariant> m_data;

private:
    QWaitCondition m_responseCondition;
    QMutex m_responseMutex;
};

/**
 * @brief The OverwriteQuery class    提示文件已存在，是否跳过或替换
 */
class OverwriteQuery : public Query
{
    Q_OBJECT
public:
    explicit OverwriteQuery(const QString &filename);
    void execute() override;

    /**
     * @brief getExecuteReturn      获取当前提示框状态
     * @return
     */
    int getExecuteReturn();

    /**
     * @brief getResponseCancell      是否取消
     * @return
     */
    bool getResponseCancell();

    /**
     * @brief getResponseOverwrite      是否替换
     * @return
     */
    bool getResponseOverwrite();

    /**
     * @brief getResponseOverwriteAll   是否全部替换
     * @return
     */
    bool getResponseOverwriteAll();

    /**
     * @brief getResponseRename     是否重命名
     * @return
     */
    bool getResponseRename();

    /**
     * @brief getResponseSkip    是否跳过
     * @return
     */
    bool getResponseSkip();

    /**
     * @brief getResponseSkipAll    是否全部跳过
     * @return
     */
    bool getResponseSkipAll();

    /**
     * @brief getNewFilename    获取新的文件名
     * @return
     */
    QString getNewFilename();

//    void setNoRenameMode(bool enableNoRenameMode);
//    bool getNoRenameMode();
//    void setMultiMode(bool enableMultiMode);
//    bool getMultiMode();

    /**
     * @brief applyAll    是否应用到全部
     * @return
     */
    bool getApplyAll();

private:
    bool m_noRenameMode;
    bool m_multiMode;
    int m_dialogMode;
    bool m_applyAll;     // 是否勾选应用到全部
};

/**
 * @brief The PasswordNeededQuery class    解压提示输入密码
 */
class PasswordNeededQuery : public Query
{
    Q_OBJECT
public:
    explicit PasswordNeededQuery(const QString &archiveFilename, bool incorrectTryAgain = false);
    void execute() override;

//    bool responseCancelled();
//    QString getInputPassword();
};

/**
 * @brief The AddCompressUsePasswordQuery class   追加压缩提示是否使用密码
 */
class AddCompressUsePasswordQuery : public Query
{
    Q_OBJECT
public:
    explicit AddCompressUsePasswordQuery();
    void execute() override;

    /**
     * @brief getIsUsePassword    获取是否使用密码
     * @return
     */
    bool getIsUsePassword();

    /**
     * @brief getPassword   获取密码
     * @return
     */
    QString getPassword();

private:
    bool m_isUsePassword = false;           // 是勾选否使用密码
    QString m_password = "";            // 密码
};

#endif // QUERIES_H
