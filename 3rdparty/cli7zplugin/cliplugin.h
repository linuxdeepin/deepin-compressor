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
    bool isPasswordList();
    bool readListLine(const QString &line) override;
    bool readExtractLine(const QString &line) override;
    bool readDeleteLine(const QString &line) override;
    bool isPasswordPrompt(const QString &line) override;
    bool isWrongPasswordMsg(const QString &line) override;
    bool isCorruptArchiveMsg(const QString &line) override;
    bool isDiskFullMsg(const QString &line) override;
    bool isFileExistsMsg(const QString &line) override;
    bool isFileExistsFileName(const QString &line) override;
    void watchFileList(QStringList *strList)override;

    virtual void showEntryListFirstLevel(const QString &directory) override;
    virtual void RefreshEntryFileCount(Archive::Entry *file) override;

private:
    enum ArchiveType {
        ArchiveType7z = 0,
        ArchiveTypeBZip2,
        ArchiveTypeGZip,
        ArchiveTypeXz,
        ArchiveTypeTar,
        ArchiveTypeZip,
        ArchiveTypeRar,
        ArchiveTypeUdf,
        ArchiveTypeIso
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

    bool emitEntryForIndex(archive_stat &archive);
    void setEntryVal(archive_stat &archive);
//    void setEntryVal(archive_stat &archive, int &index, const QString &name, QString &dirRecord);
    void setEntryData(archive_stat &archive, bool isMutilFolderFile = false);
//    Archive::Entry *setEntryData(archive_stat &archive, qlonglong index, const QString &name, bool isMutilFolderFile = false);
    Archive::Entry *setEntryDataA(archive_stat &archive);
    virtual qint64 extractSize(const QVector<Archive::Entry *> &files) override;

    QString m_DirRecord;
    QString m_SigDirRecord;
    int m_indexCount = 0;

    int m_linesComment;
    Archive::Entry *m_currentArchiveEntry;
    bool m_isFirstInformationEntry;

    QMap<QString, archive_stat> m_listMap;
    archive_stat m_fileStat;
};


#endif // CLIPLUGIN_H
