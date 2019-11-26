#include "libzipplugin.h"
//#include "kpluginfactory.h"


//#include <KIO/Global>
//#include <KLocalizedString>


#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <qplatformdefs.h>
#include <QThread>
#include <QTextCodec>
#include <KEncodingProber>
#include <utime.h>
#include <zlib.h>
#include <memory>
#include <QMimeDatabase>
#include <QRegularExpression>
#include "queries.h"

//K_PLUGIN_CLASS_WITH_JSON(LibzipPlugin, "kerfuffle_libzip.json")

static float codecConfidenceForData(const QTextCodec *codec, const QByteArray &data, const QLocale::Country &country)
{
    qreal hep_count = 0;
    int non_base_latin_count = 0;
    qreal unidentification_count = 0;
    int replacement_count = 0;

    QTextDecoder decoder(codec);
    const QString &unicode_data = decoder.toUnicode(data);

    for (int i = 0; i < unicode_data.size(); ++i) {
        const QChar &ch = unicode_data.at(i);

        if (ch.unicode() > 0x7f)
            ++non_base_latin_count;

        switch (ch.script()) {
        case QChar::Script_Hiragana:
        case QChar::Script_Katakana:
            hep_count += (country == QLocale::Japan) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::Japan) ? 0 : 0.3;
            break;
        case QChar::Script_Han:
            hep_count += (country == QLocale::China) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::China) ? 0 : 0.3;
            break;
        case QChar::Script_Hangul:
            hep_count += (country == QLocale::NorthKorea) || (country == QLocale::SouthKorea) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::NorthKorea) || (country == QLocale::SouthKorea) ? 0 : 0.3;
            break;
        case QChar::Script_Cyrillic:
            hep_count += (country == QLocale::Russia) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::Russia) ? 0 : 0.3;
            break;
        case QChar::Script_Devanagari:
            hep_count += (country == QLocale::Nepal || country == QLocale::India) ? 1.2 : 0.5;
            unidentification_count += (country == QLocale::Nepal || country == QLocale::India) ? 0 : 0.3;
            break;
        default:
            // full-width character, emoji, 常用标点, 拉丁文补充1，天城文补充，CJK符号和标点符号（如：【】）
            if ((ch.unicode() >= 0xff00 && ch <= 0xffef)
                    || (ch.unicode() >= 0x2600 && ch.unicode() <= 0x27ff)
                    || (ch.unicode() >= 0x2000 && ch.unicode() <= 0x206f)
                    || (ch.unicode() >= 0x80 && ch.unicode() <= 0xff)
                    || (ch.unicode() >= 0xa8e0 && ch.unicode() <= 0xa8ff)
                    || (ch.unicode() >= 0x0900 && ch.unicode() <= 0x097f)
                    || (ch.unicode() >= 0x3000 && ch.unicode() <= 0x303f)) {
                ++hep_count;
            } else if (ch.isSurrogate() && ch.isHighSurrogate()) {
                ++i;

                if (i < unicode_data.size()) {
                    const QChar &next_ch = unicode_data.at(i);

                    if (!next_ch.isLowSurrogate()) {
                        --i;
                        break;
                    }

                    uint unicode = QChar::surrogateToUcs4(ch, next_ch);

                    // emoji
                    if (unicode >= 0x1f000 && unicode <= 0x1f6ff) {
                        hep_count += 2;
                    }
                }
            } else if (ch.unicode() == QChar::ReplacementCharacter) {
                ++replacement_count;
            } else if (ch.unicode() > 0x7f) {
                // 因为UTF-8编码的容错性很低，所以未识别的编码只需要判断是否为 QChar::ReplacementCharacter 就能排除
                if (codec->name() != "UTF-8")
                    ++unidentification_count;
            }
            break;
        }
    }

    float c = qreal(hep_count) / non_base_latin_count / 1.2;

    c -= qreal(replacement_count) / non_base_latin_count;
    c -= qreal(unidentification_count) / non_base_latin_count;

    return qMax(0.0f, c);
}

LibzipPluginFactory::LibzipPluginFactory()
{
    registerPlugin<LibzipPlugin>();
}
LibzipPluginFactory::~LibzipPluginFactory()
{

}

void LibzipPlugin::progressCallback(zip_t *, double progress, void *that)
{
    static_cast<LibzipPlugin *>(that)->emitProgress(progress);
}

LibzipPlugin::LibzipPlugin(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
    , m_overwriteAll(false)
    , m_skipAll(false)
    , m_listAfterAdd(false)
{
}

LibzipPlugin::~LibzipPlugin()
{
    for (const auto e : qAsConst(m_emittedEntries)) {
        // Entries might be passed to pending slots, so we just schedule their deletion.
        e->deleteLater();
    }
}

bool LibzipPlugin::list(bool isbatch)
{
    m_numberOfEntries = 0;

    int errcode = 0;
    zip_error_t err;

    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(filename()).constData(), ZIP_RDONLY, &errcode);
    zip_error_init_with_code(&err, errcode);
    if (!archive) {
        emit error(tr("Failed to open archive: %1"));
        return false;
    }

    // Fetch archive comment.
    m_comment = QString::fromLocal8Bit(zip_get_archive_comment(archive, nullptr, ZIP_FL_ENC_RAW));

    // Get number of archive entries.
    const auto nofEntries = zip_get_num_entries(archive, 0);

    detectAllfile(archive, nofEntries);
    // Loop through all archive entries.
    for (int i = 0; i < nofEntries; i++) {

        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        emitEntryForIndex(archive, i);
        emit progress(float(i + 1) / nofEntries);
    }

    zip_close(archive);
    m_listAfterAdd = false;
    return true;
}

void LibzipPlugin::detectAllfile(zip_t *archive, int num)
{
    m_codecname.clear();
    QByteArrayList codeclist;

    // Loop through all archive entries.
    for (int i = 0; i < num; i++) {
        zip_stat_t statBuffer;
        if (zip_stat_index(archive, i, ZIP_FL_ENC_RAW, &statBuffer)) {
            return;
        }

        if (statBuffer.valid & ZIP_STAT_NAME) {
            QByteArray codec = detectEncode(statBuffer.name);
            codeclist.append(codec);
        }
    }
    if (codeclist.count() > 1) {
        QMap<QByteArray, int> codec_map;
        QByteArray max_codec;

        foreach (QByteArray codec, codeclist) {

            if (!codec_map.contains(codec)) {
                int count = codeclist.count(codec);
                codec_map.insert(codec, count);

                if (codec_map.value(max_codec) < count) {
                    max_codec = codec;
                }
            }
        }

        m_codecname = max_codec;
    }
}

QString  LibzipPlugin::trans2uft8(const char *str)
{
    QByteArray codec_name = detectEncode(str);
    qDebug() << codec_name;
    if ("" == m_codecname) {
        QTextCodec *codec = QTextCodec::codecForName(codec_name);
        m_codecstr = codec_name;
        return codec->toUnicode(str);
    } else if ("gb18030" == codec_name) {
        QTextCodec *codec = QTextCodec::codecForName(codec_name);
        m_codecstr = codec_name;
        return codec->toUnicode(str);
    } else if ("UTF-8" != codec_name) {
        QTextCodec *codec = QTextCodec::codecForName(m_codecname);
        m_codecstr = m_codecname;
        return codec->toUnicode(str);
    } else {
        m_codecstr = "UTF-8";
        return QString(str);
    }
}

bool LibzipPlugin::addFiles(const QVector<Archive::Entry *> &files, const Archive::Entry *destination, const CompressionOptions &options, uint numberOfEntriesToAdd)
{
    Q_UNUSED(numberOfEntriesToAdd)
    int errcode = 0;
    zip_error_t err;

    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(filename()).constData(), ZIP_CREATE, &errcode);
    zip_error_init_with_code(&err, errcode);
    if (!archive) {
        emit error(tr("Failed to open archive: %1"));
        return false;
    }

    uint i = 0;
    for (const Archive::Entry *e : files) {

        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        // If entry is a directory, traverse and add all its files and subfolders.
        if (QFileInfo(e->fullPath()).isDir()) {

            if (!writeEntry(archive, e->fullPath(), destination, options, true)) {
                return false;
            }

            QDirIterator it(e->fullPath(),
                            QDir::AllEntries | QDir::Readable |
                            QDir::Hidden | QDir::NoDotAndDotDot,
                            QDirIterator::Subdirectories);

            while (!QThread::currentThread()->isInterruptionRequested() && it.hasNext()) {
                const QString path = it.next();

                if (QFileInfo(path).isDir()) {
                    if (!writeEntry(archive, path, destination, options, true)) {
                        return false;
                    }
                } else {
                    if (!writeEntry(archive, path, destination, options)) {
                        return false;
                    }
                }
                i++;
            }
        } else {
            if (!writeEntry(archive, e->fullPath(), destination, options)) {
                return false;
            }
        }
        i++;

    }
    m_filesize = i;
    m_addarchive = archive;
    // Register the callback function to get progress feedback.
    zip_register_progress_callback_with_state(archive, 0.001, progressCallback, nullptr, this);

    if (zip_close(archive)) {
        emit error(tr("Failed to write archive."));
        return false;
    }

    // We list the entire archive after adding files to ensure entry
    // properties are up-to-date.
    m_listAfterAdd = true;
//    list();

    return true;
}

void LibzipPlugin::emitProgress(double percentage)
{
    int i = m_filesize * percentage;
    if (m_addarchive) {
        emit progress_filename(trans2uft8(zip_get_name(m_addarchive, i, ZIP_FL_ENC_RAW)));
    }

    // Go from 0 to 50%. The second half is the subsequent listing.
    emit progress(percentage);
}

bool LibzipPlugin::writeEntry(zip_t *archive, const QString &file, const Archive::Entry *destination, const CompressionOptions &options, bool isDir)
{
    Q_ASSERT(archive);

    QByteArray destFile;
    if (destination) {
        destFile = QString(destination->fullPath() + file).toUtf8();
    } else {
        destFile = file.toUtf8();
    }

    qlonglong index;
    if (isDir) {
        index = zip_dir_add(archive, destFile.constData(), ZIP_FL_ENC_GUESS);
        if (index == -1) {
            // If directory already exists in archive, we get an error.
            return true;
        }
    } else {
        zip_source_t *src = zip_source_file(archive, QFile::encodeName(file).constData(), 0, -1);
        Q_ASSERT(src);

        index = zip_file_add(archive, destFile.constData(), src, ZIP_FL_ENC_GUESS | ZIP_FL_OVERWRITE);
        if (index == -1) {
            zip_source_free(src);
            emit error(tr("Failed to add entry: %1"));
            return false;
        }
    }

#ifndef Q_OS_WIN
    // Set permissions.
    QT_STATBUF result;
    if (QT_STAT(QFile::encodeName(file).constData(), &result) != 0) {
    } else {
        zip_uint32_t attributes = result.st_mode << 16;
        if (zip_file_set_external_attributes(archive, index, ZIP_FL_UNCHANGED, ZIP_OPSYS_UNIX, attributes) != 0) {
        }
    }
#endif

    if (!password().isEmpty()) {
        Q_ASSERT(!options.encryptionMethod().isEmpty());
        if (options.encryptionMethod() == QLatin1String("AES128")) {
            zip_file_set_encryption(archive, index, ZIP_EM_AES_128, password().toUtf8().constData());
        } else if (options.encryptionMethod() == QLatin1String("AES192")) {
            zip_file_set_encryption(archive, index, ZIP_EM_AES_192, password().toUtf8().constData());
        } else if (options.encryptionMethod() == QLatin1String("AES256")) {
            zip_file_set_encryption(archive, index, ZIP_EM_AES_256, password().toUtf8().constData());
        }
    }

    // Set compression level and method.
    zip_int32_t compMethod = ZIP_CM_DEFAULT;
    if (!options.compressionMethod().isEmpty()) {
        if (options.compressionMethod() == QLatin1String("Deflate")) {
            compMethod = ZIP_CM_DEFLATE;
        } else if (options.compressionMethod() == QLatin1String("BZip2")) {
            compMethod = ZIP_CM_BZIP2;
        } else if (options.compressionMethod() == QLatin1String("Store")) {
            compMethod = ZIP_CM_STORE;
        }
    }
    const int compLevel = options.isCompressionLevelSet() ? options.compressionLevel() : 6;
    if (zip_set_file_compression(archive, index, compMethod, compLevel) != 0) {
        emit error(tr("Failed to set compression options for entry: %1"));
        return false;
    }

    return true;
}

bool LibzipPlugin::emitEntryForIndex(zip_t *archive, qlonglong index)
{
    Q_ASSERT(archive);

    zip_stat_t statBuffer;
    if (zip_stat_index(archive, index, ZIP_FL_ENC_RAW, &statBuffer)) {
        return false;
    }

    auto e = new Archive::Entry();

    if (statBuffer.valid & ZIP_STAT_NAME) {
        e->setFullPath(trans2uft8(statBuffer.name));
    }

    if (e->fullPath(PathFormat::WithTrailingSlash).endsWith(QDir::separator())) {
        e->setProperty("isDirectory", true);
    }

    if (statBuffer.valid & ZIP_STAT_MTIME) {
        e->setProperty("timestamp", QDateTime::fromTime_t(statBuffer.mtime));
    }
    if (statBuffer.valid & ZIP_STAT_SIZE) {
        e->setProperty("size", (qulonglong)statBuffer.size);
    }
    if (statBuffer.valid & ZIP_STAT_COMP_SIZE) {
        e->setProperty("compressedSize", (qlonglong)statBuffer.comp_size);
    }
    if (statBuffer.valid & ZIP_STAT_CRC) {
        if (!e->isDir()) {
            e->setProperty("CRC", QString::number((qulonglong)statBuffer.crc, 16).toUpper());
        }
    }
    if (statBuffer.valid & ZIP_STAT_COMP_METHOD) {
        switch (statBuffer.comp_method) {
        case ZIP_CM_STORE:
            e->setProperty("method", QStringLiteral("Store"));
            emit compressionMethodFound(QStringLiteral("Store"));
            break;
        case ZIP_CM_DEFLATE:
            e->setProperty("method", QStringLiteral("Deflate"));
            emit compressionMethodFound(QStringLiteral("Deflate"));
            break;
        case ZIP_CM_DEFLATE64:
            e->setProperty("method", QStringLiteral("Deflate64"));
            emit compressionMethodFound(QStringLiteral("Deflate64"));
            break;
        case ZIP_CM_BZIP2:
            e->setProperty("method", QStringLiteral("BZip2"));
            emit compressionMethodFound(QStringLiteral("BZip2"));
            break;
        case ZIP_CM_LZMA:
            e->setProperty("method", QStringLiteral("LZMA"));
            emit compressionMethodFound(QStringLiteral("LZMA"));
            break;
        case ZIP_CM_XZ:
            e->setProperty("method", QStringLiteral("XZ"));
            emit compressionMethodFound(QStringLiteral("XZ"));
            break;
        }
    }
    if (statBuffer.valid & ZIP_STAT_ENCRYPTION_METHOD) {
        if (statBuffer.encryption_method != ZIP_EM_NONE) {
            e->setProperty("isPasswordProtected", true);
            switch (statBuffer.encryption_method) {
            case ZIP_EM_TRAD_PKWARE:
                emit encryptionMethodFound(QStringLiteral("ZipCrypto"));
                break;
            case ZIP_EM_AES_128:
                emit encryptionMethodFound(QStringLiteral("AES128"));
                break;
            case ZIP_EM_AES_192:
                emit encryptionMethodFound(QStringLiteral("AES192"));
                break;
            case ZIP_EM_AES_256:
                emit encryptionMethodFound(QStringLiteral("AES256"));
                break;
            }
        }
    }

    // Read external attributes, which contains the file permissions.
    zip_uint8_t opsys;
    zip_uint32_t attributes;
    if (zip_file_get_external_attributes(archive, index, ZIP_FL_UNCHANGED, &opsys, &attributes) == -1) {
        emit error(tr("Failed to read metadata for entry: %1"));
        return false;
    }

    // Set permissions.
    switch (opsys) {
    case ZIP_OPSYS_UNIX:
        // Unix permissions are stored in the leftmost 16 bits of the external file attribute.
        e->setProperty("permissions", permissionsToString(attributes >> 16));
        break;
    default:    // TODO: non-UNIX.
        break;
    }

    emit entry(e);
    m_emittedEntries << e;

    return true;
}

bool LibzipPlugin::deleteFiles(const QVector<Archive::Entry *> &files)
{
    int errcode = 0;
    zip_error_t err;

    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(filename()).constData(), 0, &errcode);
    zip_error_init_with_code(&err, errcode);
    if (archive == nullptr) {
        emit error(tr("Failed to open archive: %1"));
        return false;
    }

    qulonglong i = 0;
    for (const Archive::Entry *e : files) {

        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        const qlonglong index = zip_name_locate(archive, e->fullPath().toUtf8().constData(), ZIP_FL_ENC_GUESS);
        if (index == -1) {
            emit error(tr("Failed to delete entry: %1"));
            return false;
        }
        if (zip_delete(archive, index) == -1) {
            emit error(tr("Failed to delete entry: %1"));
            return false;
        }
        emit entryRemoved(e->fullPath());
        emit progress(float(++i) / files.size());
    }

    if (zip_close(archive)) {
        emit error(tr("Failed to write archive."));
        return false;
    }
    return true;
}

bool LibzipPlugin::addComment(const QString &comment)
{
    int errcode = 0;
    zip_error_t err;

    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(filename()).constData(), 0, &errcode);
    zip_error_init_with_code(&err, errcode);
    if (archive == nullptr) {
        emit error(tr("Failed to open archive: %1"));
        return false;
    }

    // Set archive comment.
    if (zip_set_archive_comment(archive, comment.toUtf8().constData(), comment.length())) {
        return false;
    }

    if (zip_close(archive)) {
        emit error(tr("Failed to write archive."));
        return false;
    }
    return true;
}

bool LibzipPlugin::testArchive()
{
    int errcode = 0;
    zip_error_t err;

    // Open archive performing extra consistency checks.
    zip_t *archive = zip_open(QFile::encodeName(filename()).constData(), ZIP_CHECKCONS, &errcode);
    zip_error_init_with_code(&err, errcode);
    if (archive == nullptr) {
        return false;
    }

    // Check CRC-32 for each archive entry.
    const int nofEntries = zip_get_num_entries(archive, 0);
    for (int i = 0; i < nofEntries; i++) {

        if (QThread::currentThread()->isInterruptionRequested()) {
            return false;
        }

        // Get statistic for entry. Used to get entry size.
        zip_stat_t statBuffer;
        if (zip_stat_index(archive, i, 0, &statBuffer) != 0) {
            return false;
        }

        zip_file *zipFile = zip_fopen_index(archive, i, 0);
        std::unique_ptr<uchar[]> buf(new uchar[statBuffer.size]);
        const int len = zip_fread(zipFile, buf.get(), statBuffer.size);
        if (len == -1 || uint(len) != statBuffer.size) {
            return false;
        }
        if (statBuffer.crc != crc32(0, &buf.get()[0], len)) {
            return false;
        }

        emit progress(float(i) / nofEntries);
    }

    zip_close(archive);

    emit testSuccess();
    return true;
}

bool LibzipPlugin::doKill()
{
    return false;
}

bool LibzipPlugin::extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options)
{
    const bool extractAll = files.isEmpty();
    const bool removeRootNode = options.isDragAndDropEnabled();
    m_extractionOptions = options;

    int errcode = 0;
    zip_error_t err;

    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(filename()).constData(), ZIP_RDONLY, &errcode);
    zip_error_init_with_code(&err, errcode);
    if (archive == nullptr) {
        emit error(tr("Failed to open archive: %1"));
        return false;
    }

    // Set password if known.
    if (!password().isEmpty()) {
        zip_set_default_password(archive, password().toUtf8().constData());
    }

    // Get number of archive entries.
    const qlonglong nofEntries = extractAll ? zip_get_num_entries(archive, 0) : files.size();

    // Extract entries.
    m_overwriteAll = false; // Whether to overwrite all files
    m_skipAll = false; // Whether to skip all files
    if (extractAll) {
        // We extract all entries.
        for (qlonglong i = 0; i < nofEntries; i++) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }

            if (!extractEntry(archive,
                              QDir::fromNativeSeparators(trans2uft8(zip_get_name(archive, i, ZIP_FL_ENC_RAW))),
                              QString(),
                              destinationDirectory,
                              options.preservePaths(),
                              removeRootNode)) {
                return false;
            }
            emit progress(float(i + 1) / nofEntries);
            emit progress_filename(trans2uft8(zip_get_name(archive, i, ZIP_FL_ENC_RAW)));
        }
    } else {
        // We extract only the entries in files.
        qulonglong i = 0;
        for (const Archive::Entry *e : files) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }
            if (!extractEntry(archive,
                              e->fullPath(),
                              e->rootNode,
                              destinationDirectory,
                              options.preservePaths(),
                              removeRootNode)) {
                return false;
            }
            emit progress(float(++i) / nofEntries);
            emit progress_filename(e->name());
        }
    }

    zip_close(archive);
    return true;
}

bool LibzipPlugin::extractEntry(zip_t *archive, const QString &entry, const QString &rootNode, const QString &destDir, bool preservePaths, bool removeRootNode)
{
    const bool isDirectory = entry.endsWith(QDir::separator());

    // Add trailing slash to destDir if not present.
    QString destDirCorrected(destDir);
    if (!destDir.endsWith(QDir::separator())) {
        destDirCorrected.append(QDir::separator());
    }

    // Remove rootnode if supplied and set destination path.
    QString destination;
    if (preservePaths) {
        if (!removeRootNode || rootNode.isEmpty()) {
            destination = destDirCorrected + entry;
        } else {
            QString truncatedEntry = entry;
            truncatedEntry.remove(0, rootNode.size());
            destination = destDirCorrected + truncatedEntry;
        }
    } else {
        if (isDirectory) {
            return true;
        }
        destination = destDirCorrected + QFileInfo(entry).fileName();
    }

    // Store parent mtime.
    QString parentDir;
    if (isDirectory) {
        QDir pDir = QFileInfo(destination).dir();
        pDir.cdUp();
        parentDir = pDir.path();
    } else {
        parentDir = QFileInfo(destination).path();
    }
    // For top-level items, don't restore parent dir mtime.
    const bool restoreParentMtime = (parentDir + QDir::separator() != destDirCorrected);

    time_t parent_mtime;
    if (restoreParentMtime) {
        parent_mtime = QFileInfo(parentDir).lastModified().toMSecsSinceEpoch() / 1000;
    }

    // Create parent directories for files. For directories create them.
    if (!QDir().mkpath(QFileInfo(destination).path())) {
        emit error(tr("Failed to create directory: %1"));
        return false;
    }

    // Get statistic for entry. Used to get entry size and mtime.
    zip_stat_t statBuffer;
    QTextCodec *codec = QTextCodec::codecForName(m_codecstr);
    qDebug() << m_codecstr;
    QByteArray  name = codec->fromUnicode(entry.toLocal8Bit());


    if (zip_stat(archive, name.constData(), 0, &statBuffer) != 0) {
        if (isDirectory && zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_NOENT) {
            return true;
        }
        return false;
    }

    if (!isDirectory) {

        // Handle existing destination files.
        QString renamedEntry = entry;
        while (!m_overwriteAll && QFileInfo::exists(destination)) {
            if (m_skipAll) {
                return true;
            } else {
                OverwriteQuery query(renamedEntry);
                emit userQuery(&query);
                query.waitForResponse();

                if (query.responseCancelled()) {
                    emit cancelled();
                    return false;
                } else if (query.responseSkip()) {
                    return true;
                } else if (query.responseAutoSkip()) {
                    m_skipAll = true;
                    return true;
                } else if (query.responseRename()) {
                    const QString newName(query.newFilename());
                    destination = QFileInfo(destination).path() + QDir::separator() + QFileInfo(newName).fileName();
                    renamedEntry = QFileInfo(entry).path() + QDir::separator() + QFileInfo(newName).fileName();
                } else if (query.responseOverwriteAll()) {
                    m_overwriteAll = true;
                    break;
                } else if (query.responseOverwrite()) {
                    break;
                }
            }
        }

        // Handle password-protected files.
        zip_file *zipFile = nullptr;
        bool firstTry = true;
        while (!zipFile) {
            zipFile = zip_fopen(archive, name.constData(), 0);
            if (zipFile) {
                break;
            } else if (zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_NOPASSWD ||
                       zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_WRONGPASSWD) {
                if(zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_NOPASSWD)
                {

                    if(m_extractionOptions.isBatchExtract())
                    {
                        PasswordNeededQuery query(filename());
                        emit userQuery(&query);
                        query.waitForResponse();

                        if (query.responseCancelled()) {
                            setPassword(QString());
                            emit cancelled();
                            return false;
                        }

                        setPassword(query.password());

                        if (zip_set_default_password(archive, password().toUtf8().constData())) {
                        }
                        firstTry = false;
                    }
                    else {
                        emit sigExtractNeedPassword();
                        setPassword(QString());
                        zip_set_default_password(archive, password().toUtf8().constData());
                        return false;
                    }
                }
                else if(zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_WRONGPASSWD)
                {
                    if(m_extractionOptions.isBatchExtract())
                    {
                        setPassword(QString());
                        emit cancelled();
                        return false;
                    }
                    else {
                        emit sigExtractNeedPassword();
                    }
                    setPassword(QString());
                    zip_set_default_password(archive, password().toUtf8().constData());
                    return false;
                }



            } else {
                emit error(tr("Failed to open '%1':<nl/>%2"));
                return false;
            }
        }

        QFile file(destination);
        if (!file.open(QIODevice::WriteOnly)) {
            emit error(tr("Failed to open file for writing: %1"));
            return false;
        }

        QDataStream out(&file);

        // Write archive entry to file. We use a read/write buffer of 1000 chars.
        qulonglong sum = 0;
        char buf[1000];
        while (sum != statBuffer.size) {
            const auto readBytes = zip_fread(zipFile, buf, 1000);
            if (readBytes < 0) {
                emit error(tr("Failed to read data for entry: %1"));
                return false;
            }
            if (out.writeRawData(buf, readBytes) != readBytes) {
                emit error(tr("Failed to write data for entry: %1"));
                return false;
            }

            sum += readBytes;
        }

        const auto index = zip_name_locate(archive, name.constData(), ZIP_FL_ENC_RAW);
        if (index == -1) {
            emit error(tr("Failed to locate entry: %1"));
            return false;
        }

        zip_uint8_t opsys;
        zip_uint32_t attributes;
        if (zip_file_get_external_attributes(archive, index, ZIP_FL_UNCHANGED, &opsys, &attributes) == -1) {
            emit error(tr("Failed to read metadata for entry: %1"));
            return false;
        }

        // Inspired by fuse-zip source code: fuse-zip/lib/fileNode.cpp
        switch (opsys) {
        case ZIP_OPSYS_UNIX:
            // Unix permissions are stored in the leftmost 16 bits of the external file attribute.
//            file.setPermissions(KIO::convertPermissions(attributes >> 16)); //TODO_DS
            break;
        default:    // TODO: non-UNIX.
            break;
        }

        file.close();
    }

    // Set mtime for entry.
    utimbuf times;
    times.modtime = statBuffer.mtime;
    if (utime(destination.toUtf8().constData(), &times) != 0) {
    }

    if (restoreParentMtime) {
        // Restore mtime for parent dir.
        times.modtime = parent_mtime;
        if (utime(parentDir.toUtf8().constData(), &times) != 0) {
        }
    }

    return true;
}

bool LibzipPlugin::moveFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options)
{
    Q_UNUSED(options)
    int errcode = 0;
    zip_error_t err;

    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(filename()).constData(), 0, &errcode);
    zip_error_init_with_code(&err, errcode);
    if (archive == nullptr) {
        emit error(tr("Failed to open archive: %1"));
        return false;
    }

    QStringList filePaths = entryFullPaths(files);
    filePaths.sort();
    const QStringList destPaths = entryPathsFromDestination(filePaths, destination, entriesWithoutChildren(files).count());

    int i;
    for (i = 0; i < filePaths.size(); ++i) {

        const int index = zip_name_locate(archive, filePaths.at(i).toUtf8().constData(), ZIP_FL_ENC_GUESS);
        if (index == -1) {
            emit error(tr("Failed to move entry: %1"));
            return false;
        }

        if (zip_file_rename(archive, index, destPaths.at(i).toUtf8().constData(), ZIP_FL_ENC_GUESS) == -1) {
            emit error(tr("Failed to move entry: %1"));
            return false;
        }

        emit entryRemoved(filePaths.at(i));
        emitEntryForIndex(archive, index);
        emit progress(i / filePaths.count());
    }
    if (zip_close(archive)) {
        emit error(tr("Failed to write archive."));
        return false;
    }

    return true;
}

bool LibzipPlugin::copyFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options)
{
    Q_UNUSED(options)
    int errcode = 0;
    zip_error_t err;

    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(filename()).constData(), 0, &errcode);
    zip_error_init_with_code(&err, errcode);
    if (archive == nullptr) {
        emit error(tr("Failed to open archive: %1"));
        return false;
    }

    const QStringList filePaths = entryFullPaths(files);
    const QStringList destPaths = entryPathsFromDestination(filePaths, destination, 0);

    int i;
    for (i = 0; i < filePaths.size(); ++i) {

        QString dest = destPaths.at(i);

        if (dest.endsWith(QDir::separator())) {
            if (zip_dir_add(archive, dest.toUtf8().constData(), ZIP_FL_ENC_GUESS) == -1) {
                // If directory already exists in archive, we get an error.
                continue;
            }
        }

        const int srcIndex = zip_name_locate(archive, filePaths.at(i).toUtf8().constData(), ZIP_FL_ENC_GUESS);
        if (srcIndex == -1) {
            emit error(tr("Failed to copy entry: %1"));
            return false;
        }

        zip_source_t *src = zip_source_zip(archive, archive, srcIndex, 0, 0, -1);
        if (!src) {
            return false;
        }

        const int destIndex = zip_file_add(archive, dest.toUtf8().constData(), src, ZIP_FL_ENC_GUESS | ZIP_FL_OVERWRITE);
        if (destIndex == -1) {
            zip_source_free(src);
            emit error(tr("Failed to add entry: %1"));
            return false;
        }

        // Get permissions from source entry.
        zip_uint8_t opsys;
        zip_uint32_t attributes;
        if (zip_file_get_external_attributes(archive, srcIndex, ZIP_FL_UNCHANGED, &opsys, &attributes) == -1) {
            emit error(tr("Failed to read metadata for entry: %1"));
            return false;
        }

        // Set permissions on dest entry.
        if (zip_file_set_external_attributes(archive, destIndex, ZIP_FL_UNCHANGED, opsys, attributes) != 0) {
            emit error(tr("Failed to set metadata for entry: %1"));
            return false;
        }
    }

    // Register the callback function to get progress feedback.
    zip_register_progress_callback_with_state(archive, 0.001, progressCallback, nullptr, this);

    if (zip_close(archive)) {
        emit error(tr("Failed to write archive."));
        return false;
    }

    // List the archive to update the model.
    m_listAfterAdd = true;
    list();


    return true;
}

QString LibzipPlugin::permissionsToString(const mode_t &perm)
{
    QString modeval;
    if ((perm & S_IFMT) == S_IFDIR) {
        modeval.append(QLatin1Char('d'));
    } else if ((perm & S_IFMT) == S_IFLNK) {
        modeval.append(QLatin1Char('l'));
    } else {
        modeval.append(QLatin1Char('-'));
    }
    modeval.append((perm & S_IRUSR) ? QLatin1Char('r') : QLatin1Char('-'));
    modeval.append((perm & S_IWUSR) ? QLatin1Char('w') : QLatin1Char('-'));
    if ((perm & S_ISUID) && (perm & S_IXUSR)) {
        modeval.append(QLatin1Char('s'));
    } else if ((perm & S_ISUID)) {
        modeval.append(QLatin1Char('S'));
    } else if ((perm & S_IXUSR)) {
        modeval.append(QLatin1Char('x'));
    } else {
        modeval.append(QLatin1Char('-'));
    }
    modeval.append((perm & S_IRGRP) ? QLatin1Char('r') : QLatin1Char('-'));
    modeval.append((perm & S_IWGRP) ? QLatin1Char('w') : QLatin1Char('-'));
    if ((perm & S_ISGID) && (perm & S_IXGRP)) {
        modeval.append(QLatin1Char('s'));
    } else if ((perm & S_ISGID)) {
        modeval.append(QLatin1Char('S'));
    } else if ((perm & S_IXGRP)) {
        modeval.append(QLatin1Char('x'));
    } else {
        modeval.append(QLatin1Char('-'));
    }
    modeval.append((perm & S_IROTH) ? QLatin1Char('r') : QLatin1Char('-'));
    modeval.append((perm & S_IWOTH) ? QLatin1Char('w') : QLatin1Char('-'));
    if ((perm & S_ISVTX) && (perm & S_IXOTH)) {
        modeval.append(QLatin1Char('t'));
    } else if ((perm & S_ISVTX)) {
        modeval.append(QLatin1Char('T'));
    } else if ((perm & S_IXOTH)) {
        modeval.append(QLatin1Char('x'));
    } else {
        modeval.append(QLatin1Char('-'));
    }
    return modeval;
}

QByteArray LibzipPlugin::detectEncode(const QByteArray &data, const QString &fileName)
{
    // Return local encoding if nothing in file.
    if (data.isEmpty()) {
        return QTextCodec::codecForLocale()->name();
    }

    if (QTextCodec *c = QTextCodec::codecForUtfText(data, nullptr)) {
        return c->name();
    }

    QMimeDatabase mime_database;
    const QMimeType &mime_type = fileName.isEmpty() ? mime_database.mimeTypeForData(data) : mime_database.mimeTypeForFileNameAndData(fileName, data);
    const QString &mimetype_name = mime_type.name();
    KEncodingProber::ProberType proberType = KEncodingProber::Universal;

    if (mimetype_name == QStringLiteral("application/xml")
            || mimetype_name == QStringLiteral("text/html")
            || mimetype_name == QStringLiteral("application/xhtml+xml")) {
        const QString &_data = QString::fromLatin1(data);
        QRegularExpression pattern("<\\bmeta.+\\bcharset=(?'charset'\\S+?)\\s*['\"/>]");

        pattern.setPatternOptions(QRegularExpression::DontCaptureOption | QRegularExpression::CaseInsensitiveOption);
        const QString &charset = pattern.match(_data, 0, QRegularExpression::PartialPreferFirstMatch,
                                               QRegularExpression::DontCheckSubjectStringMatchOption).captured("charset");

        if (!charset.isEmpty()) {
            return charset.toLatin1();
        }

        pattern.setPattern("<\\bmeta\\s+http-equiv=\"Content-Language\"\\s+content=\"(?'language'[a-zA-Z-]+)\"");

        const QString &language = pattern.match(_data, 0, QRegularExpression::PartialPreferFirstMatch,
                                                QRegularExpression::DontCheckSubjectStringMatchOption).captured("language");

        if (!language.isEmpty()) {
            QLocale l(language);

            switch (l.script()) {
            case QLocale::ArabicScript:
                proberType = KEncodingProber::Arabic;
                break;
            case QLocale::SimplifiedChineseScript:
                proberType = KEncodingProber::ChineseSimplified;
                break;
            case QLocale::TraditionalChineseScript:
                proberType = KEncodingProber::ChineseTraditional;
                break;
            case QLocale::CyrillicScript:
                proberType = KEncodingProber::Cyrillic;
                break;
            case QLocale::GreekScript:
                proberType = KEncodingProber::Greek;
                break;
            case QLocale::HebrewScript:
                proberType = KEncodingProber::Hebrew;
                break;
            case QLocale::JapaneseScript:
                proberType = KEncodingProber::Japanese;
                break;
            case QLocale::KoreanScript:
                proberType = KEncodingProber::Korean;
                break;
            case QLocale::ThaiScript:
                proberType = KEncodingProber::Thai;
                break;
            default:
                break;
            }
        }
    } else if (mimetype_name == "text/x-python") {
        QRegularExpression pattern("^#coding\\s*:\\s*(?'coding'\\S+)$");
        QTextStream stream(data);

        pattern.setPatternOptions(QRegularExpression::DontCaptureOption | QRegularExpression::CaseInsensitiveOption);
        stream.setCodec("latin1");

        while (!stream.atEnd()) {
            const QString &_data = stream.readLine();
            const QString &coding = pattern.match(_data, 0).captured("coding");

            if (!coding.isEmpty()) {
                return coding.toLatin1();
            }
        }
    }

    // for CJK
    const QList<QPair<KEncodingProber::ProberType, QLocale::Country>> fallback_list {
        {KEncodingProber::ChineseSimplified, QLocale::China},
        {KEncodingProber::ChineseTraditional, QLocale::China},
        {KEncodingProber::Japanese, QLocale::Japan},
        {KEncodingProber::Korean, QLocale::NorthKorea},
        {KEncodingProber::Cyrillic, QLocale::Russia},
        {KEncodingProber::Greek, QLocale::Greece},
        {proberType, QLocale::system().country()}
    };

    KEncodingProber prober(proberType);
    prober.feed(data);
    float pre_confidence = prober.confidence();
    QByteArray pre_encoding = prober.encoding();

    QTextCodec *def_codec = QTextCodec::codecForLocale();
    QByteArray encoding;
    float confidence = 0;

    for (auto i : fallback_list) {
        prober.setProberType(i.first);
        prober.feed(data);

        float prober_confidence = prober.confidence();
        QByteArray prober_encoding = prober.encoding();

        if (i.first != proberType && qFuzzyIsNull(prober_confidence)) {
            prober_confidence = pre_confidence;
            prober_encoding = pre_encoding;
        }

    confidence:
        if (QTextCodec *codec = QTextCodec::codecForName(prober_encoding)) {
            if (def_codec == codec)
                def_codec = nullptr;

            float c = codecConfidenceForData(codec, data, i.second);

            if (prober_confidence > 0.5) {
                c = c / 2 + prober_confidence / 2;
            } else {
                c = c / 3 * 2 + prober_confidence / 3;
            }

            if (c > confidence) {
                confidence = c;
                encoding = prober_encoding;
            }

            if (i.first == KEncodingProber::ChineseTraditional && c < 0.5) {
                // test Big5
                c = codecConfidenceForData(QTextCodec::codecForName("Big5"), data, i.second);

                if (c > 0.5 && c > confidence) {
                    confidence = c;
                    encoding = "Big5";
                }
            }
        }

        if (i.first != proberType) {
            // 使用 proberType 类型探测出的结果结合此国家再次做编码检查
            i.first = proberType;
            prober_confidence = pre_confidence;
            prober_encoding = pre_encoding;
            goto confidence;
        }
    }

    if (def_codec && codecConfidenceForData(def_codec, data, QLocale::system().country()) > confidence) {
        return def_codec->name();
    }


    return encoding;
}



//#include "libzipplugin.moc"
