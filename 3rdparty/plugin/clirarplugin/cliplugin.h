#ifndef CLIPLUGIN_H
#define CLIPLUGIN_H

#include "../../deepin-compressor/interface/cliinterface.h"
#include "../../deepin-compressor/interface/kpluginfactory.h"

class CliPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_clirar.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit CliPluginFactory();
    ~CliPluginFactory();
};


class CliPlugin : public CliInterface
{
    Q_OBJECT

public:
    explicit CliPlugin(QObject *parent, const QVariantList &args);
    ~CliPlugin() override;

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
};

#endif // CLIPLUGIN_H
