#include "cliplugin.h"
#include "kprocess.h"

#include <QDateTime>
#include <QDir>
#include <QRegularExpression>

//K_PLUGIN_CLASS_WITH_JSON(CliPlugin, "kerfuffle_cli7z.json")

CliPluginFactory::CliPluginFactory()
{
    registerPlugin<Cli7zPlugin>();
}
CliPluginFactory::~CliPluginFactory()
{

}

Cli7zPlugin::Cli7zPlugin(QObject *parent, const QVariantList &args)
    : CliInterface(parent, args)
    , m_archiveType(ArchiveType7z)
    , m_parseState(ParseStateTitle)
    , m_linesComment(0)
    , m_isFirstInformationEntry(true)
{

    setupCliProperties();
}

Cli7zPlugin::~Cli7zPlugin()
{
}

void Cli7zPlugin::resetParsing()
{
    m_listMap.clear();
    m_parseState = ParseStateTitle;
    m_comment.clear();
    m_numberOfVolumes = 0;
}

void Cli7zPlugin::setupCliProperties()
{
    m_cliProps->setProperty("captureProgress", false);

    m_cliProps->setProperty("addProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("addSwitch", QStringList{QStringLiteral("a"),
                                                     QStringLiteral("-l")});

    m_cliProps->setProperty("deleteProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("deleteSwitch", QStringLiteral("d"));

    m_cliProps->setProperty("extractProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("extractSwitch", QStringList{QStringLiteral("x")});
    m_cliProps->setProperty("progressarg", QStringList{QStringLiteral("-bsp1")});
    m_cliProps->setProperty("extractSwitchNoPreserve", QStringList{QStringLiteral("e")});

    m_cliProps->setProperty("listProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("listSwitch", QStringList{QStringLiteral("l"),
                                                      QStringLiteral("-slt")});

    m_cliProps->setProperty("moveProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("moveSwitch", QStringLiteral("rn"));

    m_cliProps->setProperty("testProgram", QStringLiteral("7z"));
    m_cliProps->setProperty("testSwitch", QStringLiteral("t"));

    m_cliProps->setProperty("passwordSwitch", QStringList{QStringLiteral("-p$Password")});
    m_cliProps->setProperty("passwordSwitchHeaderEnc", QStringList{QStringLiteral("-p$Password"),
                                                                   QStringLiteral("-mhe=on")});
    m_cliProps->setProperty("compressionLevelSwitch", QStringLiteral("-mx=$CompressionLevel"));
    m_cliProps->setProperty("compressionMethodSwitch", QHash<QString, QVariant> {{QStringLiteral("application/x-7z-compressed"), QStringLiteral("-m0=$CompressionMethod")},
        {QStringLiteral("application/zip"), QStringLiteral("-mm=$CompressionMethod")}
    });
    m_cliProps->setProperty("encryptionMethodSwitch", QHash<QString, QVariant> {{QStringLiteral("application/x-7z-compressed"), QString()},
        {QStringLiteral("application/zip"), QStringLiteral("-mem=$EncryptionMethod")}
    });
    m_cliProps->setProperty("multiVolumeSwitch", QStringLiteral("-v$VolumeSizek"));
    m_cliProps->setProperty("testPassedPatterns", QStringList{QStringLiteral("^Everything is Ok$")});
    m_cliProps->setProperty("fileExistsFileNameRegExp", QStringList{QStringLiteral("^file \\./(.*)$"),
                                                                    QStringLiteral("^  Path:     \\./(.*)$")});
    m_cliProps->setProperty("fileExistsInput", QStringList{QStringLiteral("Y"),   //Overwrite
                                                           QStringLiteral("N"),   //Skip
                                                           QStringLiteral("A"),   //Overwrite all
                                                           QStringLiteral("S"),   //Autoskip
                                                           QStringLiteral("Q")}); //Cancel
    m_cliProps->setProperty("multiVolumeSuffix", QStringList{QStringLiteral("$Suffix.001")});
}

void Cli7zPlugin::fixDirectoryFullName()
{
//    if (m_currentArchiveEntry->isDir()) {
//        const QString directoryName = m_currentArchiveEntry->fullPath();
//        if (!directoryName.endsWith(QLatin1Char('/'))) {
//            m_currentArchiveEntry->setProperty("fullPath", QString(directoryName + QLatin1Char('/')));
//        }
//    }
    if (m_fileStat.archive_isDirectory) {
        const QString directoryName = m_fileStat.archive_fullPath;
        if (!directoryName.endsWith(QLatin1Char('/'))) {
            m_fileStat.archive_fullPath = QString(directoryName + QLatin1Char('/'));
//            m_currentArchiveEntry->setProperty("fullPath", QString(directoryName + QLatin1Char('/')));
        }
    }
}

bool Cli7zPlugin::emitEntryForIndex(ReadOnlyArchiveInterface::archive_stat &archive)
{
    setEntryVal(archive/*, m_indexCount, archive.archive_fullPath, m_DirRecord*/);
    if (m_listMap.find(archive.archive_fullPath) == m_listMap.end()) {
        m_listMap.insert(archive.archive_fullPath, archive);
    }

    return true;
}

void Cli7zPlugin::setEntryVal(ReadOnlyArchiveInterface::archive_stat &archive)
{
    if ((archive.archive_fullPath.endsWith("/") && archive.archive_fullPath.count("/") == 1) || (archive.archive_fullPath.count("/") == 0)) {
        setEntryData(archive);
    }
}

void Cli7zPlugin::setEntryData(ReadOnlyArchiveInterface::archive_stat &archive, bool isMutilFolderFile)
{
    m_currentArchiveEntry->setProperty("fullPath", archive.archive_fullPath);
    if (!isMutilFolderFile) {
        m_currentArchiveEntry->setProperty("size", archive.archive_size);
    } else {
        m_currentArchiveEntry->setProperty("size", 0);
    }

    m_currentArchiveEntry->setProperty("compressedSize", archive.archive_compressedSize);
    m_currentArchiveEntry->setProperty("timestamp", archive.archive_timestamp);
    m_currentArchiveEntry->setProperty("isDirectory", archive.archive_isDirectory);
    m_currentArchiveEntry->setProperty("permissions", archive.archive_permissions);
    m_currentArchiveEntry->setProperty("CRC", archive.archive_CRC);
    m_currentArchiveEntry->setProperty("method", archive.archive_method);
    m_currentArchiveEntry->setProperty("isPasswordProtected", archive.archive_isPasswordProtected);

    if (!m_currentArchiveEntry->fullPath().isEmpty()) {
        emit entry(m_currentArchiveEntry);
    } else {
        delete m_currentArchiveEntry;
    }
}

Archive::Entry *Cli7zPlugin::setEntryDataA(ReadOnlyArchiveInterface::archive_stat &archive)
{
    Archive::Entry *pCurEntry = new Archive::Entry(this);

    pCurEntry->setProperty("fullPath", archive.archive_fullPath);
    pCurEntry->setProperty("size", archive.archive_size);
    pCurEntry->setProperty("compressedSize", archive.archive_compressedSize);
    pCurEntry->setProperty("timestamp", archive.archive_timestamp);
    pCurEntry->setProperty("isDirectory", archive.archive_isDirectory);
    pCurEntry->setProperty("permissions", archive.archive_permissions);
    pCurEntry->setProperty("CRC", archive.archive_CRC);
    pCurEntry->setProperty("method", archive.archive_method);
    pCurEntry->setProperty("isPasswordProtected", archive.archive_isPasswordProtected);

    return pCurEntry;
}

qint64 Cli7zPlugin::extractSize(const QVector<Archive::Entry *> &files)
{
    qint64 qExtractSize = 0;
    for (Archive::Entry *e : files) {
        QString strPath = e->fullPath();
        auto iter = m_listMap.find(strPath);
        for (; iter != m_listMap.end();) {
            if (!iter.key().startsWith(strPath)) {
                break;
            } else {
                if (!iter.key().endsWith("/")) {
                    qExtractSize += iter.value().archive_size;
                }

                ++iter;
            }
        }
    }

    return qExtractSize;
}

bool Cli7zPlugin::isPasswordList()
{
    QStringList programLst = m_process->program();

    foreach (auto str, programLst) {
        if (str.startsWith("-p")) {
            return true;
        }
    }

    return false;
}

bool Cli7zPlugin::readListLine(const QString &line)
{
    static const QLatin1String archiveInfoDelimiter1("--"); // 7z 9.13+
    static const QLatin1String archiveInfoDelimiter2("----"); // 7z 9.04
    static const QLatin1String entryInfoDelimiter("----------");

    if (line.startsWith(QLatin1String("Open ERROR: Can not open the file as [7z] archive"))) {
        //  7z l -slt -p123 1G压缩文件.7z.001
        //  Open ERROR: Can not open the file as [7z] archive means password error
        if (isPasswordList()) {
            return true;
        }

        emit error(tr("Listing the archive failed."));
        return false;
    } else if (line.startsWith(QLatin1String("ERROR:")) && line.contains(QLatin1String("Can not open the file as archive"))) {
        //  7z l -slt -p123 1G压缩文件.7z.001
        //  Open ERROR: Can not open the file as [7z] archive means password error
        if (isPasswordList()) {
            return true;
        }

        emit error(tr("Listing the archive failed."));
        return false;
    }

    const QRegularExpression rxVersionLine(QStringLiteral("^p7zip Version ([\\d\\.]+) .*$"));
    QRegularExpressionMatch matchVersion;

    switch (m_parseState) {
    case ParseStateTitle:
        matchVersion = rxVersionLine.match(line);
        if (matchVersion.hasMatch()) {
            m_parseState = ParseStateHeader;
            const QString p7zipVersion = matchVersion.captured(1);
        }
        break;
    case ParseStateHeader:
        if (line.startsWith(QLatin1String("Listing archive:"))) {

        } else if ((line == archiveInfoDelimiter1) ||
                   (line == archiveInfoDelimiter2)) {
            m_parseState = ParseStateArchiveInformation;
        } else if (line.contains(QLatin1String("Error: "))) {

        }
        break;
    case ParseStateArchiveInformation:
        if (line == entryInfoDelimiter) {
            m_parseState = ParseStateEntryInformation;
        } else if (line.startsWith(QLatin1String("Type = "))) {
            const QString type = line.mid(7).trimmed();
            if (type == QLatin1String("7z")) {
                m_archiveType = ArchiveType7z;
            } else if (type == QLatin1String("bzip2")) {
                m_archiveType = ArchiveTypeBZip2;
            } else if (type == QLatin1String("gzip")) {
                m_archiveType = ArchiveTypeGZip;
            } else if (type == QLatin1String("xz")) {
                m_archiveType = ArchiveTypeXz;
            } else if (type == QLatin1String("tar")) {
                m_archiveType = ArchiveTypeTar;
            } else if (type == QLatin1String("zip")) {
                m_archiveType = ArchiveTypeZip;
            } else if (type == QLatin1String("Rar")) {
                m_archiveType = ArchiveTypeRar;
            } else if (type == QLatin1String("Split")) {
                setMultiVolume(true);
            } else if (type == QLatin1String("Udf")) {
                m_archiveType = ArchiveTypeUdf;
            } else if (type == QLatin1String("Iso")) {
                m_archiveType = ArchiveTypeIso;
            } else {
                // Should not happen
                return false;
            }
        } else if (line.startsWith(QLatin1String("Volumes = "))) {
            m_numberOfVolumes = line.section(QLatin1Char('='), 1).trimmed().toInt();
        } else if (line.startsWith(QLatin1String("Method = "))) {
            QStringList methods = line.section(QLatin1Char('='), 1).trimmed().split(QLatin1Char(' '), QString::SkipEmptyParts);
            handleMethods(methods);
        } else if (line.startsWith(QLatin1String("Comment = "))) {
            m_parseState = ParseStateComment;
            m_comment.append(line.section(QLatin1Char('='), 1) + QLatin1Char('\n'));
        }
        break;
    case ParseStateComment:
        if (line == entryInfoDelimiter) {
            m_parseState = ParseStateEntryInformation;
            if (!m_comment.trimmed().isEmpty()) {
                m_comment = m_comment.trimmed();
                m_linesComment = m_comment.count(QLatin1Char('\n')) + 1;
            }
        } else {
            m_comment.append(line + QLatin1Char('\n'));
        }
        break;
    case ParseStateEntryInformation:
        if (m_isFirstInformationEntry) {
            m_isFirstInformationEntry = false;
            m_currentArchiveEntry = new Archive::Entry(this);
            m_currentArchiveEntry->compressedSizeIsSet = false;
        }

        if (line.startsWith(QLatin1String("Path = "))) {
            const QString entryFilename = QDir::fromNativeSeparators(line.mid(7).trimmed());
            m_fileStat.archive_fullPath = entryFilename;
//            m_currentArchiveEntry->setProperty("fullPath", entryFilename);
        } else if (line.startsWith(QLatin1String("Size = "))) {
            m_fileStat.archive_size = line.mid(7).trimmed().toInt();
//            m_currentArchiveEntry->setProperty("size", line.mid(7).trimmed());
        } else if (line.startsWith(QLatin1String("Packed Size = "))) {
            // #236696: 7z files only show a single Packed Size value
            //          corresponding to the whole archive.
            if (m_archiveType != ArchiveType7z) {
                m_currentArchiveEntry->compressedSizeIsSet = true;
                m_fileStat.archive_compressedSize = line.mid(14).trimmed().toInt();
//                m_currentArchiveEntry->setProperty("compressedSize", line.mid(14).trimmed());
            }
        } else if (line.startsWith(QLatin1String("Modified = "))) {
            m_fileStat.archive_timestamp = QDateTime::fromString(line.mid(11).trimmed(), QStringLiteral("yyyy-MM-dd hh:mm:ss"));
//            m_currentArchiveEntry->setProperty("timestamp", QDateTime::fromString(line.mid(11).trimmed(), QStringLiteral("yyyy-MM-dd hh:mm:ss")));
            if (ArchiveTypeIso == m_archiveType) {
                m_isFirstInformationEntry = true;
//                if (!m_currentArchiveEntry->fullPath().isEmpty()) {
//                    emit entry(m_currentArchiveEntry);
//                } else {
//                    delete m_currentArchiveEntry;
//                }

                m_currentArchiveEntry = nullptr;
            }
        } else if (line.startsWith(QLatin1String("Folder = "))) {
            const QString isDirectoryStr = line.mid(9).trimmed();
            Q_ASSERT(isDirectoryStr == QStringLiteral("+") || isDirectoryStr == QStringLiteral("-"));
            const bool isDirectory = isDirectoryStr.startsWith(QLatin1Char('+'));
            m_fileStat.archive_isDirectory = isDirectory;
//            m_currentArchiveEntry->setProperty("isDirectory", isDirectory);
            fixDirectoryFullName();
        } else if (line.startsWith(QLatin1String("Attributes = "))) {
            const QString attributes = line.mid(13).trimmed();
            if (attributes.contains(QLatin1Char('D'))) {
                m_fileStat.archive_isDirectory = true;
//                m_currentArchiveEntry->setProperty("isDirectory", true);
                fixDirectoryFullName();
            } else {
                m_fileStat.archive_isDirectory = false;
            }

            if (attributes.contains(QLatin1Char('_'))) {
                m_fileStat.archive_permissions = attributes.mid(attributes.indexOf(QLatin1Char(' ')) + 1);
                // Unix attributes
//                m_currentArchiveEntry->setProperty("permissions", attributes.mid(attributes.indexOf(QLatin1Char(' ')) + 1));
            } else {
                m_fileStat.archive_permissions = attributes;
                // FAT attributes
//                m_currentArchiveEntry->setProperty("permissions", attributes);
            }
        } else if (line.startsWith(QLatin1String("CRC = "))) {
            m_fileStat.archive_CRC = line.mid(6).trimmed();
//            m_currentArchiveEntry->setProperty("CRC", line.mid(6).trimmed());
        } else if (line.startsWith(QLatin1String("Method = "))) {
            m_fileStat.archive_method = line.mid(9).trimmed();
//            m_currentArchiveEntry->setProperty("method", line.mid(9).trimmed());
            // For zip archives we need to check method for each entry.
            if (m_archiveType == ArchiveTypeZip) {
                QStringList methods = line.section(QLatin1Char('='), 1).trimmed().split(QLatin1Char(' '), QString::SkipEmptyParts);
                handleMethods(methods);
            }
        } else if (line.startsWith(QLatin1String("Encrypted = ")) && line.size() >= 13) {
            m_fileStat.archive_isPasswordProtected = line.at(12) == QLatin1Char('+');
//            m_currentArchiveEntry->setProperty("isPasswordProtected", line.at(12) == QLatin1Char('+'));
        } else if (line.startsWith(QLatin1String("Block = ")) || line.startsWith(QLatin1String("Version = "))) {
            m_isFirstInformationEntry = true;
            emitEntryForIndex(m_fileStat);
//            if (!m_currentArchiveEntry->fullPath().isEmpty()) {
//                emit entry(m_currentArchiveEntry);
//            } else {
//                delete m_currentArchiveEntry;
//            }

            m_currentArchiveEntry = nullptr;
        } else if (line.startsWith(QLatin1String("Accessed = ")) && ArchiveTypeUdf == m_archiveType) {
            m_isFirstInformationEntry = true;
//            if (!m_currentArchiveEntry->fullPath().isEmpty()) {
//                emit entry(m_currentArchiveEntry);
//            } else {
//                delete m_currentArchiveEntry;
//            }

            m_currentArchiveEntry = nullptr;
        } else if (line.startsWith(QLatin1String("Hard Link =")) && ArchiveTypeTar == m_archiveType) {
            m_isFirstInformationEntry = true;
//            if (!m_currentArchiveEntry->fullPath().isEmpty()) {
//                emit entry(m_currentArchiveEntry);
//            } else {
//                delete m_currentArchiveEntry;
//            }

            m_currentArchiveEntry = nullptr;
        }

        break;
    }

    return true;
}

bool Cli7zPlugin::readExtractLine(const QString &line)
{
    if (line.startsWith(QLatin1String("ERROR: E_FAIL"))) {
        emit error(tr("Extraction failed due to an unknown error."));
        return false;
    }

    if (line.startsWith(QLatin1String("ERROR: CRC Failed")) ||
            line.startsWith(QLatin1String("ERROR: Headers Error"))) {
        emit error(tr("Extraction failed due to one or more corrupt files. Any extracted files may be damaged."));
        return false;
    }

    return true;
}

bool Cli7zPlugin::readDeleteLine(const QString &line)
{
    if (line.startsWith(QLatin1String("Error: ")) &&
            line.endsWith(QLatin1String(" is not supported archive"))) {
        emit error(tr("Delete operation failed. Try upgrading p7zip or disabling the p7zip plugin in the configuration dialog."));
        return false;
    }

    return true;
}

void Cli7zPlugin::handleMethods(const QStringList &methods)
{
    for (const QString &method : methods) {

        QRegularExpression rxEncMethod(QStringLiteral("^(7zAES|AES-128|AES-192|AES-256|ZipCrypto)$"));
        if (rxEncMethod.match(method).hasMatch()) {
            QRegularExpression rxAESMethods(QStringLiteral("^(AES-128|AES-192|AES-256)$"));
            if (rxAESMethods.match(method).hasMatch()) {
                // Remove dash for AES methods.
                emit encryptionMethodFound(QString(method).remove(QLatin1Char('-')));
            } else {
                emit encryptionMethodFound(method);
            }
            continue;
        }

        // LZMA methods are output with some trailing numbers by 7z representing dictionary/block sizes.
        // We are not interested in these, so remove them.
        if (method.startsWith(QLatin1String("LZMA2"))) {
            emit compressionMethodFound(method.left(5));
        } else if (method.startsWith(QLatin1String("LZMA"))) {
            emit compressionMethodFound(method.left(4));
        } else if (method == QLatin1String("xz")) {
            emit compressionMethodFound(method.toUpper());
        } else {
            emit compressionMethodFound(method);
        }
    }
}

bool Cli7zPlugin::isPasswordPrompt(const QString &line)
{
    return line.startsWith(QLatin1String("Enter password (will not be echoed):"));
}

bool Cli7zPlugin::isWrongPasswordMsg(const QString &line)
{
    return line.contains(QLatin1String("Wrong password"));
}

bool Cli7zPlugin::isCorruptArchiveMsg(const QString &line)
{
    return (line == QLatin1String("Unexpected end of archive") ||
            line == QLatin1String("Headers Error"));
}

bool Cli7zPlugin::isDiskFullMsg(const QString &line)
{
    return line.contains(QLatin1String("No space left on device"));
}

bool Cli7zPlugin::isFileExistsMsg(const QString &line)
{
    return (line == QLatin1String("(Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? ") ||
            line == QLatin1String("? (Y)es / (N)o / (A)lways / (S)kip all / A(u)to rename all / (Q)uit? "));
}

bool Cli7zPlugin::isFileExistsFileName(const QString &line)
{
    return (line.startsWith(QLatin1String("file ./")) ||
            line.startsWith(QLatin1String("  Path:     ./")));
}

void Cli7zPlugin::watchFileList(QStringList *strList)
{
    CliInterface::watchFileList(strList);
}

void Cli7zPlugin::showEntryListFirstLevel(const QString &directory)
{
    if (directory.isEmpty()) return;
    auto iter = m_listMap.find(directory);
    for (; iter != m_listMap.end() ;) {
        if (iter.key().left(directory.size()) != directory) {
            break;
        } else {
            QString chopStr = iter.key().right(iter.key().size() - directory.size());
            if (!chopStr.isEmpty()) {
                if ((chopStr.endsWith("/") && chopStr.count("/") == 1) || chopStr.count("/") == 0) {
                    Archive::Entry *fileEntry = setEntryDataA(iter.value());
                    RefreshEntryFileCount(fileEntry);
                    emit entry(fileEntry);
//                    m_emittedEntries << fileEntry;
                }
            }

            ++iter;
        }
    }
}

void Cli7zPlugin::RefreshEntryFileCount(Archive::Entry *file)
{
    if (!file || !file->isDir()) return;
    qulonglong count = 0;
    auto iter = m_listMap.find(file->fullPath());
    for (; iter != m_listMap.end();) {
        if (!iter.key().startsWith(file->fullPath())) {
            break;
        } else {
            if (iter.key().size() > file->fullPath().size()) {
                QString chopStr = iter.key().right(iter.key().size() - file->fullPath().size());
                if ((chopStr.endsWith("/") && chopStr.count("/") == 1) || chopStr.count("/") == 0) {
                    ++count;
                }
            }

            ++iter;
        }

        file->setProperty("size", count);
    }
}

//#include "moc_cliplugin.cpp"
