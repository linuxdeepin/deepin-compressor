#ifndef CLIPLUGIN_H
#define CLIPLUGIN_H

#include "cliinterface.h"
#include "kpluginfactory.h"


class CliPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_cli7z.json")
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
    bool readDeleteLine(const QString &line) override;
    bool isPasswordPrompt(const QString &line) override;
    bool isWrongPasswordMsg(const QString &line) override;
    bool isCorruptArchiveMsg(const QString &line) override;
    bool isDiskFullMsg(const QString &line) override;
    bool isFileExistsMsg(const QString &line) override;
    bool isFileExistsFileName(const QString &line) override;

private:
    enum ArchiveType {
        ArchiveType7z = 0,
        ArchiveTypeBZip2,
        ArchiveTypeGZip,
        ArchiveTypeXz,
        ArchiveTypeTar,
        ArchiveTypeZip,
        ArchiveTypeRar
    } m_archiveType;

    enum ParseState {
        ParseStateTitle = 0,
        ParseStateHeader,
        ParseStateArchiveInformation,
        ParseStateComment,
        ParseStateEntryInformation
    } m_parseState;

    void setupCliProperties();
    void handleMethods(const QStringList &methods);
    void fixDirectoryFullName();

    int m_linesComment;
    Archive::Entry *m_currentArchiveEntry;
    bool m_isFirstInformationEntry;
};


#endif // CLIPLUGIN_H
