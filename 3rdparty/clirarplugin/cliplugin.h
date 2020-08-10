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
};

#endif // CLIPLUGIN_H
