#include "libzipplugin.h"
//#include "kpluginfactory.h"
#include "../common/detectencoding.h"
//#include "../common/common.h"
#include "queries.h"

//#include <KIO/Global>
//#include <KLocalizedString>

#include <QElapsedTimer>
#include <QDataStream>
#include <QDateTime>
#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <qplatformdefs.h>
#include <QThread>
#include <QTextCodec>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QProcess>

#include <zlib.h>
#include <chardet.h>
#include <KEncodingProber>
#include <utime.h>
#include <memory>

//K_PLUGIN_CLASS_WITH_JSON(LibzipPlugin, "kerfuffle_libzip.json")

#define MAX_FILENAME 512
#define READ_SIZE 8192


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

int LibzipPlugin::cancelCallback(zip_t *, void *that)
{
    return static_cast<LibzipPlugin *>(that)->cancelResult();
}

LibzipPlugin::LibzipPlugin(QObject *parent, const QVariantList &args)
    : ReadWriteArchiveInterface(parent, args)
    , m_overwriteAll(false)
    , m_skipAll(false)
    , m_listAfterAdd(false)
{
    m_listCodecs.clear();
    m_listCodecs << "UTF-8" << "GB18030" << "GBK" << "Big5" << "us-ascii";
    mType = ENUM_PLUGINTYPE::PLUGIN_LIBZIP;
    m_common = new Common(this);
    connect(this, &ReadOnlyArchiveInterface::error, this, &LibzipPlugin::slotRestoreWorkingDir);
    connect(this, &ReadOnlyArchiveInterface::cancelled, this, &LibzipPlugin::slotRestoreWorkingDir);
}

LibzipPlugin::~LibzipPlugin()
{
//    for (const auto e : qAsConst(m_emittedEntries)) {
//        // Entries might be passed to pending slots, so we just schedule their deletion.
//        e->deleteLater();
//    }
//    m_emittedEntries.clear();
}

bool LibzipPlugin::list(bool /*isbatch*/)
{
    m_fileNameEncodeMap.clear();
    m_numberOfEntries = 0;

    int errcode = 0;
    zip_error_t err;

    // Open archive.
    QString fileName = filename();
    zip_t *archive = zip_open(QFile::encodeName(fileName).constData(), ZIP_RDONLY, &errcode);
    zip_error_init_with_code(&err, errcode);
    m_bAllEntry = false;
    if (!archive) {
        //emit error(("Failed to open archive: %1"));
        //return false;
        m_bAllEntry = true;
        return minizip_list();
    }

//    if (!verifyPassword()) {
//        return false;
//    }

    // Fetch archive comment.
    m_comment = QString::fromLocal8Bit(zip_get_archive_comment(archive, nullptr, ZIP_FL_ENC_RAW));

    // Get number of archive entries.
    const auto nofEntries = zip_get_num_entries(archive, 0);

    //detectAllfile(archive, nofEntries);
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
            QByteArray codec = m_common->detectEncode(statBuffer.name);
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

const char *LibzipPlugin::passwordUnicode(const QString &strPassword, int iIndex)
{
    if (filename().endsWith(".zip")) {
        // QStringList listCodecName = QStringList() << "UTF-8" << "GB18030" << "GBK" <<"Big5"<< "us-ascii";
        int nCount = strPassword.count();
        bool b = false;
        for (int i = 0 ; i < nCount ; i++) {
            QChar cha = strPassword.at(i);
            ushort uni = cha.unicode();
            if (uni >= 0x4E00 && uni <= 0x9FA5) {
                b = true;
                break;
            }
        }

        // chinese
        if (b) {
            QTextCodec *utf8 = QTextCodec::codecForName("UTF-8");
            QTextCodec *gbk = QTextCodec::codecForName(m_listCodecs[iIndex].toUtf8().data());
            // QTextCodec *gbk = QTextCodec::codecForName("UTF-8");

            //utf8 -> 所需编码
            //1. utf8 -> unicode
            QString strUnicode = utf8->toUnicode(strPassword.toUtf8().data());
            //2. unicode -> 所需编码, 得到QByteArray
            QByteArray gb_bytes = gbk->fromUnicode(strUnicode);
            return gb_bytes.data(); //获取其char *
        } else {
            return strPassword.toUtf8().constData();
        }
    } else {
        return strPassword.toUtf8().constData();
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
        emit error(("Failed to open archive: %1"));
        return false;
    }

    uint i = 0;
    for (const Archive::Entry *e : files) {
        // 过滤上级目录（不对全路径进行压缩）
        QString strPath = QFileInfo(e->fullPath()).absolutePath() + QDir::separator();

        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        // If entry is a directory, traverse and add all its files and subfolders.
        if (QFileInfo(e->fullPath()).isDir()) {
            if (!writeEntry(archive, e->fullPath(), destination, options, true, strPath)) {
                if (zip_close(archive)) {
                    emit error(("Failed to write archive."));
                    return false;
                }
                return false;
            }

            QDirIterator it(e->fullPath(),
                            QDir::AllEntries | QDir::Readable |
                            QDir::Hidden | QDir::NoDotAndDotDot,
                            QDirIterator::Subdirectories);

            while (!QThread::currentThread()->isInterruptionRequested() && it.hasNext()) {
                const QString path = it.next();

                if (QFileInfo(path).isDir()) {
                    if (!writeEntry(archive, path, destination, options, true, strPath)) {
                        if (zip_close(archive)) {
                            emit error(("Failed to write archive."));
                            return false;
                        }
                        return false;
                    }
                } else {
                    if (!writeEntry(archive, path, destination, options, false, strPath)) {
                        if (zip_close(archive)) {
                            emit error(("Failed to write archive."));
                            return false;
                        }
                        return false;
                    }
                }
                i++;
            }
        } else {
            if (!writeEntry(archive, e->fullPath(), destination, options, false, strPath)) {
                if (zip_close(archive)) {
                    emit error(("Failed to write archive."));
                    return false;
                }
                return false;
            }
        }
        i++;

    }

    m_filesize = i;
    m_addarchive = archive;
    // Register the callback function to get progress feedback.
    zip_register_progress_callback_with_state(archive, 0.001, progressCallback, nullptr, this);
    zip_register_cancel_callback_with_state(archive, cancelCallback, nullptr, this);

    if (zip_close(archive)) {
        emit error(("Failed to write archive."));
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
    bool flag = true;
    while (flag) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }
        if (m_isPause) { //压缩暂停
            sleep(1);
            //            qDebug() << "pause";
            continue;
        }

        int i = m_filesize * percentage;
        if (m_addarchive) {
            emit progress_filename(m_common->trans2uft8(zip_get_name(m_addarchive, i, ZIP_FL_ENC_RAW)));
        }

        // Go from 0 to 50%. The second half is the subsequent listing.
        emit progress(percentage);
        flag = false;
    }
}

int LibzipPlugin::cancelResult()
{
    if (m_bCancel) {
        m_bCancel = false;
        return 1;
    } else {
        return 0;
    }
}

bool LibzipPlugin::writeEntry(zip_t *archive, const QString &file, const Archive::Entry *destination, const CompressionOptions &options, bool isDir, const QString &strRoot)
{
    Q_ASSERT(archive);

    QByteArray destFile;
    if (destination) {
        destFile = QString(destination->fullPath() + file).toUtf8();
    } else {
        destFile = file.toUtf8();
    }

    QString str = destFile.constData();
    str = str.remove(0, strRoot.length());

    qlonglong index;
    if (isDir) {
        index = zip_dir_add(archive, str.toUtf8().constData(), ZIP_FL_ENC_GUESS);
        if (index == -1) {
            // If directory already exists in archive, we get an error.
            return true;
        }
    } else {
        zip_source_t *src = zip_source_file(archive, QFile::encodeName(file).constData(), 0, -1);
        Q_ASSERT(src);

        index = zip_file_add(archive, str.toUtf8().constData(), src, ZIP_FL_ENC_GUESS | ZIP_FL_OVERWRITE);
        if (index == -1) {
            zip_source_free(src);
            emit error(("Failed to add entry: %1"));
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
        emit error(("Failed to set compression options for entry: %1"));
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

    if (statBuffer.valid & ZIP_STAT_ENCRYPTION_METHOD) {
        if (!m_isEncrypted && statBuffer.encryption_method != ZIP_EM_NONE) { //若list时发现有文件加密，则通知loadjob该压缩包是加密
            emit sigIsEncrypted();
            m_isEncrypted = true;
        }
    }

    QString name = m_common->trans2uft8(statBuffer.name);
//    m_fileNameEncodeMap.insert(statBuffer.name, name);
    setEntryVal(statBuffer, m_indexCount, name, m_DirRecord);
    if (m_fileNameEncodeMap.find(name) == m_fileNameEncodeMap.end()) {
        m_fileNameEncodeMap.insert(name, statBuffer.name);
    }

    if (m_listMap.find(name) == m_listMap.end()) {
        m_listMap.insert(name, qMakePair(statBuffer, index));
    }

    return true;
}

bool LibzipPlugin::deleteFiles(const QVector<Archive::Entry *> &files)
{
    int errcode = 0;
    zip_error_t err;

    // Open archive.
    char *fileName = QFile::encodeName(filename()).data();
    zip_t *archive = zip_open(fileName, 0, &errcode);
    if (errcode != ZIP_ER_OK) {
        emit error(("Failed to open archive: %1"));
        return false;
    }

    zip_error_init_with_code(&err, errcode);
    if (archive == nullptr) {
        emit error(("Failed to open archive: %1"));
        return false;
    }

    // Register the callback function to get progress feedback.
    m_addarchive = nullptr;
    zip_register_progress_callback_with_state(archive, 0.001, progressCallback, nullptr, this);
    //    for (Archive::Entry *pCurEntry : files) {
    //        //        int i = 0;
    //        //        qint64 count = 0;
    //        //        pCurEntry->calEntriesCount(count);
    //        bool status = this->deleteEntry(pCurEntry, archive/*, i, count*/);  //delete from archive
    //        if (status == true) {
    //            emit entryRemoved(pCurEntry->fullPath());                   //delete from model
    //        }
    //    }
    for (int i = 0; i < m_listExtractIndex.count(); i++) {
        QString strFilePath = m_common->trans2uft8(zip_get_name(archive, m_listExtractIndex[i], ZIP_FL_ENC_RAW));
        bool status = this->deleteEntry(strFilePath, m_listExtractIndex[i], archive/*, i, count*/);        //delete from archive
        if (status == true) {
            emit entryRemoved(/*files.at(i)->fullPath()*/strFilePath);      //delete from model
        } else {
            return false;
        }
    }

    if (this->extractPsdStatus == ReadOnlyArchiveInterface::Canceled) {
        qDebug() << "canceled";
        return false;
    }

    if (zip_close(archive)) {
        emit error(("Failed to write archive."));
        return false;
    }
    return true;
}

bool LibzipPlugin::deleteEntry(QString file, int index/*Archive::Entry *pCurEntry*/, zip_t *archive/*, int &curNo, int count*/)
{
//    QFileInfo fi(file);
//    if (fi.isDir() == true) {
//        for (int i = 0; i < m_listExtractIndex.count()/*pCurEntry->entries().length()*/; i++) {
//            this->deleteEntry(file/*->entries().at(i)*/, archive/*, curNo, count*/);
//        }
//    }

    if (QThread::currentThread()->isInterruptionRequested()) {
        if (zip_close(archive)) {
            emit error(("Failed to write archive."));
            return false;
        }
        return false;
    }

    int statusDel = zip_delete(archive, index);
    if (statusDel == -1) {
        emit error(("Failed to delete entry: %1"));
        if (zip_close(archive)) {
            emit error(("Failed to write archive."));
            return false;
        }

        return false;
    }

    /*QString fullpath = file;
    char *fileNameDel = nullptr;
    //char *path = fullpath.toUtf8().data();
    size_t length = strlen(fullpath.toUtf8().data());
    fileNameDel = (char *)malloc((length + 1) * sizeof(char));
    strcpy(fileNameDel, fullpath.toUtf8().data());
    //    const char *nn1 = zip_get_name(archive, 0, ZIP_FL_ENC_GUESS);
    zip_int64_t index1 = zip_name_locate(archive, fileNameDel, ZIP_FL_ENC_GUESS);
    if (index1 == -1) {
    //        free(fileNameDel);
    //        return false;
        QString val = m_fileNameEncodeMap.value(fullpath);
        QStringList listArgs;
        listArgs << "d" << filename() << val;
        QString strProgram = QStandardPaths::findExecutable("7z");
        QProcess cmd;
        cmd.start(strProgram, listArgs);
        if (cmd.waitForFinished(-1)) {
            return true;
        } else {
            return false;
        }
    } else {
    //        qDebug() << "------deleteOk------" << fileNameDel;
    //        free(fileNameDel);
        zip_int64_t indexDel = index;
    //        zip_file_t *zipFile = zip_fopen_index(archive, indexDel, 0);
    //        const char *gg = zip_get_name(archive, index, ZIP_FL_ENC_RAW);
    //        zip_stat_t statBuffer;
    //        zip_stat_index(archive, index, 0, &statBuffer);
    //        int ii = statBuffer.index;
        int statusDel = zip_delete(archive, indexDel);
        if (statusDel == -1) {
            emit error(("Failed to delete entry: %1"));
            if (zip_close(archive)) {
                emit error(("Failed to write archive."));
                return false;
            }

            return false;
        }
    }*/

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
        emit error(("Failed to open archive: %1"));
        return false;
    }

    // Set archive comment.
    if (zip_set_archive_comment(archive, comment.toUtf8().constData(), comment.length())) {
        return false;
    }

    if (zip_close(archive)) {
        emit error(("Failed to write archive."));
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
    m_bCancel = true;
    return false;
}

bool LibzipPlugin::checkArchivePsd(zip_t *archive, int &iCodecIndex)
{
    const qlonglong nofEntries =  zip_get_num_entries(archive, 0);

    bool bPasswordRight = false;
    while (!bPasswordRight) {
        //check password
        for (qlonglong i = 0; i < nofEntries; i++) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }

            QString entry;
            entry = QDir::fromNativeSeparators(m_common->trans2uft8(zip_get_name(archive, i, ZIP_FL_ENC_RAW)));
            const bool isDirectory = entry.endsWith(QDir::separator());
            if (isDirectory) {
                if (i == nofEntries - 1) {
                    bPasswordRight = true;
                    break;
                }

                continue;
            } else {
                zip_uint64_t iIndex = static_cast <zip_uint64_t>(i);
                zip_file *zipFile = zip_fopen_index(archive, iIndex, 0);
//                zip_file *zipFile = zip_fopen(archive, name.constData(), 0);
                //if zipFile return not 0,it sees normal，so break，then done extract; else，check why failed.
                if (zipFile) {
                    zip_fclose(zipFile);
                    bPasswordRight = true;
                    break;
                } else if (zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_NOPASSWD) {
                    m_isckeckpsd = false; //阻止解压zip加密包出现解压失败界面再出现输入密码界面
                    if (m_extractionOptions.isBatchExtract()) {//批量提取
                        PasswordNeededQuery query(filename());
                        emit userQuery(&query);
                        query.waitForResponse();
                        if (query.responseCancelled()) {
                            setPassword(QString());
                            emit cancelled();
                            return false;
                        }

                        setPassword(query.password());
                        if (zip_set_default_password(archive, passwordUnicode(password(), 0))) {
                        }
                    } else {
                        emit sigExtractNeedPassword();
                        //setPassword(QString());
                        zip_set_default_password(archive, passwordUnicode(password(), 0));
                        return false;
                    }
                } else if (zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_WRONGPASSWD) {
                    m_isckeckpsd = true;

                    /*zip加密包的中文密码会有多种编码
                    * 打开加密包失败可能是1、密码错误 2、当前编码不符合原来的密码编码导致密码错误
                    * 所以密码错误时尝试使用另一种编码的密码打开加密包
                    */
                    //setPassword(QString());
                    iCodecIndex++;
                    if (iCodecIndex == m_listCodecs.size()) { //密码
                        if (m_extractionOptions.isBatchExtract()) { //批量解压时，密码错误继续提示输入密码
                            PasswordNeededQuery query(filename());
                            emit userQuery(&query);
                            query.waitForResponse();
                            if (query.responseCancelled()) {
                                setPassword(QString());
                                emit cancelled();
                                return false;
                            }
                            iCodecIndex = 0;
                            setPassword(query.password());
                        } else {
                            emit sigExtractNeedPassword();
                            return false;
                        }
                    } else {
                        i--;
                    }
                    zip_set_default_password(archive, passwordUnicode(password(), iCodecIndex));
                    bPasswordRight = false;
                }

            }
        }
    }

    return bPasswordRight;
}

//void LibzipPlugin::checkEntryPsd(zip_t *archive, Archive::Entry *pCur, enum_checkEntryPsd &status)
//{
//    if (QThread::currentThread()->isInterruptionRequested()) {
//        return;
//    }

//    if (status != enum_checkEntryPsd::NOTCHECK) {
//        return;
//    }

//    if (pCur->isDir()) {
//        foreach (Archive::Entry *p, pCur->entries()) {
//            checkEntryPsd(archive, p, status);
//            if (status != enum_checkEntryPsd::NOTCHECK) {
//                break;
//            }
//        }
//    } else {
//        bool bPasswordRight = false;
//        int iCodecIndex = 0;
//        int iIndex = pCur->compressIndex();
//        while (!bPasswordRight) {
//            zip_file *zipFile = zip_fopen_index(archive, iIndex, 0);
//            if (iCodecIndex == m_listCodecs.length()) {
//                bPasswordRight = true;
//                status = enum_checkEntryPsd::PSDWRONG;
//                emit sigExtractNeedPassword();
//                break;
//            }
//            //if zipFile return not 0,it sees normal，so break，then done extract; else，check why failed.
//            if (zipFile) {
//                zip_fclose(zipFile);
//                status = enum_checkEntryPsd::RIGHT;
//                bPasswordRight = true;
//                return;
//            } else if (zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_NOPASSWD) {
//                m_isckeckpsd = false; //阻止解压zip加密包出现解压失败界面再出现输入密码界面
//                if (m_extractionOptions.isBatchExtract()) {//批量提取
//                    PasswordNeededQuery query(filename());
//                    emit userQuery(&query);
//                    query.waitForResponse();
//                    if (query.responseCancelled()) {
//                        setPassword(QString());
//                        emit cancelled();
//                    }
//                    setPassword(query.password());
//                    zip_set_default_password(archive, passwordUnicode(password(), iCodecIndex));
//                } else {
//                    emit sigExtractNeedPassword();
//                    setPassword(QString());
//                    zip_set_default_password(archive, passwordUnicode(password(), iCodecIndex));

//                }
//                bPasswordRight = true;
//                status = enum_checkEntryPsd::PSDNEED;
//            } else if (zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_WRONGPASSWD) {
//                m_isckeckpsd = true;
//                if (m_extractionOptions.isBatchExtract()) {
//                    //setPassword(QString());
//                    bPasswordRight = true;
//                    emit cancelled();
//                } else {
//                    //emit sigExtractNeedPassword();
//                }
//                //setPassword(QString());
//                zip_set_default_password(archive, passwordUnicode(password(), iCodecIndex));
//                iCodecIndex++;
//            }
//        }
//    }
//}

void LibzipPlugin::checkEntryPsd(zip_t *archive, int iIndex, enum_checkEntryPsd &status)
{
    if (QThread::currentThread()->isInterruptionRequested()) {
        return;
    }

    if (status != enum_checkEntryPsd::NOTCHECK) {
        return;
    }

    QString strFileName = m_common->trans2uft8(zip_get_name(archive, iIndex, ZIP_FL_ENC_RAW));

    if (strFileName.endsWith(QDir::separator()) || iIndex == -1) {
        return;
    } else {
        bool bPasswordRight = false;
        int iCodecIndex = 0;
        //int iIndex = pCur->compressIndex();
        while (!bPasswordRight) {
            zip_file *zipFile = zip_fopen_index(archive, iIndex, 0);
            if (iCodecIndex == m_listCodecs.length()) {
                bPasswordRight = true;
                status = enum_checkEntryPsd::PSDWRONG;
                emit sigExtractNeedPassword();
                break;
            }

            //if zipFile return not 0,it sees normal，so break，then done extract; else，check why failed.
            if (zipFile) {
                zip_fclose(zipFile);
                status = enum_checkEntryPsd::RIGHT;
                bPasswordRight = true;
                return;
            } else if (zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_NOPASSWD) {
                m_isckeckpsd = false; //阻止解压zip加密包出现解压失败界面再出现输入密码界面
                if (m_extractionOptions.isBatchExtract()) {//批量提取
                    PasswordNeededQuery query(filename());
                    emit userQuery(&query);
                    query.waitForResponse();
                    if (query.responseCancelled()) {
                        setPassword(QString());
                        emit cancelled();
                    }

                    setPassword(query.password());
                    zip_set_default_password(archive, passwordUnicode(password(), iCodecIndex));
                } else {
                    emit sigExtractNeedPassword();
                    setPassword(QString());
                    zip_set_default_password(archive, passwordUnicode(password(), iCodecIndex));
                }

                bPasswordRight = true;
                status = enum_checkEntryPsd::PSDNEED;
            } else if (zip_error_code_zip(zip_get_error(archive)) == ZIP_ER_WRONGPASSWD) {
                m_isckeckpsd = true;
                if (m_extractionOptions.isBatchExtract()) {
                    //setPassword(QString());
                    bPasswordRight = true;
                    emit cancelled();
                } else {
                    //emit sigExtractNeedPassword();
                }

                //setPassword(QString());
                zip_set_default_password(archive, passwordUnicode(password(), iCodecIndex));
                iCodecIndex++;
            }
        }
    }
}

//bool LibzipPlugin::checkEntriesPsd(zip_t *archive, const QVector<Archive::Entry *> &selectedEnV)
//{
//    //遍历树，去检索有无密码
//    // const qlonglong nofEntries = selectedEnV.size();
//    enum_checkEntryPsd status = enum_checkEntryPsd::NOTCHECK;
//    foreach (Archive::Entry *pE, selectedEnV) {
//        checkEntryPsd(archive, pE, status);
//        if (status == enum_checkEntryPsd::RIGHT) {
//            break;
//        } else if (status != enum_checkEntryPsd::NOTCHECK) {
//            return false;
//        }
//    }
//    return true;
//}

bool LibzipPlugin::checkEntriesPsd(zip_t *archive, QList<int> listExtractIndex)
{
    //遍历树，去检索有无密码
    //const qlonglong nofEntries = selectedEnV.size();
    enum_checkEntryPsd status = enum_checkEntryPsd::NOTCHECK;

//    foreach (Archive::Entry *pE, selectedEnV) {
//        checkEntryPsd(archive, pE, status);
//        if (status == enum_checkEntryPsd::RIGHT) {
//            break;
//        } else if (status != enum_checkEntryPsd::NOTCHECK) {
//            return false;
//        }
//    }

    foreach (int index, listExtractIndex) {
        checkEntryPsd(archive, index, status);
        if (status == enum_checkEntryPsd::RIGHT) {
            break;
        } else if (status != enum_checkEntryPsd::NOTCHECK) {
            return false;
        }
    }

    return true;
}

bool LibzipPlugin::extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options)
{
    this->extractPsdStatus = ReadOnlyArchiveInterface::NotChecked;
    //reset member variable ifReplace;
    ifReplaceTip = false;
    m_extractDestDir = destinationDirectory;
    destDirName = extractTopFolderName;

    bAnyFileExtracted = false;

    const bool extractAll = files.isEmpty();    // true:"解压"; false:"提取"
    const bool removeRootNode = options.isDragAndDropEnabled();
    m_extractionOptions = options;

    int errcode = 0;
    zip_error_t err;

    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(filename()).constData(), ZIP_RDONLY, &errcode);
    zip_error_init_with_code(&err, errcode);
    if (archive == nullptr) {
//        emit error(("Failed to open archive: %1"));
//        return false;
        return minizip_extractFiles(files, destinationDirectory, options);
    }

    int iCodecIndex = 0;

    // Set password if known.
    if (!password().isEmpty()) {
        zip_set_default_password(archive, passwordUnicode(password(), iCodecIndex));
    }

    // Get number of archive entries.
    const qlonglong nofEntries = extractAll ? zip_get_num_entries(archive, 0) : m_listExtractIndex.size();
    //首次检测密码
    bool status = true;
    if (extractAll == true) {
        status = checkArchivePsd(archive, iCodecIndex);
    } else {
        status = checkEntriesPsd(archive, m_listExtractIndex);
    }

    if (status == false) {
        return false;
    }
    //检测完毕,if psd right or no need psd,emit signal to show progress view.

    emit sigExtractPwdCheckDown();
    m_isckeckpsd = true;
    // Extract entries.
    m_overwriteAll = false; // Whether to overwrite all files
    m_skipAll = false; // Whether to skip all files
    if (extractAll) {
        // We extract all entries.
        QString extractDst;
        for (qlonglong i = 0; i < nofEntries; i++) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }

            QString strfileNameTemp = m_common->trans2uft8(zip_get_name(archive, i, ZIP_FL_ENC_RAW));
            //QString strfileNameTemp = zip_get_name(archive, i, ZIP_FL_ENC_RAW);

            if (i == 0) {
                extractDst = QDir::fromNativeSeparators(strfileNameTemp);
            } else if (extractDst.isEmpty() == false) {
                if (QDir::fromNativeSeparators(strfileNameTemp).startsWith(extractDst + (extractDst.endsWith("/") ? "" : "/")) == false) {
                    extractDst.clear();
                }
            }

            emit progress_filename(strfileNameTemp);

            FileProgressInfo pi;

            if (nofEntries < 5) {
                pi.fileName = strfileNameTemp;
                pi.fileProgressProportion = float(1.0) / float(nofEntries);
                pi.fileProgressStart = pi.fileProgressProportion * float(i);
            }

            QString entryName = QDir::fromNativeSeparators(strfileNameTemp);
            if (i == 0) {
                destDirName = entryName;
            }

            enum_extractEntryStatus enumRes = extractEntry(archive,
                                                           i,
                                                           entryName,
                                                           QString(),
                                                           destinationDirectory,
                                                           options.preservePaths(),
                                                           removeRootNode, pi);
            if (enumRes == enum_extractEntryStatus::FAIL) {
                zip_close(archive);
                return false;
            } else if (enumRes == enum_extractEntryStatus::SUCCESS) {
                emit progress(float(i + 1) / nofEntries);
            } else if (enumRes == enum_extractEntryStatus::PSD_NEED) {// need input psd and extractEntry again, added by hsw 20200613
                PasswordNeededQuery query(entryName);
                emit userQuery(&query);
                query.waitForResponse();

                if (query.responseCancelled()) {
                    setPassword(QString());
                    zip_close(archive);
                    emit error("Wrong password.");
                    return false;
                } else {
                    setPassword(query.password());
                    zip_set_default_password(archive, password().toUtf8().constData());
                    i--;
                }
            }
        }

        if (extractDst.isEmpty() == false) {
            emit updateDestFileSignal(destinationDirectory + "/" + extractDst);
        }
    } else {
        // We extract only the entries in files.
        //qulonglong i = 0;
        for (/*int index : m_listExtractIndex*/int i = 0; i < m_listExtractIndex.count(); i++) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }

            FileProgressInfo pi;

            QString strFilePath = m_common->trans2uft8(zip_get_name(archive, m_listExtractIndex[i], ZIP_FL_ENC_RAW));
            const QStringList pieces = strFilePath.split(QLatin1Char('/'), QString::SkipEmptyParts);
            QString strName = pieces.isEmpty() ? QString() : pieces.last();
            emit progress_filename(strName);

            if (nofEntries < 5) {
                pi.fileName = strFilePath;
                pi.fileProgressProportion = float(1.0) / float(nofEntries);
                pi.fileProgressStart = pi.fileProgressProportion * float(i);
            }

            //QString entryName = e->fullPath();
            QString entryName = strFilePath;

            enum_extractEntryStatus enumRes = extractEntry(archive,
                                                           m_listExtractIndex[i],
                                                           strFilePath,
                                                           m_strRootNode,
                                                           destinationDirectory,
                                                           options.preservePaths(),
                                                           removeRootNode, pi);
            if (enumRes == enum_extractEntryStatus::FAIL) {
                zip_close(archive);
                return false;
            } else if (enumRes == enum_extractEntryStatus::SUCCESS) {
                emit progress(float(i + 1) / nofEntries);
            } else if (enumRes == enum_extractEntryStatus::PSD_NEED) {// need input psd and extractEntry again, added by hsw 20200613
                PasswordNeededQuery query(entryName);
                emit userQuery(&query);
                query.waitForResponse();

                if (query.responseCancelled()) {
                    setPassword(QString());
                    zip_close(archive);
                    emit error("Wrong password.");
                    return false;
                } else {
                    setPassword(query.password());
                    zip_set_default_password(archive, password().toUtf8().constData());
                    i--;
                }
            }
        }
    }

    zip_close(archive);
    return true;
}

enum_extractEntryStatus LibzipPlugin::extractEntry(zip_t *archive, int index,  const QString &entry, const QString &rootNode, const QString &destDir, bool preservePaths, bool removeRootNode, FileProgressInfo &pi)
{
    //extract = false;

    userCancel = false;

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
            return enum_extractEntryStatus::SUCCESS;
        }

        destination = destDirCorrected + QFileInfo(entry).fileName();
    }

    // Store parent mtime.
    QString parentDir;
    QFileInfo fileInfo = QFileInfo(destination);
    if (isDirectory) {
        QDir pDir = fileInfo.dir();
        pDir.cdUp();
        parentDir = pDir.path();
    } else {
        parentDir = fileInfo.path();
    }
    // For top-level items, don't restore parent dir mtime.

    const bool restoreParentMtime = (parentDir + QDir::separator() != destDirCorrected);

    time_t parent_mtime = time_t();
    if (restoreParentMtime) {
        parent_mtime = QFileInfo(parentDir).lastModified().toMSecsSinceEpoch() / 1000;
    }

    // Create parent directories for files. For directories create them.
    if (QDir().exists(fileInfo.path()) == false) {
        //extract = true;
        if (!QDir().mkpath(fileInfo.path())) {
            emit error(("Failed to create directory: %1"));
            return enum_extractEntryStatus::FAIL;
        }

        bAnyFileExtracted = true;
    }

    // Get statistic for entry. Used to get entry size and mtime.
    zip_stat_t statBuffer;

    int errcode2 = ZIP_ER_OK;
    errcode2 = zip_error_code_zip(zip_get_error(archive));
    if (zip_stat_index(archive, index, 0, &statBuffer) != 0) {
        if (isDirectory && errcode2 == ZIP_ER_NOENT) {
            return enum_extractEntryStatus::SUCCESS;
        }

        return enum_extractEntryStatus::FAIL;
    }

    if (!isDirectory) {
        // Handle existing destination files.
        QString renamedEntry = entry;
        while (!m_overwriteAll && QFileInfo::exists(destination)) {
            if (m_skipAll) {
                return enum_extractEntryStatus::SUCCESS;
            } else {
                OverwriteQuery query(renamedEntry);
                emit userQuery(&query);
                query.waitForResponse();
                ifReplaceTip = true;

                if (query.responseCancelled()) {
                    userCancel = true;
                    emit cancelled();
                    return enum_extractEntryStatus::FAIL;
                } else if (query.responseSkip()) {
                    return enum_extractEntryStatus::SUCCESS;
                } else if (query.responseAutoSkip()) {
                    m_skipAll = true;
                    return enum_extractEntryStatus::SUCCESS;
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

        m_extractFile = destination;

//        if (!m_extractionOptions.isAutoCreatDir()) {
//            // QFileInfo extractileInfo(destination);
//            QDir dir;
//            if (!dir.exists(m_extractDestDir + "/.extractCache")) {
//                qDebug() << dir.mkdir(m_extractDestDir + "/.extractCache");
//            }

//            qDebug() << QFile::rename(fileInfo.absoluteFilePath(), m_extractDestDir + "/.extractCache/" + fileInfo.fileName());
//        }

        QString strtmp = destination;
        if (strtmp.endsWith(QDir::separator())) {
            strtmp.chop(1);
        }

        int i = strtmp.lastIndexOf(QDir::separator());
        if (strtmp.mid(i + 1).toUtf8().length() > NAME_MAX) { //Is the file name too long
            emit error("Filename is too long");
            return enum_extractEntryStatus::FAIL;
        }

        QFile file(destination);
        if (file.exists() && !file.isWritable()) {
            file.remove();
            file.setFileName(destination);
            file.setPermissions(QFileDevice::WriteUser);
        }

        if (file.open(QIODevice::WriteOnly) == false) {
            emit error(("Failed to open file for writing: %1"));
            return enum_extractEntryStatus::FAIL;
        }

        QDataStream out(&file);

        // Write archive entry to file. We use a read/write buffer of 1024 chars.
        int kb = 1024;
        qulonglong sum = 0;
        char buf[kb];

        if (pi.fileProgressProportion > 0) {
            emit progress(pi.fileProgressStart + pi.fileProgressProportion * 0.01);
        }

        // check entry psd begin : 1st clean error, 2nd fopen, 3rd get error
        zip_error_clear(archive);
        zip_file_t *zipFile = zip_fopen_index(archive, index, 0);
        if (zipFile == nullptr) {
            int iErr = zip_error_code_zip(zip_get_error(archive));
            if (iErr == ZIP_ER_WRONGPASSWD) {//密码错误
                bool bCheckFinished = false;
                int iCodecIndex = 0;
                while (zipFile == nullptr && bCheckFinished == false) {
                    if (iCodecIndex == m_listCodecs.length()) {
                        bCheckFinished = true;
                        if (file.exists()) {
                            file.remove();
                        }

                        file.close();
                        return enum_extractEntryStatus::PSD_NEED;
                    } else {
                        iCodecIndex++;
                        zip_set_default_password(archive, passwordUnicode(password(), iCodecIndex));
                        zip_error_clear(archive);
                        zipFile = zip_fopen_index(archive, index, 0);
                        iErr = zip_error_code_zip(zip_get_error(archive));
                        if (iErr != ZIP_ER_WRONGPASSWD && zipFile != nullptr) {//密码正确
                            bCheckFinished = true;
                        }
                    }
                }
            } else if (iErr == ZIP_ER_NOPASSWD) {
                file.close();
                return enum_extractEntryStatus::PSD_NEED;
            } else {
                emit error(("Failed to read data for entry: %1"));
                file.close();
                return enum_extractEntryStatus::FAIL;
            }
        }
        // check entry psd end .

        int writeSize = 0;
        while (sum != statBuffer.size) {
            if (this->extractPsdStatus == ReadOnlyArchiveInterface::Canceled) { //if have canceled the extraction,so break.
                break;
            }
            if (m_isPause) { //解压暂停
                sleep(1);
                qDebug() << "pause";
                continue;
            }

            const auto readBytes = zip_fread(zipFile, buf, kb);
            if (readBytes < 0) {
                emit error(("Failed to read data for entry: %1"));
                file.close();
                zip_fclose(zipFile);
                return enum_extractEntryStatus::FAIL;
            }

            if (out.writeRawData(buf, readBytes) != readBytes) {
                emit error(("Failed to write data for entry: %1"));
                file.close();
                zip_fclose(zipFile);
                return enum_extractEntryStatus::FAIL;
            }

            sum += readBytes;
            writeSize += readBytes;

            if (pi.fileProgressProportion > 0 && writeSize > statBuffer.size * 0.2/* / 5*/) {
                pi.fileProgressStart += pi.fileProgressProportion * 0.2;
                emit progress(pi.fileProgressStart);
                writeSize = 0;
            }
        }

        slotRestoreWorkingDir();

        //const auto index = zip_name_locate(archive, name.constData(), ZIP_FL_ENC_RAW);
        if (index == -1) {
            emit error(("Failed to locate entry: %1"));
            file.close();
            zip_fclose(zipFile);
            return enum_extractEntryStatus::FAIL;
        }

        zip_uint8_t opsys;
        zip_uint32_t attributes;
        if (zip_file_get_external_attributes(archive, index, ZIP_FL_UNCHANGED, &opsys, &attributes) == -1) {
            emit error(("Failed to read metadata for entry: %1"));
            file.close();
            zip_fclose(zipFile);
            return enum_extractEntryStatus::FAIL;
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
        file.setPermissions(getPermissions(attributes >> 16));
        //extract = true;
        bAnyFileExtracted = true;

        file.close();
        zip_fclose(zipFile);
    } else {
        //const auto index = zip_name_locate(archive, name.constData(), ZIP_FL_ENC_RAW);
        if (index == -1) {
            emit error(("Failed to locate entry: %1"));
            return enum_extractEntryStatus::FAIL;
        }

        zip_uint8_t opsys;
        zip_uint32_t attributes;
        if (zip_file_get_external_attributes(archive, index, ZIP_FL_UNCHANGED, &opsys, &attributes) == -1) {
            emit error(("Failed to read metadata for entry: %1"));
            return enum_extractEntryStatus::FAIL;
        }

        mode_t value = attributes >> 16;
        QFileDevice::Permissions per = getPermissions(value);
        //if (value == 0)
        per = per | QFileDevice::ReadUser | QFileDevice::WriteUser | QFileDevice::ExeUser ;
        QFile::setPermissions(destination, per);
    }

    // Set mtime for entry.
    utimbuf times = utimbuf();
    times.modtime = statBuffer.mtime;
    if (utime(destination.toUtf8().constData(), &times) != 0) {
    }

    if (restoreParentMtime) {
        // Restore mtime for parent dir.
        times.modtime = parent_mtime;
        if (utime(parentDir.toUtf8().constData(), &times) != 0) {
        }
    }

    return enum_extractEntryStatus::SUCCESS;
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
        emit error(("Failed to open archive: %1"));
        return false;
    }

    QStringList filePaths = entryFullPaths(files);
    filePaths.sort();
    const QStringList destPaths = entryPathsFromDestination(filePaths, destination, entriesWithoutChildren(files).count());

    int i;
    for (i = 0; i < filePaths.size(); ++i) {

        const int index = zip_name_locate(archive, filePaths.at(i).toUtf8().constData(), ZIP_FL_ENC_GUESS);
        if (index == -1) {
            if (zip_close(archive)) {
                emit error(("Failed to write archive."));
                return false;
            }

            emit error(("Failed to move entry: %1"));
            return false;
        }

        if (zip_file_rename(archive, index, destPaths.at(i).toUtf8().constData(), ZIP_FL_ENC_GUESS) == -1) {
            if (zip_close(archive)) {
                emit error(("Failed to write archive."));
                return false;
            }

            emit error(("Failed to move entry: %1"));
            return false;
        }

        emit entryRemoved(filePaths.at(i));
        emitEntryForIndex(archive, index);
        emit progress(i / filePaths.count());
    }

    if (zip_close(archive)) {
        emit error(("Failed to write archive."));
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
        emit error(("Failed to open archive: %1"));
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
            if (zip_close(archive)) {
                emit error(("Failed to write archive."));
                return false;
            }
            emit error(("Failed to copy entry: %1"));
            return false;
        }

        zip_source_t *src = zip_source_zip(archive, archive, srcIndex, 0, 0, -1);
        if (!src) {
            if (zip_close(archive)) {
                emit error(("Failed to write archive."));
                return false;
            }

            return false;
        }

        const int destIndex = zip_file_add(archive, dest.toUtf8().constData(), src, ZIP_FL_ENC_GUESS | ZIP_FL_OVERWRITE);
        if (destIndex == -1) {
            zip_source_free(src);
            emit error(("Failed to add entry: %1"));
            if (zip_close(archive)) {
                emit error(("Failed to write archive."));
                return false;
            }

            return false;
        }

        // Get permissions from source entry.
        zip_uint8_t opsys;
        zip_uint32_t attributes;
        if (zip_file_get_external_attributes(archive, srcIndex, ZIP_FL_UNCHANGED, &opsys, &attributes) == -1) {
            emit error(("Failed to read metadata for entry: %1"));
            if (zip_close(archive)) {
                emit error(("Failed to write archive."));
                return false;
            }

            return false;
        }

        // Set permissions on dest entry.
        if (zip_file_set_external_attributes(archive, destIndex, ZIP_FL_UNCHANGED, opsys, attributes) != 0) {
            emit error(("Failed to set metadata for entry: %1"));
            if (zip_close(archive)) {
                emit error(("Failed to write archive."));
                return false;
            }

            return false;
        }
    }

    // Register the callback function to get progress feedback.
    zip_register_progress_callback_with_state(archive, 0.001, progressCallback, nullptr, this);

    if (zip_close(archive)) {
        emit error(("Failed to write archive."));
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

void LibzipPlugin::cleanIfCanceled()
{
//    qDebug() << "cleanIfCanceled";
//    if (this->ifReplaceTip == false) {
//        if (this->extractPsdStatus == ReadOnlyArchiveInterface::Canceled) {
//            if (this->destDirName == "" || this->m_extractDestDir == "") {
//                return;
//            }
//            QString fullPath = m_extractDestDir + QDir::separator() + this->destDirName;
//            QFileInfo fileInfo(fullPath);
//            if (fileInfo.exists()) {
//                ReadWriteArchiveInterface::clearPath(fullPath);
//            }
//        }
//    }

//    QDir dir;
//    dir.setPath(m_extractDestDir + "/.extractCache");
//    if (dir.exists()) {
//        qDebug() << dir.removeRecursively();
//    }
}

void LibzipPlugin::watchFileList(QStringList */*strList*/)
{

}

void LibzipPlugin::slotRestoreWorkingDir()
{
    if (this->extractPsdStatus == ReadOnlyArchiveInterface::Canceled) {
        qDebug() << "=====点击了取消";

//        if (this->ifReplaceTip == true) {
//            if (!m_extractionOptions.isAutoCreatDir()) {
//                QFileInfo fileInfo(m_extractFile);
//                QString strFileName = fileInfo.fileName();
//                QDir dir;

//                qDebug() << QFile::remove(m_extractFile);
//                qDebug() << QFile::exists(m_extractDestDir + "/.extractCache/" + strFileName);
//                qDebug() << dir.exists(m_extractDestDir + "/.extractCache");
//                qDebug() << QFile::exists(m_extractDestDir + "/.extractCache/" + strFileName);
//                qDebug() << m_extractDestDir + "/.extractCache/" + fileInfo.fileName();
//                qDebug() << m_extractDestDir + "/" + fileInfo.fileName();
//                qDebug() << QFile::rename(m_extractDestDir + "/.extractCache/" + strFileName, m_extractDestDir + "/" + strFileName);
//            }
//        }
    }
}

void LibzipPlugin::showEntryListFirstLevel(const QString &directory)
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
                    Archive::Entry *fileEntry = setEntryDataA(iter.value().first, iter.value().second, iter.key());
                    RefreshEntryFileCount(fileEntry);
                    emit entry(fileEntry);
//                    m_emittedEntries << fileEntry;
                }
            }

            ++iter;
        }
    }
}

void LibzipPlugin::RefreshEntryFileCount(Archive::Entry *file)
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

qint64 LibzipPlugin::extractSize(const QVector<Archive::Entry *> &files)
{
    m_listExtractIndex.clear();
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
                    qExtractSize += iter.value().first.size;
                }

                int iIndex = iter.value().second;
                if (iIndex >= 0) {
                    m_listExtractIndex << iIndex;
                }

                ++iter;
                if (!strPath.endsWith(QDir::separator())) {
                    break;
                }
            }
        }
    }

    std::sort(m_listExtractIndex.begin(), m_listExtractIndex.end());

    return qExtractSize;

}

bool LibzipPlugin::minizip_list(bool /*isbatch*/)
{
    QString fileName = filename();

    // Open the zip file
    unzFile zipfile = unzOpen(fileName.toUtf8().constData());
    if (zipfile == nullptr) {
        emit error(("Failed to open archive: %1"));
        return false;
    }

    // Get info about the zip file
    unz_global_info global_info;
    if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK) {
        emit error(("could not read file global info"));
        unzClose(zipfile);
    }

    // Loop to extract all files
    uLong i;
    for (i = 0; i < global_info.number_entry; ++i) {
        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        minizip_emitEntryForIndex(zipfile);

        if ((i + 1) < global_info.number_entry) {
            if (unzGoToNextFile(zipfile) != UNZ_OK) {
                qDebug() << ("cound not read next file\n");
                //unzClose(zipfile);
                //return -1;
            }
        }
    }

    unzClose(zipfile);
    m_listAfterAdd = false;
    return true;
}

bool LibzipPlugin::minizip_emitEntryForIndex(unzFile zipfile)
{
    unz_file_info file_info;

    char filename[ MAX_FILENAME ];

    if (unzGetCurrentFileInfo(zipfile, &file_info, filename, MAX_FILENAME, nullptr, 0, nullptr, 0) != UNZ_OK) {
        qDebug() << ("could not read file info\n");
        return false;
    }

    QString strFileName = filename;
    auto e = new Archive::Entry();
    QDateTime datetime;
    datetime.setDate(QDate(file_info.tmu_date.tm_year, file_info.tmu_date.tm_mon + 1, file_info.tmu_date.tm_mday));
    datetime.setTime(QTime(file_info.tmu_date.tm_hour, file_info.tmu_date.tm_min, file_info.tmu_date.tm_sec));
    //qDebug() << i << filename << datetime.toString("yyyy-MM-dd hh:mm:ss") << file_info.flag << file_info.uncompressed_size;
    e->setFullPath(m_common->trans2uft8(filename));
    e->setProperty("isDirectory", strFileName.endsWith(QDir::separator()));
    e->setProperty("timestamp", datetime);
    e->setProperty("size", (qulonglong)file_info.uncompressed_size);
    e->setProperty("compressedSize", (qlonglong)file_info.compressed_size);
    if (!e->isDir()) {
        e->setProperty("CRC", QString::number((qulonglong)file_info.crc, 16).toUpper());
    }

    switch (file_info.compression_method) {
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

    emit entry(e);
//    m_emittedEntries << e;

    return true;
}

bool LibzipPlugin::minizip_extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options)
{
    const bool extractAll = files.isEmpty();
    const bool removeRootNode = options.isDragAndDropEnabled();

    // Open the zip file
    unzFile zipfile = unzOpen(filename().toUtf8().constData());
    if (zipfile == nullptr) {
        emit error(("Failed to open archive: %1"));
        return false;
    }

    // Get info about the zip file
    unz_global_info global_info;
    if (unzGetGlobalInfo(zipfile, &global_info) != UNZ_OK) {
        emit error(("could not read file global info"));
        unzClose(zipfile);
    }

    // Loop to extract files
    qlonglong nofEntries = extractAll ? global_info.number_entry : files.size();
    // Extract entries.
    m_overwriteAll = false; // Whether to overwrite all files
    m_skipAll = false; // Whether to skip all files

    if (extractAll) {
        // Loop to extract all files
        qlonglong i;
        QString extractDst;
        for (i = 0; i < nofEntries; ++i) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }

            unz_file_info file_info;
            char filename[ MAX_FILENAME ];
            if (unzGetCurrentFileInfo(zipfile, &file_info, filename, MAX_FILENAME, nullptr, 0, nullptr, 0) != UNZ_OK) {
                unzClose(zipfile);
                return false;
            }

            QString strfileNameTemp = m_common->trans2uft8(filename);

            if (i == 0) {
                extractDst = QDir::fromNativeSeparators(strfileNameTemp);
            } else if (extractDst.isEmpty() == false) {
                if (QDir::fromNativeSeparators(strfileNameTemp).startsWith(extractDst + (extractDst.endsWith("/") ? "" : "/")) == false) {
                    extractDst.clear();
                }
            }

            emit progress_filename(strfileNameTemp);

            FileProgressInfo pi;

            if (nofEntries < 5) {
                pi.fileName = strfileNameTemp;
                pi.fileProgressProportion = float(1.0) / float(nofEntries);
                pi.fileProgressStart = pi.fileProgressProportion * float(i);
            }

            QString entryName = QDir::fromNativeSeparators(strfileNameTemp);
            if (i == 0) {
                destDirName = entryName;
            }

            if (!minizip_extractEntry(zipfile, file_info, strfileNameTemp, QString(), destinationDirectory, options.preservePaths(), removeRootNode, pi)) {
                unzClose(zipfile);
                return false;
            }

            emit progress(float(i + 1) / nofEntries);

            // Go the the next entry listed in the zip file.
            if ((i + 1) < nofEntries) {
                if (unzGoToNextFile(zipfile) != UNZ_OK) {
//                    qDebug() << "cound not read next file\n";
//                    unzClose(zipfile);
//                    return false;
                    continue;
                }
            }
        }

        if (extractDst.isEmpty() == false) {
            emit updateDestFileSignal(destinationDirectory + "/" + extractDst);
        }
    } else {
        qlonglong i = 0;
        QVector<Archive::Entry *> vecFiles;         // 不包括文件夹
        for (Archive::Entry *e : files) {
            if (!e->fullPath().endsWith(QDir::separator())) {
                vecFiles << e;
            }
        }

        nofEntries = vecFiles.size();

        if (vecFiles.count() > 0) {
//            QString strFileName = files[0]->fullPath();
//            if (files[0]->fullPath().endsWith(QDir::separator()))
//                strFileName.chop(1);
            qDebug() << unzLocateFile(zipfile, m_common->trans2uft8(vecFiles[0]->fullPath().toUtf8().data()).toLatin1(), 0);
        }

        for (const Archive::Entry *e : vecFiles) {
            if (QThread::currentThread()->isInterruptionRequested()) {
                break;
            }

            unz_file_info file_info;
            char filename[ MAX_FILENAME ];
            if (unzGetCurrentFileInfo(zipfile, &file_info, filename, MAX_FILENAME, nullptr, 0, nullptr, 0) != UNZ_OK) {
                unzClose(zipfile);
                return false;
            }

            FileProgressInfo pi;

            QString entryName = e->fullPath();
            emit progress_filename(e->name());

            if (nofEntries < 5) {
                pi.fileName = entryName;
                pi.fileProgressProportion = float(1.0) / float(nofEntries);
                pi.fileProgressStart = pi.fileProgressProportion * float(i);
            }

            if (!minizip_extractEntry(zipfile, file_info, entryName, e->rootNode, destinationDirectory, options.preservePaths(), removeRootNode, pi)) {
                unzClose(zipfile);
                return false;
            }

            emit progress(float(i + 1) / nofEntries);

            // Go the the next entry listed in the zip file.
            if ((++i) < nofEntries) {
//                QString strFileName = files[i]->fullPath();
//                if (files[i]->fullPath().endsWith(QDir::separator()))
//                    strFileName.chop(1);
                int error = unzLocateFile(zipfile, m_common->trans2uft8(vecFiles[i]->fullPath().toUtf8().data()).toLatin1(), 0);
                if (error != UNZ_OK) {
//                    qDebug() << "cound not read next file\n";
//                    unzClose(zipfile);
//                    return false;
                    continue;
                }
            }

        }
    }

    unzClose(zipfile);
    return true;
}

bool LibzipPlugin::minizip_extractEntry(unzFile zipfile, unz_file_info file_info, const QString &entry, const QString &rootNode, const QString &destDir, bool preservePaths, bool removeRootNode, FileProgressInfo &pi)
{
    //extract = false;
    userCancel = false;

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
    QFileInfo fileInfo = QFileInfo(destination);
    if (isDirectory) {
        QDir pDir = fileInfo.dir();
        pDir.cdUp();
        parentDir = pDir.path();
    } else {
        parentDir = fileInfo.path();
    }
    // For top-level items, don't restore parent dir mtime.
//    const bool restoreParentMtime = (parentDir + QDir::separator() != destDirCorrected);

//    time_t parent_mtime = time_t();
//    if (restoreParentMtime) {
//        parent_mtime = QFileInfo(parentDir).lastModified().toMSecsSinceEpoch() / 1000;
//    }

    // Create parent directories for files. For directories create them.
    if (QDir().exists(fileInfo.path()) == false) {
        //extract = true;
        if (!QDir().mkpath(fileInfo.path())) {
            emit error(("Failed to create directory: %1"));
            return false;
        }

        bAnyFileExtracted = true;
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
                ifReplaceTip = true;

                if (query.responseCancelled()) {
                    userCancel = true;
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

        m_extractFile = destination;

//        if (!m_extractionOptions.isAutoCreatDir()) {
//            // QFileInfo extractileInfo(destination);
//            QDir dir;
//            if (!dir.exists(m_extractDestDir + "/.extractCache")) {
//                qDebug() << dir.mkdir(m_extractDestDir + "/.extractCache");
//            }

//            qDebug() << QFile::rename(fileInfo.absoluteFilePath(), m_extractDestDir + "/.extractCache/" + fileInfo.fileName());

//        }

        QString strtmp = destination;
        if (strtmp.endsWith(QDir::separator())) {
            strtmp.chop(1);
        }

        int i = strtmp.lastIndexOf(QDir::separator());
        if (strtmp.mid(i + 1).toUtf8().length() > NAME_MAX) { //Is the file name too long
            emit error("Filename is too long");
            return false;
        }

        QFile file(destination);
        if (file.exists() && !file.isWritable()) {
            file.setPermissions(QFileDevice::WriteUser);
        }

        if (file.open(QIODevice::WriteOnly) == false) {
            emit error(("Failed to open file for writing: %1"));
            return false;
        }

        QDataStream out(&file);

        // Write archive entry to file. We use a read/write buffer of 1024 chars.
        int kb = 1024;
        qulonglong sum = 0;
        char buf[kb];

        if (pi.fileProgressProportion > 0) {
            emit progress(pi.fileProgressStart + pi.fileProgressProportion * 0.01);
        }

        int iError = unzOpenCurrentFile(zipfile);
        qDebug() << "Error" << iError;
        if (iError != UNZ_OK) {
            unzClose(zipfile);
            return false;
        }

        int writeSize = 0;
        while (sum != file_info.uncompressed_size) {
            if (this->extractPsdStatus == ReadOnlyArchiveInterface::Canceled) { //if have canceled the extraction,so break.
                break;
            }
            if (m_isPause) { //解压暂停
                sleep(1);
                qDebug() << "pause";
                continue;
            }

            const auto readBytes = unzReadCurrentFile(zipfile, buf, kb);
            if (readBytes < 0) {
                emit error(("Failed to read data for entry: %1"));
                file.close();
                return false;
            }

            if (out.writeRawData(buf, readBytes) != readBytes) {
                emit error(("Failed to write data for entry: %1"));
                file.close();
                return false;
            }

            sum += readBytes;
            writeSize += readBytes;

            if (pi.fileProgressProportion > 0 && writeSize > file_info.uncompressed_size * 0.2/*/ 5*/) {
                pi.fileProgressStart += pi.fileProgressProportion * 0.2;
                emit progress(pi.fileProgressStart);
                writeSize = 0;
            }
        }

        slotRestoreWorkingDir();
        //fclose(out);
        file.close();
        bAnyFileExtracted = true;
    }

    unzCloseCurrentFile(zipfile);
    return true;
}

Archive::Entry *LibzipPlugin::setEntryData(const zip_stat_t &statBuffer, qlonglong index, const QString &name, bool isMutilFolderFile)
{
    auto e = new Archive::Entry(/*this*/);

    // e->setCompressIndex(index);

    if (statBuffer.valid & ZIP_STAT_NAME) {
        // e->setFullPath(statBuffer.name);
        e->setFullPath(name);
    }

    if (e->fullPath(PathFormat::WithTrailingSlash).endsWith(QDir::separator())) {
        e->setProperty("isDirectory", true);
    }

    if (statBuffer.valid & ZIP_STAT_MTIME) {
        e->setProperty("timestamp", QDateTime::fromTime_t(statBuffer.mtime));
    }

    if (statBuffer.valid & ZIP_STAT_SIZE) {
        if (!isMutilFolderFile) {
            e->setProperty("size", (qulonglong)statBuffer.size);
        } else {
            e->setProperty("size", 0);
        }
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

    emit entry(e);
//    m_emittedEntries << e;
    return e;
}

Archive::Entry *LibzipPlugin::setEntryDataA(const zip_stat_t &statBuffer, qlonglong index, const QString &name)
{
    auto e = new Archive::Entry(this);

    // e->setCompressIndex(index);

    if (statBuffer.valid & ZIP_STAT_NAME) {
        // e->setFullPath(statBuffer.name);
        e->setFullPath(name);
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

    return e;
}

void LibzipPlugin::setEntryVal(const zip_stat_t &statBuffer, int &index, const QString &name, QString &dirRecord)
{
    if (dirRecord.isEmpty()) {
        if (name.endsWith("/") && name.count("/") == 1) {
            setEntryData(statBuffer, index, name);
            m_SigDirRecord = name;
            ++index;
            // m_DirRecord = name;
        } else  if (name.endsWith("/") && name.count("/") > 1) {
            if (!m_SigDirRecord.isEmpty() && name.left(m_SigDirRecord.size()) == m_SigDirRecord) {
                setEntryData(statBuffer, index, name);
                ++index;
                return;
            }

            //Create FileFolder
            QStringList fileDirs = name.split("/");
            QString folderAppendStr = "";
            for (int i = 0 ; i < fileDirs.size() - 1; ++i) {
                folderAppendStr += fileDirs[i] + "/";
                setEntryData(statBuffer, index, folderAppendStr);
                m_listMap.insert(folderAppendStr, qMakePair(statBuffer, -1));
            }

            ++index;
            m_DirRecord = name;
        } else if (name.count("/") == 0) {
            setEntryData(statBuffer, index, name);
            ++index;
        } else if (!name.endsWith("/") && name.count("/") >= 1) {
            if (!m_SigDirRecord.isEmpty() && (name.left(m_SigDirRecord.size()) == m_SigDirRecord)) {
                return;
            } else if (!m_DirRecord.isEmpty() && (name.left(m_DirRecord.size()) == m_DirRecord)) {
                return;
            }

            //Create FileFolder and file
            QStringList fileDirs = name.split("/");
            QString folderAppendStr = "";
            for (int i = 0 ; i <  fileDirs.size() ; ++i) {
                if (i < fileDirs.size() - 1) {
                    folderAppendStr.append(fileDirs[i]).append("/");
                    setEntryData(statBuffer, index, folderAppendStr, true);
                    m_listMap.insert(folderAppendStr, qMakePair(statBuffer, -1));
                } else {
                    folderAppendStr.append(fileDirs[i]);
                    //setEntryData(statBuffer, index, folderAppendStr, false);
                }
            }

            ++index;
        }
    } else {
//        if (name.left(m_DirRecord.size()) == m_DirRecord) {
//            setEntryData(statBuffer, index, name);
//            ++index;
//        } else {
//            m_DirRecord = "";
//            setEntryVal(statBuffer, index, name, m_DirRecord);
//        }
        m_DirRecord = "";
        setEntryVal(statBuffer, index, name, m_DirRecord);
    }
}

void LibzipPlugin::updateListMap()
{
    m_listMap.clear();

    int errcode = 0;
    // Open archive.
    zip_t *archive = zip_open(QFile::encodeName(filename()).constData(), ZIP_RDONLY, &errcode);

    // Get number of archive entries.
    const auto nofEntries = zip_get_num_entries(archive, 0);

    for (int i = 0; i < nofEntries; i++) {

        if (QThread::currentThread()->isInterruptionRequested()) {
            break;
        }

        zip_stat_t statBuffer;
        if (zip_stat_index(archive, i, ZIP_FL_ENC_RAW, &statBuffer)) {
            continue;
        }

        QString name = m_common->trans2uft8(statBuffer.name);
        setEntryVal1(statBuffer, m_indexCount, name, m_DirRecord);

        if (m_listMap.find(name) == m_listMap.end()) {
            m_listMap.insert(name, qMakePair(statBuffer, i));
        }
    }

    zip_close(archive);
}

void LibzipPlugin::setEntryVal1(const zip_stat_t &statBuffer, int &index, const QString &name, QString &dirRecord)
{
    if (dirRecord.isEmpty()) {
        if (name.endsWith("/") && name.count("/") == 1) {
            // setEntryData(statBuffer, index, name);
            m_SigDirRecord = name;
            ++index;
            // m_DirRecord = name;
        } else  if (name.endsWith("/") && name.count("/") > 1) {
            if (!m_SigDirRecord.isEmpty() && name.left(m_SigDirRecord.size()) == m_SigDirRecord) {
                //setEntryData(statBuffer, index, name);
                ++index;
                return;
            }

            //Create FileFolder
            QStringList fileDirs = name.split("/");
            QString folderAppendStr = "";
            for (int i = 0 ; i < fileDirs.size() - 1; ++i) {
                folderAppendStr += fileDirs[i] + "/";
                //setEntryData(statBuffer, index, folderAppendStr);
                m_listMap.insert(folderAppendStr, qMakePair(statBuffer, -1));
            }

            ++index;
            m_DirRecord = name;
        } else if (name.count("/") == 0) {
            //setEntryData(statBuffer, index, name);
            ++index;
        } else if (!name.endsWith("/") && name.count("/") >= 1) {
            if (!m_SigDirRecord.isEmpty() && (name.left(m_SigDirRecord.size()) == m_SigDirRecord)) {
                return;
            } else if (!m_DirRecord.isEmpty() && (name.left(m_DirRecord.size()) == m_DirRecord)) {
                return;
            }

            //Create FileFolder and file
            QStringList fileDirs = name.split("/");
            QString folderAppendStr = "";
            for (int i = 0 ; i <  fileDirs.size() ; ++i) {
                if (i < fileDirs.size() - 1) {
                    folderAppendStr.append(fileDirs[i]).append("/");
                    //setEntryData(statBuffer, index, folderAppendStr, true);
                    m_listMap.insert(folderAppendStr, qMakePair(statBuffer, -1));
                } else {
                    folderAppendStr.append(fileDirs[i]);
                    //setEntryData(statBuffer, index, folderAppendStr, false);
                }
            }

            ++index;
        }
    } else {
//        if (name.left(m_DirRecord.size()) == m_DirRecord) {
//            setEntryData(statBuffer, index, name);
//            ++index;
//        } else {
//            m_DirRecord = "";
//            setEntryVal(statBuffer, index, name, m_DirRecord);
//        }
        m_DirRecord = "";
        setEntryVal1(statBuffer, index, name, m_DirRecord);
    }
}
