#include "cliplugin.h"
#include "../interface/archiveentry.h"
#include <QDateTime>
#include <QDir>


//K_PLUGIN_CLASS_WITH_JSON(CliPlugin, "kerfuffle_clirar.json")
CliPluginFactory::CliPluginFactory()
{
    registerPlugin<CliRarPlugin>();
}

CliPluginFactory::~CliPluginFactory()
{

}

CliRarPlugin::CliRarPlugin(QObject *parent, const QVariantList &args)
    : CliInterface(parent, args)
    , m_parseState(ParseStateTitle)
    , m_isUnrar5(false)
    , m_isPasswordProtected(false)
    , m_isSolid(false)
    , m_isRAR5(false)
    , m_isLocked(false)
    , m_remainingIgnoreLines(1) //The first line of UNRAR output is empty.
    , m_linesComment(0)
{
    // Empty lines are needed for parsing output of unrar.
    setListEmptyLines(true);

    setupCliProperties();
}

CliRarPlugin::~CliRarPlugin()
{
}

void CliRarPlugin::resetParsing()
{
    m_listMap.clear();
    m_parseState = ParseStateTitle;
    m_remainingIgnoreLines = 1;
    m_unrarVersion.clear();
    m_comment.clear();
    m_numberOfVolumes = 0;
}

void CliRarPlugin::setupCliProperties()
{

    m_cliProps->setProperty("captureProgress", true);

    m_cliProps->setProperty("addProgram", QStringLiteral("rar"));
    m_cliProps->setProperty("addSwitch", QStringList({QStringLiteral("a")}));

    m_cliProps->setProperty("deleteProgram", QStringLiteral("rar"));
    m_cliProps->setProperty("deleteSwitch", QStringLiteral("d"));

    m_cliProps->setProperty("extractProgram", QStringLiteral("unrar"));
    m_cliProps->setProperty("extractSwitch", QStringList{QStringLiteral("x"),
                                                         QStringLiteral("-kb"),
                                                         QStringLiteral("-p-")});
    m_cliProps->setProperty("extractSwitchNoPreserve", QStringList{QStringLiteral("e"),
                                                                   QStringLiteral("-kb"),
                                                                   QStringLiteral("-p-")});

    m_cliProps->setProperty("listProgram", QStringLiteral("unrar"));
    m_cliProps->setProperty("listSwitch", QStringList{QStringLiteral("vt"),
                                                      QStringLiteral("-v")});

    m_cliProps->setProperty("moveProgram", QStringLiteral("rar"));
    m_cliProps->setProperty("moveSwitch", QStringLiteral("rn"));

    m_cliProps->setProperty("testProgram", QStringLiteral("unrar"));
    m_cliProps->setProperty("testSwitch", QStringLiteral("t"));

    m_cliProps->setProperty("commentSwitch", QStringList{QStringLiteral("c"),
                                                         QStringLiteral("-z$CommentFile")});

    m_cliProps->setProperty("passwordSwitch", QStringList{QStringLiteral("-p$Password")});
    m_cliProps->setProperty("passwordSwitchHeaderEnc", QStringList{QStringLiteral("-hp$Password")});

    m_cliProps->setProperty("compressionLevelSwitch", QStringLiteral("-m$CompressionLevel"));
    m_cliProps->setProperty("compressionMethodSwitch", QHash<QString, QVariant> {{QStringLiteral("application/vnd.rar"), QStringLiteral("-ma$CompressionMethod")},
        {QStringLiteral("application/x-rar"), QStringLiteral("-ma$CompressionMethod")}
    });
    m_cliProps->setProperty("multiVolumeSwitch", QStringLiteral("-v$VolumeSizek"));


    m_cliProps->setProperty("testPassedPatterns", QStringList{QStringLiteral("^All OK$")});
    m_cliProps->setProperty("fileExistsFileNameRegExp", QStringList{QStringLiteral("^(.+) already exists. Overwrite it"),  // unrar 3 & 4
                                                                    QStringLiteral("^Would you like to replace the existing file (.+)$")}); // unrar 5
    m_cliProps->setProperty("fileExistsInput", QStringList{QStringLiteral("Y"),   //Overwrite
                                                           QStringLiteral("N"),   //Skip
                                                           QStringLiteral("A"),   //Overwrite all
                                                           QStringLiteral("E"),   //Autoskip
                                                           QStringLiteral("Q")}); //Cancel

    // rar will sometimes create multi-volume archives where first volume is
    // called name.part1.rar and other times name.part01.rar.
    m_cliProps->setProperty("multiVolumeSuffix", QStringList{QStringLiteral("part01.$Suffix"),
                                                             QStringLiteral("part1.$Suffix")});
}

bool CliRarPlugin::readListLine(const QString &line)
{
    // Ignore number of lines corresponding to m_remainingIgnoreLines.
    if (m_remainingIgnoreLines > 0) {
        --m_remainingIgnoreLines;
        return true;
    }

    // Parse the title line, which contains the version of unrar.
    if (m_parseState == ParseStateTitle) {
        QRegularExpression rxVersionLine(QStringLiteral("^UNRAR (\\d+\\.\\d+)( beta \\d)? .*$"));
        QRegularExpressionMatch matchVersion = rxVersionLine.match(line);

        if (matchVersion.hasMatch()) {
            m_parseState = ParseStateComment;
            m_unrarVersion = matchVersion.captured(1);
            if (m_unrarVersion.toFloat() >= 5) {
                m_isUnrar5 = true;
            }
        }  else {
            // If the second line doesn't contain an UNRAR title, something
            // is wrong.
            return false;
        }

        // Or see what version of unrar we are dealing with and call specific
        // handler functions.
    } else if (m_isUnrar5) {
        return handleUnrar5Line(line);
    } else {
        return handleUnrar4Line(line);
    }

    return true;
}

bool CliRarPlugin::handleUnrar5Line(const QString &line)
{
    if (line.startsWith(QLatin1String("Cannot find volume "))) {
        emit error(tr("Failed to find all archive volumes."));
        return false;
    }

    switch (m_parseState) {
    // Parses the comment field.
    case ParseStateComment:
        // "Archive: " is printed after the comment.
        // FIXME: Comment itself could also contain the searched string.
        if (line.startsWith(QLatin1String("Archive: "))) {
            m_parseState = ParseStateHeader;
            m_comment = m_comment.trimmed();
            m_linesComment = m_comment.count(QLatin1Char('\n')) + 1;
            if (!m_comment.isEmpty()) {
            }
        } else {
            m_comment.append(line + QLatin1Char('\n'));
        }

        break;
    // Parses the header, which is whatever is between the comment field
    // and the entries.
    case ParseStateHeader:
        // "Details: " indicates end of header.
        if (line.startsWith(QLatin1String("Details: "))) {
            ignoreLines(1, ParseStateEntryDetails);
            if (line.contains(QLatin1String("volume"))) {
                m_numberOfVolumes++;
                if (!isMultiVolume()) {
                    setMultiVolume(true);
                }
            }

            if (line.contains(QLatin1String("solid")) && !m_isSolid) {
                m_isSolid = true;
            }

            if (line.contains(QLatin1String("RAR 4"))) {
                emit compressionMethodFound(QStringLiteral("RAR4"));
            } else if (line.contains(QLatin1String("RAR 5"))) {
                emit compressionMethodFound(QStringLiteral("RAR5"));
                m_isRAR5 = true;
            }

            if (line.contains(QLatin1String("lock"))) {
                m_isLocked = true;
            }
        }
        break;
    // Parses the entry details for each entry.
    case ParseStateEntryDetails:
        // For multi-volume archives there is a header between the entries in
        // each volume.
        if (line.startsWith(QLatin1String("Archive: "))) {
            m_parseState = ParseStateHeader;
            return true;
            // Empty line indicates end of entry.
        } else if (line.trimmed().isEmpty() && !m_unrar5Details.isEmpty()) {
//            handleUnrar5Entry();
            emitEntryForIndex(m_fileStat);
        } else {
            // All detail lines should contain a colon.
            if (!line.contains(QLatin1Char(':'))) {
                return true;
            }

//            bool isLink = false;
//            QString linkTarget;
//            bool isDircetory = false;
            // The details are on separate lines, so we store them in the QHash
            // m_unrar5Details.
            m_unrar5Details.insert(line.section(QLatin1Char(':'), 0, 0).trimmed().toLower(), line.section(QLatin1Char(':'), 1).trimmed());
            /*if (line.section(QLatin1Char(':'), 0, 0).trimmed().toLower() == "attributes") {
                m_fileStat.archive_permissions = line.section(QLatin1Char(':'), 1).trimmed();
                if (m_fileStat.archive_permissions.startsWith("l")) {
                    isLink = true;
                }
            } else if (line.section(QLatin1Char(':'), 0, 0).trimmed().toLower() == "packed size") {
                m_fileStat.archive_compressedSize = line.section(QLatin1Char(':'), 1).trimmed().toUInt();
            } else if (line.section(QLatin1Char(':'), 0, 0).trimmed().toLower() == "mtime") {
                QString time = line.section(QLatin1Char(':'), 1).trimmed();
                m_fileStat.archive_timestamp = QDateTime::fromString(time.left((time.length() - 10)), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
            } else if (line.section(QLatin1Char(':'), 0, 0).trimmed().toLower() == "name") {
                m_fileStat.archive_fullPath = line.section(QLatin1Char(':'), 1).trimmed();
                m_fileStat.archive_name = line.section(QLatin1Char(':'), 1).trimmed();
            } else if (line.section(QLatin1Char(':'), 0, 0).trimmed().toLower() == "size") {
                m_fileStat.archive_size = line.section(QLatin1Char(':'), 1).trimmed().toInt();
            } else if (line.section(QLatin1Char(':'), 0, 0).trimmed().toLower() == "type") {
                if (line.section(QLatin1Char(':'), 1).trimmed() == "File") {
                    m_fileStat.archive_isDirectory = false;
                } else if (line.section(QLatin1Char(':'), 1).trimmed().contains("link")) {
                    isLink = true;
                } else {
                    isDircetory = true;
                    m_fileStat.archive_isDirectory = true;
                }
            } else if (line.section(QLatin1Char(':'), 0, 0).trimmed().toLower() == "ratio") {
                QString compressionRatio = line.section(QLatin1Char(':'), 1).trimmed();
                compressionRatio.chop(1); // Remove the '%'
                m_fileStat.archive_ratio = compressionRatio;
            } else if (line.section(QLatin1Char(':'), 0, 0).trimmed().toLower() == "compression") {
                QString compression = line.section(QLatin1Char(':'), 1).trimmed();
                int optionPos = compression.indexOf(QLatin1Char('-'));
                if (optionPos != -1) {
                    m_fileStat.archive_method = compression.mid(optionPos);
                    m_fileStat.archive_version = compression.left(optionPos).trimmed();
                } else {
                    // No method specified.
                    m_fileStat.archive_method = QString();
                    m_fileStat.archive_version = compression;
                }
            } else if (line.section(QLatin1Char(':'), 0, 0).trimmed().toLower().contains("crc32")) {
                m_fileStat.archive_CRC = line.section(QLatin1Char(':'), 1).trimmed();
            } else if (line.section(QLatin1Char(':'), 0, 0).trimmed().toLower() == "blake2") {
                m_fileStat.archive_BLAKE2 = line.section(QLatin1Char(':'), 1).trimmed();
            } else if (line.section(QLatin1Char(':'), 0, 0).trimmed().toLower() == "flags") {
                m_fileStat.archive_isPasswordProtected = line.section(QLatin1Char(':'), 1).trimmed().contains("encrypted");
            } else if (line.section(QLatin1Char(':'), 0, 0).trimmed().toLower() == "target") {
                linkTarget = line.section(QLatin1Char(':'), 1).trimmed();
            }

            if (isDircetory && (!m_fileStat.archive_fullPath.endsWith(QLatin1Char('/')))) {
                m_fileStat.archive_fullPath += QLatin1Char('/');
                m_fileStat.archive_name += QLatin1Char('/');
                isDircetory = false;
            }

            if (isLink) {
                m_fileStat.archive_link = linkTarget;
                isLink = false;
                linkTarget = "";
            }*/
        }

        break;
    default:
        break;
    }

    return true;
}

void CliRarPlugin::handleUnrar5Entry()
{
    Archive::Entry *e = new Archive::Entry(this);

    QString compressionRatio = m_unrar5Details.value(QStringLiteral("ratio"));
    compressionRatio.chop(1); // Remove the '%'
    e->setProperty("ratio", compressionRatio);

    QString dateTime = m_unrar5Details.value(QStringLiteral("mtime"));
    QDateTime ts = QDateTime::fromString(dateTime.left((dateTime.length() - 10)), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    e->setProperty("timestamp", ts);

    bool isDirectory = (m_unrar5Details.value(QStringLiteral("type")) == QLatin1String("Directory"));
    e->setProperty("isDirectory", isDirectory);

    if (isDirectory && !m_unrar5Details.value(QStringLiteral("name")).endsWith(QLatin1Char('/'))) {
        m_unrar5Details[QStringLiteral("name")] += QLatin1Char('/');
    }

    QString compression = m_unrar5Details.value(QStringLiteral("compression"));
    int optionPos = compression.indexOf(QLatin1Char('-'));
    if (optionPos != -1) {
        e->setProperty("method", compression.mid(optionPos));
        e->setProperty("version", compression.left(optionPos).trimmed());
    } else {
        // No method specified.
        e->setProperty("method", QString());
        e->setProperty("version", compression);
    }

    m_isPasswordProtected = m_unrar5Details.value(QStringLiteral("flags")).contains(QStringLiteral("encrypted"));
    e->setProperty("isPasswordProtected", m_isPasswordProtected);
    if (m_isPasswordProtected) {
        m_isRAR5 ? emit encryptionMethodFound(QStringLiteral("AES256")) : emit encryptionMethodFound(QStringLiteral("AES128"));
    }

    e->setProperty("fullPath", m_unrar5Details.value(QStringLiteral("name")));
    e->setProperty("size", m_unrar5Details.value(QStringLiteral("size")));
    e->setProperty("compressedSize", m_unrar5Details.value(QStringLiteral("packed size")));
    e->setProperty("permissions", m_unrar5Details.value(QStringLiteral("attributes")));
    e->setProperty("CRC", m_unrar5Details.value(QStringLiteral("crc32")));
    e->setProperty("BLAKE2", m_unrar5Details.value(QStringLiteral("blake2")));

    if (e->property("permissions").toString().startsWith(QLatin1Char('l'))) {
        e->setProperty("link", m_unrar5Details.value(QStringLiteral("target")));
    }

    m_unrar5Details.clear();
    emit entry(e);
}

bool CliRarPlugin::handleUnrar4Line(const QString &line)
{
    if (line.startsWith(QLatin1String("Cannot find volume "))) {
        emit error(tr("Failed to find all archive volumes."));
        return false;
    }

    // RegExp matching end of comment field.
    // FIXME: Comment itself could also contain the Archive path string here.
    QRegularExpression rxCommentEnd(QStringLiteral("^(Solid archive|Archive|Volume) .+$"));

    // Three types of subHeaders can be displayed for unrar 3 and 4.
    // STM has 4 lines, RR has 3, and CMT has lines corresponding to
    // length of comment field +3. We ignore the subheaders.
    QRegularExpression rxSubHeader(QStringLiteral("^Data header type: (CMT|STM|RR)$"));
    QRegularExpressionMatch matchSubHeader;

    switch (m_parseState) {

    // Parses the comment field.
    case ParseStateComment:
        // unrar 4 outputs the following string when opening v5 RAR archives.
        if (line == QLatin1String("Unsupported archive format. Please update RAR to a newer version.")) {
            emit error(tr("Your unrar executable is version %1, which is too old to handle this archive. Please update to a more recent version."));
            return false;
        }

        // unrar 3 reports a non-RAR archive when opening v5 RAR archives.
        if (line.endsWith(QLatin1String(" is not RAR archive"))) {
            emit error(tr("Unrar reported a non-RAR archive. The installed unrar version (%1) is old. Try updating your unrar."));
            return false;
        }

        // If we reach this point, then we can be sure that it's not a RAR5
        // archive, so assume RAR4.
        emit compressionMethodFound(QStringLiteral("RAR4"));

        if (rxCommentEnd.match(line).hasMatch()) {

            if (line.startsWith(QLatin1String("Volume "))) {
                m_numberOfVolumes++;
                if (!isMultiVolume()) {
                    setMultiVolume(true);
                }
            }
            if (line.startsWith(QLatin1String("Solid archive")) && !m_isSolid) {
                m_isSolid = true;
            }

            m_parseState = ParseStateHeader;
            m_comment = m_comment.trimmed();
            m_linesComment = m_comment.count(QLatin1Char('\n')) + 1;
            if (!m_comment.isEmpty()) {
            }

        } else {
            m_comment.append(line + QLatin1Char('\n'));
        }

        break;

    // Parses the header, which is whatever is between the comment field
    // and the entries.
    case ParseStateHeader:
        // Horizontal line indicates end of header.
        if (line.startsWith(QLatin1String("--------------------"))) {
            m_parseState = ParseStateEntryFileName;
        } else if (line.startsWith(QLatin1String("Volume "))) {
            m_numberOfVolumes++;
        } else if (line == QLatin1String("Lock is present")) {
            m_isLocked = true;
        }
        break;

    // Parses the entry name, which is on the first line of each entry.
    case ParseStateEntryFileName:
        // Ignore empty lines.
        if (line.trimmed().isEmpty()) {
            return true;
        }

        matchSubHeader = rxSubHeader.match(line);

        if (matchSubHeader.hasMatch()) {
            if (matchSubHeader.captured(1) == QLatin1String("STM")) {
                ignoreLines(4, ParseStateEntryFileName);
            } else if (matchSubHeader.captured(1) == QLatin1String("CMT")) {
                ignoreLines(m_linesComment + 3, ParseStateEntryFileName);
            } else if (matchSubHeader.captured(1) == QLatin1String("RR")) {
                ignoreLines(3, ParseStateEntryFileName);
            }
            return true;
        }

        // The entries list ends with a horizontal line, followed by a
        // single summary line or, for multi-volume archives, another header.
        if (line.startsWith(QLatin1String("-----------------"))) {
            m_parseState = ParseStateHeader;
            return true;
            // Encrypted files are marked with an asterisk.
        } else if (line.startsWith(QLatin1Char('*'))) {
            m_isPasswordProtected = true;
            if (!m_isEncrypted && m_isPasswordProtected) { //若list时发现有文件加密，则通知loadjob该压缩包是加密
                emit sigIsEncrypted();
                m_isEncrypted = true;
            }
            m_unrar4Details.append(QString(line.trimmed()).remove(0, 1)); //Remove the asterisk
            emit encryptionMethodFound(QStringLiteral("AES128"));
            // Entry names always start at the second position, so a line not
            // starting with a space is not an entry name.
        } else if (!line.startsWith(QLatin1Char(' '))) {
            return true;
            // If we reach this, then we can assume the line is an entry name, so
            // save it, and move on to the rest of the entry details.
        } else {
            m_unrar4Details.append(line.trimmed());
        }

        m_parseState = ParseStateEntryDetails;
        break;
    // Parses the remainder of the entry details for each entry.
    case ParseStateEntryDetails:
        // If the line following an entry name is empty, we did something
        // wrong.
        Q_ASSERT(!line.trimmed().isEmpty());

        // If we reach a horizontal line, then the previous line was not an
        // entry name, so go back to header.
        if (line.startsWith(QLatin1String("-----------------"))) {
            m_parseState = ParseStateHeader;
            return true;
        }

        // In unrar 3 and 4 the details are on a single line, so we
        // pass a QStringList containing the details. We need to store
        // it due to symlinks (see below).
        m_unrar4Details.append(line.split(QLatin1Char(' '), QString::SkipEmptyParts));

        // The details line contains 9 fields, so m_unrar4Details
        // should now contain 9 + the filename = 10 strings. If not, this is
        // not an archive entry.
        if (m_unrar4Details.size() != 10) {
            m_parseState = ParseStateHeader;
            return true;
        }

        // When unrar 3 and 4 list a symlink, they output an extra line
        // containing the link target. The extra line is output after
        // the line we ignore, so we first need to ignore one line.
        if (m_unrar4Details.at(6).startsWith(QLatin1Char('l'))) {
            ignoreLines(1, ParseStateLinkTarget);
            return true;
        } else {
            handleUnrar4Entry();
        }

        // Unrar 3 & 4 show a third line for each entry, which contains
        // three details: Host OS, Solid, and Old. We can ignore this
        // line.
        ignoreLines(1, ParseStateEntryFileName);

        break;
    // Parses a symlink target.
    case ParseStateLinkTarget:
        m_unrar4Details.append(QString(line).remove(QStringLiteral("-->")).trimmed());
        handleUnrar4Entry();

        m_parseState = ParseStateEntryFileName;
        break;
    default:
        break;
    }

    return true;
}

void CliRarPlugin::handleUnrar4Entry()
{
    Archive::Entry *e = new Archive::Entry(this);

    QDateTime ts = QDateTime::fromString(QString(m_unrar4Details.at(4) + QLatin1Char(' ') + m_unrar4Details.at(5)), QStringLiteral("dd-MM-yy hh:mm"));
    // Unrar 3 & 4 output dates with a 2-digit year but QDateTime takes it as
    // 19??. Let's take 1950 as cut-off; similar to KDateTime.
    if (ts.date().year() < 1950) {
        ts = ts.addYears(100);
    }
    e->setProperty("timestamp", ts);

    bool isDirectory = ((m_unrar4Details.at(6).at(0) == QLatin1Char('d')) ||
                        (m_unrar4Details.at(6).at(1) == QLatin1Char('D')));
    e->setProperty("isDirectory", isDirectory);

    if (isDirectory && !m_unrar4Details.at(0).endsWith(QLatin1Char('/'))) {
        m_unrar4Details[0] += QLatin1Char('/');
    }

    // Unrar reports the ratio as ((compressed size * 100) / size);
    // we consider ratio as (100 * ((size - compressed size) / size)).
    // If the archive is a multivolume archive, a string indicating
    // whether the archive's position in the volume is displayed
    // instead of the compression ratio.
    QString compressionRatio = m_unrar4Details.at(3);
    if ((compressionRatio == QStringLiteral("<--")) ||
            (compressionRatio == QStringLiteral("<->")) ||
            (compressionRatio == QStringLiteral("-->"))) {
        compressionRatio = QLatin1Char('0');
    } else {
        compressionRatio.chop(1); // Remove the '%'
    }
    e->setProperty("ratio", compressionRatio);

    // TODO:
    // - Permissions differ depending on the system the entry was added
    //   to the archive.
    e->setProperty("fullPath", m_unrar4Details.at(0));
    e->setProperty("size", m_unrar4Details.at(1));
    e->setProperty("compressedSize", m_unrar4Details.at(2));
    e->setProperty("permissions", m_unrar4Details.at(6));
    e->setProperty("CRC", m_unrar4Details.at(7));
    e->setProperty("method", m_unrar4Details.at(8));
    e->setProperty("version", m_unrar4Details.at(9));
    e->setProperty("isPasswordProtected", m_isPasswordProtected);

    if (e->property("permissions").toString().startsWith(QLatin1Char('l'))) {
        e->setProperty("link", m_unrar4Details.at(10));
    }

    m_unrar4Details.clear();
    emit entry(e);
}

bool CliRarPlugin::readExtractLine(const QString &line)
{
    const QRegularExpression rxCRC(QStringLiteral("CRC failed"));
    if (rxCRC.match(line).hasMatch()) {
        emit error(tr("One or more wrong checksums"));
        return false;
    }

    if (line.startsWith(QLatin1String("Cannot find volume "))) {
        emit error(tr("Failed to find all archive volumes."));
        return false;
    }

    return true;
}

bool CliRarPlugin::hasBatchExtractionProgress() const
{
    return true;
}

void CliRarPlugin::ignoreLines(int lines, ParseState nextState)
{
    m_remainingIgnoreLines = lines;
    m_parseState = nextState;
}

bool CliRarPlugin::emitEntryForIndex(ReadOnlyArchiveInterface::archive_stat &archive)
{
    QString compressionRatio = m_unrar5Details.value(QStringLiteral("ratio"));
    compressionRatio.chop(1); // Remove the '%'
    m_fileStat.archive_ratio = compressionRatio;

    QString dateTime = m_unrar5Details.value(QStringLiteral("mtime"));
    QDateTime ts = QDateTime::fromString(dateTime.left((dateTime.length() - 10)), QStringLiteral("yyyy-MM-dd HH:mm:ss"));
    m_fileStat.archive_timestamp = ts;

    bool isDirectory = (m_unrar5Details.value(QStringLiteral("type")) == QLatin1String("Directory"));
    m_fileStat.archive_isDirectory = isDirectory;
    if (isDirectory && !m_unrar5Details.value(QStringLiteral("name")).endsWith(QLatin1Char('/'))) {
        m_unrar5Details[QStringLiteral("name")] += QLatin1Char('/');
    }

    QString compression = m_unrar5Details.value(QStringLiteral("compression"));
    int optionPos = compression.indexOf(QLatin1Char('-'));
    if (optionPos != -1) {
        m_fileStat.archive_method = compression.mid(optionPos);
        m_fileStat.archive_version = compression.left(optionPos).trimmed();
    } else {
        // No method specified.
        m_fileStat.archive_method = QString();
        m_fileStat.archive_version = compression;
    }

    m_isPasswordProtected = m_unrar5Details.value(QStringLiteral("flags")).contains(QStringLiteral("encrypted"));
    m_fileStat.archive_isPasswordProtected = m_isPasswordProtected;
    if (m_isPasswordProtected) {
        m_isRAR5 ? emit encryptionMethodFound(QStringLiteral("AES256")) : emit encryptionMethodFound(QStringLiteral("AES128"));
    }
    if (!m_isEncrypted && m_isPasswordProtected) { //若list时发现有文件加密，则通知loadjob该压缩包是加密
        emit sigIsEncrypted();
        m_isEncrypted = true;
    }

    m_fileStat.archive_fullPath = m_unrar5Details.value(QStringLiteral("name"));
    m_fileStat.archive_size = m_unrar5Details.value(QStringLiteral("size")).toInt();
    m_fileStat.archive_compressedSize = m_unrar5Details.value(QStringLiteral("packed size")).toULongLong();
    m_fileStat.archive_permissions = m_unrar5Details.value(QStringLiteral("attributes"));
    m_fileStat.archive_CRC = m_unrar5Details.value(QStringLiteral("crc32"));
    m_fileStat.archive_BLAKE2 = m_unrar5Details.value(QStringLiteral("blake2"));

    if (m_fileStat.archive_permissions.startsWith(QLatin1Char('l'))) {
        m_fileStat.archive_link = m_unrar5Details.value(QStringLiteral("target"));
    }

    m_unrar5Details.clear();

    setEntryVal(archive);
    if (m_listMap.find(archive.archive_fullPath) == m_listMap.end()) {
        m_listMap.insert(archive.archive_fullPath, archive);
    }

    return true;
}

void CliRarPlugin::setEntryVal(ReadOnlyArchiveInterface::archive_stat &archive)
{
    if ((archive.archive_fullPath.endsWith("/") && archive.archive_fullPath.count("/") == 1) || (archive.archive_fullPath.count("/") == 0)) {
        setEntryData(archive);
    }
}

void CliRarPlugin::setEntryData(ReadOnlyArchiveInterface::archive_stat &archive, bool isMutilFolderFile)
{
    Archive::Entry *e = new Archive::Entry(this);

    e->setProperty("ratio", m_fileStat.archive_ratio);
    e->setProperty("timestamp", m_fileStat.archive_timestamp);
    e->setProperty("isDirectory", m_fileStat.archive_isDirectory);
    e->setProperty("method", m_fileStat.archive_method);
    e->setProperty("version", m_fileStat.archive_version);
    e->setProperty("isPasswordProtected", m_fileStat.archive_isPasswordProtected);
    if (m_fileStat.archive_isPasswordProtected) {
        m_isRAR5 ? emit encryptionMethodFound(QStringLiteral("AES256")) : emit encryptionMethodFound(QStringLiteral("AES128"));
    }

    e->setProperty("fullPath", m_fileStat.archive_fullPath);
    if (!isMutilFolderFile) {
        e->setProperty("size", archive.archive_size);
    } else {
        e->setProperty("size", 0);
    }

    e->setProperty("compressedSize", m_fileStat.archive_compressedSize);
    e->setProperty("permissions", m_fileStat.archive_permissions);
    e->setProperty("CRC", m_fileStat.archive_CRC);
    e->setProperty("BLAKE2", m_fileStat.archive_BLAKE2);
    e->setProperty("link", m_fileStat.archive_link);

//    m_unrar5Details.clear();
    emit entry(e);
}

Archive::Entry *CliRarPlugin::setEntryDataA(ReadOnlyArchiveInterface::archive_stat &archive)
{
    Archive::Entry *e = new Archive::Entry(this);

    e->setProperty("ratio", archive.archive_ratio);
    e->setProperty("timestamp", archive.archive_timestamp);
    e->setProperty("isDirectory", archive.archive_isDirectory);
    e->setProperty("method", archive.archive_method);
    e->setProperty("version", archive.archive_version);
    e->setProperty("isPasswordProtected", archive.archive_isPasswordProtected);
    e->setProperty("fullPath", archive.archive_fullPath);
    e->setProperty("size", archive.archive_size);
    e->setProperty("compressedSize", archive.archive_compressedSize);
    e->setProperty("permissions", archive.archive_permissions);
    e->setProperty("CRC", archive.archive_CRC);
    e->setProperty("BLAKE2", archive.archive_BLAKE2);
    e->setProperty("link", archive.archive_link);

    return e;
}

qint64 CliRarPlugin::extractSize(const QVector<Archive::Entry *> &files)
{
    m_listFileName.clear();

    qint64 qExtractSize = 0;
    for (Archive::Entry *e : files) {
        QString strPath = e->fullPath();
        m_strRootNode = e->rootNode;
        auto iter = m_listMap.find(strPath);
        for (; iter != m_listMap.end();) {
            if (!iter.key().startsWith(strPath)) {
                break;
            } else {
                if (!iter.key().endsWith("/")) {
                    qExtractSize += iter.value().archive_size;
                }

                m_listFileName << iter.value().archive_fullPath;
                ++iter;
                if (!strPath.endsWith(QDir::separator())) {
                    break;
                }
            }
        }
    }

    return qExtractSize;
}

bool CliRarPlugin::isPasswordPrompt(const QString &line)
{
    return line.startsWith(QLatin1String("Enter password (will not be echoed) for"));
}

bool CliRarPlugin::isWrongPasswordMsg(const QString &line)
{
    return (line.contains(QLatin1String("password incorrect")) || line.contains(QLatin1String("wrong password")) || line.contains(QLatin1String("The specified password is incorrect")));
}

bool CliRarPlugin::isCorruptArchiveMsg(const QString &line)
{
    return (line == QLatin1String("Unexpected end of archive") ||
            line.contains(QLatin1String("the file header is corrupt")) ||
            line.endsWith(QLatin1String("checksum error")));
}

bool CliRarPlugin::isDiskFullMsg(const QString &line)
{
    return line.contains(QLatin1String("No space left on device"));
}

bool CliRarPlugin::isFileExistsMsg(const QString &line)
{
    return (line == QLatin1String("[Y]es, [N]o, [A]ll, n[E]ver, [R]ename, [Q]uit "));
}

bool CliRarPlugin::isFileExistsFileName(const QString &line)
{
    return (line.startsWith(QLatin1String("Would you like to replace the existing file ")) || // unrar 5
            line.contains(QLatin1String(" already exists. Overwrite it"))); // unrar 3 & 4
}

bool CliRarPlugin::isLocked() const
{
    return m_isLocked;
}

void CliRarPlugin::showEntryListFirstLevel(const QString &directory)
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

void CliRarPlugin::RefreshEntryFileCount(Archive::Entry *file)
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

#include "moc_cliplugin.cpp"

