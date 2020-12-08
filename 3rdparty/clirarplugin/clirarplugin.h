#ifndef CLIRARPLUGIN_H
#define CLIRARPLUGIN_H

#include "cliinterface.h"
#include "kpluginfactory.h"

class CliRarPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_clirar.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit CliRarPluginFactory();
    ~CliRarPluginFactory();
};

class CliRarPlugin : public CliInterface
{
    Q_OBJECT

public:
    explicit CliRarPlugin(QObject *parent, const QVariantList &args);
    ~CliRarPlugin() override;

    bool isPasswordPrompt(const QString &line) override;
    bool isWrongPasswordMsg(const QString &line) override;
    bool isCorruptArchiveMsg(const QString &line) override;
    bool isDiskFullMsg(const QString &line) override;
    bool isFileExistsMsg(const QString &line) override;
    bool isFileExistsFileName(const QString &line) override;
    bool isMultiPasswordPrompt(const QString &line) override;

private:
    void setupCliProperties();

    /**
     * @brief readListLine  解析加载压缩包的命令输出
     * @param line 待解析内容
     * @return
     */
    bool readListLine(const QString &line);

    bool handleLine(const QString &line, WorkType workStatus) override;

private:
    ParseState m_parseState = ParseStateTitle;
    FileEntry m_fileEntry;
    QString m_comment = "";  // 压缩包注释信息
    QString m_replaceLine = ""; // 拼接被截断的命令行内容
};

#endif // CLIRARPLUGIN_H
