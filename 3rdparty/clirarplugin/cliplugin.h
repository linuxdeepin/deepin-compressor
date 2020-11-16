#ifndef CLIPLUGIN_H
#define CLIPLUGIN_H

#include "../interface/cliinterface.h"
#include "../interface/kpluginfactory.h"

class CliPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_clirar.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit CliPluginFactory();
    ~CliPluginFactory();
};


class CliRarPlugin : public CliInterface
{
    Q_OBJECT

public:
    explicit CliRarPlugin(QObject *parent, const QVariantList &args);
    ~CliRarPlugin() override;

    void resetParsing() override;
    bool readListLine(const QString &line) override;
    bool readExtractLine(const QString &line) override;
    bool hasBatchExtractionProgress() const override;
    bool isPasswordPrompt(const QString &line) override;
    bool isWrongPasswordMsg(const QString &line) override;
    bool isCorruptArchiveMsg(const QString &line) override;
    bool isDiskFullMsg(const QString &line) override;
    bool isFileExistsMsg(const QString &line) override;
    bool isFileExistsFileName(const QString &line) override;
    bool isLocked() const override;
    bool isPromptMultiPassword(const QString &line) override;

    virtual void showEntryListFirstLevel(const QString &directory) override;
    virtual void RefreshEntryFileCount(Archive::Entry *file) override;

private:

    enum ParseState {
        ParseStateTitle = 0,
        ParseStateComment,
        ParseStateHeader,
        ParseStateEntryFileName,
        ParseStateEntryDetails,
        ParseStateLinkTarget
    } m_parseState;

    void setupCliProperties();

    bool handleUnrar5Line(const QString &line);
    void handleUnrar5Entry();
    bool handleUnrar4Line(const QString &line);
    void handleUnrar4Entry();
    void ignoreLines(int lines, ParseState nextState);

    bool emitEntryForIndex(archive_stat &archive);
    void setEntryVal(archive_stat &archive);
    void setEntryData(archive_stat &archive, bool isMutilFolderFile = false);
    Archive::Entry *setEntryDataA(archive_stat &archive);
    virtual qint64 extractSize(const QVector<Archive::Entry *> &files) override;

    /**
     * @brief handleEnterPwdLine 处理提示输入密码的命令行内容被截断
     * @param line
     * @return
     */
    bool handleEnterPwdLine(const QString &line);

    /**
     * @brief handleIncorrectPwdLine 处理提示密码错误的命令行内容被截断
     * @param line
     * @return
     */
    bool handleIncorrectPwdLine(const QString &line);

    /**
     * @brief handleFileExistsLine 处理提示文件已存在是否替换的命令行内容被截断
     * @param line
     * @return
     */
    bool handleFileExistsLine(const QString &line);

    bool isPromptEnterPwdLine(const QString &line);  // 提示需要输入密码
    bool isPromptIncorrectPwdLine(const QString &line);  // 提示密码错误
    bool isPromptFileExistsLine(const QString &line);  // 提示已存在同名文件

private:
    QStringList m_unrar4Details;
    QHash<QString, QString> m_unrar5Details;

    QString m_unrarVersion;
    bool m_isUnrar5;
    bool m_isPasswordProtected;
    bool m_isSolid;
    bool m_isRAR5;
    bool m_isLocked;

    int m_remainingIgnoreLines;
    int m_linesComment;

//    QMap<QString, archive_stat> m_listMap;
    Archive::Entry *m_rarArchiveEntry;
    archive_stat m_fileStat;
//    QString m_replaceLine = ""; // 拼接被截断的命令行内容
};

#endif // CLIPLUGIN_H
