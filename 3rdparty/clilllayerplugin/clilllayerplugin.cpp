/*
 * Copyright (C) 2026 UnionTech Software Technology Co., Ltd.
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "clilllayerplugin.h"
#include "datamanager.h"
#include "queries.h"

#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QJsonDocument>
#include <QJsonObject>
#include <QProcess>
#include <QStandardPaths>
#include <QTemporaryDir>
#include <QDebug>
#include <QtEndian>

#include <unistd.h>

namespace {

/** layer 包 40 字节魔数，对齐官方 linglong::package::magicNumber() */
const qint64 kLayerMagicSize = 40;
const char kLayerMagic[kLayerMagicSize] = "<<< deepin linglong layer archive >>>";
// 注：字符串字面量 37 字符 + 编译器补的 '\0'，官方为 leftJustified(40, 0) 补零
static_assert(sizeof(kLayerMagic) == kLayerMagicSize, "layer magic must be 40 bytes");

/** uab 包 meta 信息所在的 ELF section 名，对齐官方 common::uab::metaSection */
const char kUabMetaSection[] = "linglong.meta";

/** meta JSON 尺寸合理上限，防止异常文件导致超大读取 */
const qint64 kMaxMetaSize = 16 * 1024 * 1024;

/** fsck.erofs 解压超时（毫秒），兜底防止异常挂死 */
constexpr int kFsckTimeoutMs = 10 * 60 * 1000;

}   // namespace

LayerPluginFactory::LayerPluginFactory()
{
    registerPlugin<LayerPlugin>();
}

LayerPluginFactory::~LayerPluginFactory()
{
}

LayerPlugin::LayerPlugin(QObject *parent, const QVariantList &args)
    : ReadOnlyArchiveInterface(parent, args)
{
    qDebug() << "LayerPlugin created for:" << m_strArchiveName;
}

LayerPlugin::~LayerPlugin()
{
}

bool LayerPlugin::parseLayerHeader(QFile &file, qint64 *erofsOffset, qint64 *erofsSize, QString *errOut)
{
    // 布局对齐官方 LayerFile: [40B 魔数][4B LE metaInfoLength][LayerInfo JSON][EROFS 至文件尾]
    file.seek(0);
    QByteArray magic = file.read(kLayerMagicSize);
    if (magic.size() != kLayerMagicSize || memcmp(magic.constData(), kLayerMagic, kLayerMagicSize) != 0) {
        *errOut = QStringLiteral("invalid magic number, this is not a linglong layer");
        return false;
    }

    quint32 metaLen = 0;
    QByteArray lenBuf = file.read(4);
    if (lenBuf.size() != 4) {
        *errOut = QStringLiteral("layer file truncated");
        return false;
    }
    metaLen = qFromLittleEndian<quint32>(lenBuf.constData());
    if (metaLen == 0 || metaLen > kMaxMetaSize) {
        *errOut = QStringLiteral("layer meta info length invalid: %1").arg(metaLen);
        return false;
    }

    const qint64 fileSize = file.size();
    const qint64 offset = kLayerMagicSize + 4 + static_cast<qint64>(metaLen);
    if (offset >= fileSize) {
        *errOut = QStringLiteral("layer file truncated (no erofs payload)");
        return false;
    }
    *erofsOffset = offset;
    *erofsSize = fileSize - offset;
    return true;
}

bool LayerPlugin::parseUabHeader(QFile &file, qint64 *erofsOffset, qint64 *erofsSize, QString *errOut)
{
    // 布局对齐官方 UABFile: ELF64 容器 + section "linglong.meta"(JSON, 内含
    // sections.bundle 指向 EROFS 镜像 section) + 可选 .note.uab.sig 签名
    file.seek(0);
    const QByteArray ehdr = file.read(64);
    if (ehdr.size() != 64) {
        *errOut = QStringLiteral("uab file too small");
        return false;
    }
    const uchar *e = reinterpret_cast<const uchar *>(ehdr.constData());
    if (e[0] != 0x7f || e[1] != 'E' || e[2] != 'L' || e[3] != 'F') {
        *errOut = QStringLiteral("invalid magic number, this is not a linglong uab");
        return false;
    }
    // 仅支持 64 位小端（官方 UAB 仅在 64 位小端平台产生）
    if (e[4] != 2 || e[5] != 1) {
        *errOut = QStringLiteral("unsupported uab elf class/encoding");
        return false;
    }
    const quint64 shoff = qFromLittleEndian<quint64>(e + 40);   // e_shoff
    const quint16 shentsize = qFromLittleEndian<quint16>(e + 58);   // e_shentsize
    const quint16 shnum = qFromLittleEndian<quint16>(e + 60);   // e_shnum
    const quint16 shstrndx = qFromLittleEndian<quint16>(e + 62);   // e_shstrndx
    if (shoff == 0 || shentsize != 64 || shnum == 0 || shstrndx >= shnum) {
        *errOut = QStringLiteral("invalid uab elf section header table");
        return false;
    }

    // 读全部 section 头（避免加法回绕：先校验各分量再作差）
    if (shoff > static_cast<quint64>(file.size())
        || static_cast<qint64>(shnum) * shentsize > file.size() - static_cast<qint64>(shoff)) {
        *errOut = QStringLiteral("uab section header table out of range");
        return false;
    }
    file.seek(static_cast<qint64>(shoff));
    QByteArray shdrs = file.read(static_cast<qint64>(shnum) * shentsize);
    if (shdrs.size() != static_cast<qint64>(shnum) * shentsize) {
        *errOut = QStringLiteral("uab section header table truncated");
        return false;
    }
    auto shdrAt = [&shdrs](int idx) -> const uchar * {
        return reinterpret_cast<const uchar *>(shdrs.constData()) + qint64(idx) * 64;
    };

    // 读 section 名字表
    const uchar *strSh = shdrAt(shstrndx);
    const quint64 strOff = qFromLittleEndian<quint64>(strSh + 24);   // sh_offset
    const quint64 strSize = qFromLittleEndian<quint64>(strSh + 32);   // sh_size
    if (strSize == 0 || strSize > quint64(kMaxMetaSize)
        || strSize > static_cast<quint64>(file.size())
        || strOff > static_cast<quint64>(file.size()) - strSize) {
        *errOut = QStringLiteral("invalid uab elf string table");
        return false;
    }
    file.seek(static_cast<qint64>(strOff));
    const QByteArray shstrtab = file.read(static_cast<qint64>(strSize));

    // 按名字取 section 头信息；名字为 shstrtab 中 sh_name 偏移处的 NUL 结尾串，
    // 手动定位 NUL 边界以防恶意 ELF 字符串表无终止符导致越界读
    auto findSection = [&shdrAt, &shstrtab, shnum](const QByteArray &name,
                                                   quint64 *off,
                                                   quint64 *size) -> bool {
        for (int i = 0; i < shnum; ++i) {
            const uchar *sh = shdrAt(i);
            const quint32 nameOff = qFromLittleEndian<quint32>(sh);   // sh_name
            if (nameOff >= static_cast<quint32>(shstrtab.size())) {
                continue;
            }
            const int nameEnd = shstrtab.indexOf('\0', static_cast<int>(nameOff));
            if (nameEnd < 0) {
                continue;
            }
            const QByteArray secName = shstrtab.mid(static_cast<int>(nameOff),
                                                    nameEnd - static_cast<int>(nameOff));
            if (secName != name) {
                continue;
            }
            *off = qFromLittleEndian<quint64>(sh + 24);
            *size = qFromLittleEndian<quint64>(sh + 32);
            return true;
        }
        return false;
    };

    // linglong.meta section → UabMetaInfo JSON
    quint64 metaOff = 0, metaSize = 0;
    if (!findSection(kUabMetaSection, &metaOff, &metaSize) || metaSize == 0
        || metaSize > quint64(kMaxMetaSize) || metaSize > static_cast<quint64>(file.size())
        || metaOff > static_cast<quint64>(file.size()) - metaSize) {
        *errOut = QStringLiteral("uab meta section missing or invalid");
        return false;
    }
    file.seek(static_cast<qint64>(metaOff));
    const QByteArray metaJson = file.read(static_cast<qint64>(metaSize));
    QJsonParseError parseErr;
    const QJsonDocument doc = QJsonDocument::fromJson(metaJson, &parseErr);
    if (!doc.isObject()) {
        *errOut = QStringLiteral("uab meta json invalid: %1").arg(parseErr.errorString());
        return false;
    }
    const QString bundleName = doc.object()
                                 .value(QStringLiteral("sections"))
                                 .toObject()
                                 .value(QStringLiteral("bundle"))
                                 .toString();
    if (bundleName.isEmpty()) {
        *errOut = QStringLiteral("uab meta json has no sections.bundle");
        return false;
    }

    // bundle section 即 EROFS 镜像
    quint64 bundleOff = 0, bundleSize = 0;
    if (!findSection(bundleName.toLatin1(), &bundleOff, &bundleSize) || bundleSize == 0
        || bundleSize > static_cast<quint64>(file.size())
        || bundleOff > static_cast<quint64>(file.size()) - bundleSize) {
        *errOut = QStringLiteral("uab bundle section missing or invalid");
        return false;
    }
    *erofsOffset = static_cast<qint64>(bundleOff);
    *erofsSize = static_cast<qint64>(bundleSize);
    return true;
}

bool LayerPlugin::parseHeader(qint64 *erofsOffset, qint64 *erofsSize, QString *errOut)
{
    QFile file(m_strArchiveName);
    if (!file.open(QIODevice::ReadOnly)) {
        *errOut = QStringLiteral("failed to open %1").arg(m_strArchiveName);
        return false;
    }
    file.seek(0);
    const QByteArray head = file.peek(4);
    if (head.startsWith(QByteArrayLiteral("\x7f""ELF"))) {
        return parseUabHeader(file, erofsOffset, erofsSize, errOut);
    }
    return parseLayerHeader(file, erofsOffset, erofsSize, errOut);
}

bool LayerPlugin::copyPayload(QFile &file, qint64 offset, qint64 size, const QString &dstPath,
                              QString *errOut)
{
    QFile dst(dstPath);
    if (!dst.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        *errOut = QStringLiteral("failed to create %1").arg(dstPath);
        return false;
    }
    if (!file.seek(offset)) {
        *errOut = QStringLiteral("failed to seek to payload offset %1").arg(offset);
        return false;
    }
    constexpr qint64 kChunk = 4 * 1024 * 1024;
    QByteArray buf;
    buf.resize(int(kChunk));
    qint64 remaining = size;
    while (remaining > 0) {
        const qint64 toRead = qMin(kChunk, remaining);
        const qint64 got = file.read(buf.data(), toRead);
        if (got <= 0) {
            *errOut = QStringLiteral("failed to read erofs payload at %1").arg(size - remaining);
            return false;
        }
        if (dst.write(buf.constData(), got) != got) {
            *errOut = QStringLiteral("failed to write %1").arg(dstPath);
            return false;
        }
        remaining -= got;
    }
    if (!dst.flush()) {
        *errOut = QStringLiteral("failed to write %1").arg(dstPath);
        return false;
    }
    return true;
}

QString LayerPlugin::runFsckExtract(const QString &erofsFile, const QString &destDir)
{
    const QString fsck = QStandardPaths::findExecutable(QStringLiteral("fsck.erofs"));
    if (fsck.isEmpty()) {
        return QStringLiteral("fsck.erofs not found, please install erofs-utils");
    }

    QProcess process;
    process.setProgram(fsck);
    // 对齐官方 LayerPackager/UABFile 的 fsck.erofs 兜底分支
    process.setArguments({ QStringLiteral("--extract=%1").arg(destDir), erofsFile });
    process.start();
    if (!process.waitForStarted()) {
        return QStringLiteral("failed to start fsck.erofs");
    }
    if (!process.waitForFinished(kFsckTimeoutMs)) {
        process.kill();
        process.waitForFinished(5000);
        return QStringLiteral("fsck.erofs extract timed out");
    }
    if (process.exitCode() != 0 || process.exitStatus() != QProcess::NormalExit) {
        // stderr 可能为空，必须保证失败时返回非空串，否则上层会误判为成功
        QString strError = QString::fromLocal8Bit(process.readAllStandardError().simplified());
        if (strError.isEmpty()) {
            strError = QStringLiteral("fsck.erofs extract failed (exit code %1)").arg(process.exitCode());
        }
        return strError;
    }
    return QString();
}

QString LayerPlugin::extractPackage(const QString &destDir)
{
    qint64 erofsOffset = 0;
    qint64 erofsSize = 0;
    QString strError;
    if (!parseHeader(&erofsOffset, &erofsSize, &strError)) {
        return strError;
    }

    // fsck.erofs 老版本不支持 offset 参数，与官方兜底策略一致先落盘为独立镜像
    QFile srcFile(m_strArchiveName);
    if (!srcFile.open(QIODevice::ReadOnly)) {
        return QStringLiteral("failed to open %1").arg(m_strArchiveName);
    }
    const QString erofsFile = QFileInfo(destDir).absolutePath() + QStringLiteral("/bundle.erofs");
    if (!copyPayload(srcFile, erofsOffset, erofsSize, erofsFile, &strError)) {
        return strError;
    }
    srcFile.close();

    return runFsckExtract(erofsFile, destDir);
}

void LayerPlugin::collectEntries(const QString &dir)
{
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();

    // 先收集再排序，保证同目录下文件顺序稳定
    QFileInfoList list;
    QDirIterator it(dir, QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        list << it.fileInfo();
        it.next();
    }

    const QDir layerDir(dir);
    for (const QFileInfo &fi : list) {
        const QString rel = layerDir.relativeFilePath(fi.absoluteFilePath());
        if (rel.isEmpty() || rel == QLatin1String(".")) {
            continue;
        }

        FileEntry stEntry;
        // 符号链接按文件处理，避免跟随链接导致目录误判
        stEntry.isDirectory = fi.isDir() && !fi.isSymLink();
        stEntry.strFullPath = stEntry.isDirectory ? rel + QLatin1Char('/') : rel;
        stEntry.strFullPath.replace(QDir::separator(), QLatin1Char('/'));   // 统一为 '/' 分隔
        // 与其他插件一致：split 去空后取最后一段，避免目录尾部 '/' 导致名字为空
        const QStringList pieces = stEntry.strFullPath.split(QLatin1Char('/'), Qt::SkipEmptyParts);
        stEntry.strFileName = pieces.isEmpty() ? QString() : pieces.last();
        stEntry.qSize = stEntry.isDirectory ? 0 : fi.size();
        stEntry.uLastModifiedTime = static_cast<uint>(fi.lastModified().toSecsSinceEpoch());

        handleEntry(stEntry);   // 填充第一层列表并补齐缺失的父目录
        stArchiveData.mapFileEntry.insert(stEntry.strFullPath, stEntry);
        stArchiveData.qSize += stEntry.qSize;
    }
}

PluginFinishType LayerPlugin::list()
{
    qInfo() << "LayerPlugin加载压缩包数据";
    m_eErrorType = ET_NoError;
    ArchiveData &stArchiveData = DataManager::get_instance().archiveData();
    stArchiveData.reset();
    m_setHasHandlesDirs.clear();
    m_setHasRootDirs.clear();

    QFileInfo layerInfo(m_strArchiveName);
    if (!layerInfo.exists() || !layerInfo.isReadable()) {
        m_eErrorType = ET_FileOpenError;
        return PFT_Error;
    }
    stArchiveData.qComressSize = layerInfo.size();

    QTemporaryDir tmpRoot(QStandardPaths::writableLocation(QStandardPaths::TempLocation)
                          + QStringLiteral("/deepin-compressor-linglong-XXXXXX"));
    if (!tmpRoot.isValid()) {
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }
    const QString stageDir = tmpRoot.path() + QStringLiteral("/layer");
    if (!QDir().mkpath(stageDir)) {
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }

    const QString strError = extractPackage(stageDir);
    if (!strError.isEmpty()) {
        qWarning() << "LayerPlugin extract failed:" << strError;
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    collectEntries(stageDir);
    return PFT_Nomral;
}

PluginFinishType LayerPlugin::testArchive()
{
    // 包头解析成功即认为格式合法（EROFS 完整性由解压过程保证）
    qint64 erofsOffset = 0;
    qint64 erofsSize = 0;
    QString strError;
    if (!parseHeader(&erofsOffset, &erofsSize, &strError)) {
        qWarning() << "LayerPlugin testArchive failed:" << strError;
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }
    return PFT_Nomral;
}

bool LayerPlugin::moveToDest(const QString &src, const QString &dst)
{
    QFileInfo dstInfo(dst);
    if (dstInfo.exists()) {
        if (m_bSkipAll) {
            return true;
        }
        if (!m_bOverwriteAll) {
            OverwriteQuery query(dst);
            emit signalQuery(&query);
            query.waitForResponse();
            if (query.responseCancelled()) {
                emit signalCancel();
                m_eErrorType = ET_UserCancelOpertion;
                return false;
            } else if (query.responseSkipAll()) {
                m_bSkipAll = true;
                return true;
            } else if (query.responseSkip()) {
                return true;
            } else if (query.responseOverwriteAll()) {
                m_bOverwriteAll = true;
            }
        }
        // 覆盖：先删除已有项
        if (dstInfo.isDir() && !dstInfo.isSymLink()) {
            QDir(dst).removeRecursively();
        } else {
            QFile::remove(dst);
        }
    }

    QFileInfo srcInfo(src);
    if (srcInfo.isDir() && !srcInfo.isSymLink()) {
        if (!QDir().mkpath(dst)) {
            m_eErrorType = ET_FileWriteError;
            emit signalFileWriteErrorName(dst);
            return false;
        }
        QDir srcDir(src);
        const QFileInfoList children =
                srcDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden);
        for (const QFileInfo &child : children) {
            if (!moveToDest(child.absoluteFilePath(), dst + QLatin1Char('/') + child.fileName())) {
                return false;
            }
        }
        return true;
    }

    // 文件（含符号链接）：优先 rename（符号链接原样保留）；rename 失败时，
    // 符号链接用 QFile::link 重建（QFile::copy 会跟随链接复制内容，语义不对）
    QFile srcFile(src);
    if (srcFile.rename(dst)) {
        return true;
    }
    srcFile.unsetError();
    if (srcInfo.isSymLink()) {
        // readlink 取原始链接文本重建：symLinkTarget() 会解析为绝对路径，
        // 相对链接将指向已删除的 staging 目录；读取失败再退回解析后的目标
        const QByteArray encoded = QFile::encodeName(src);
        char buf[4096];
        const ssize_t n = ::readlink(encoded.constData(), buf, sizeof(buf) - 1);
        const QString linkTarget = (n > 0) ? QString::fromLocal8Bit(buf, static_cast<int>(n))
                                           : srcInfo.symLinkTarget();
        if (QFile::link(linkTarget, dst)) {
            return true;
        }
    } else if (srcFile.copy(dst)) {
        return true;
    }
    m_eErrorType = ET_FileWriteError;
    emit signalFileWriteErrorName(dst);
    return false;
}

bool LayerPlugin::moveStagedToTarget(const QString &stageDir, const QString &targetPath, bool extractAll,
                                     const QList<FileEntry> &files, const QString &strDestination)
{
    // 需要搬入 target 的源目录与顶层子项列表：
    // 1) 全部解压：搬 stageDir 下所有顶层项
    // 2) 提取包根目录下的项（strDestination 为空）：仅搬选中的项，保留包内名称
    // 3) 提取深层项（strDestination 非空）：搬该目录下所有子项，不保留前缀
    QString srcRoot = stageDir;
    bool keepName = true;

    const QStringList stageTopItems = QDir(stageDir)
                                              .entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System
                                                         | QDir::Hidden);

    if (!extractAll && !strDestination.isEmpty() && strDestination != QLatin1String("/")) {
        // 深层提取：进入选中的上级目录，去掉包内前缀
        QString rootNode = strDestination;
        while (rootNode.startsWith(QLatin1Char('/'))) {
            rootNode.remove(0, 1);
        }
        while (rootNode.endsWith(QLatin1Char('/'))) {
            rootNode.chop(1);
        }
        srcRoot = stageDir + QLatin1Char('/') + rootNode;
        keepName = false;
        if (!QFileInfo(srcRoot).isDir()) {
            qWarning() << "LayerPlugin selected root node missing in staged dir:" << srcRoot;
            m_eErrorType = ET_FileReadError;
            return false;
        }
    }

    QStringList itemsToMove;
    if (extractAll) {
        itemsToMove = stageTopItems;
    } else if (keepName) {
        for (const FileEntry &entry : files) {
            QString name = entry.strFullPath;
            while (name.startsWith(QLatin1Char('/'))) {
                name.remove(0, 1);
            }
            while (name.endsWith(QLatin1Char('/'))) {
                name.chop(1);
            }
            if (!name.isEmpty() && !itemsToMove.contains(name)) {
                itemsToMove << name;
            }
        }
    } else {
        itemsToMove = QDir(srcRoot)
                              .entryList(QDir::AllEntries | QDir::NoDotAndDotDot | QDir::System | QDir::Hidden);
    }

    // 统计总大小用于进度显示
    m_qTotalSize = 0;
    m_qDoneSize = 0;
    QList<QPair<QString, QString>> moveList;   // <源绝对路径, 目标绝对路径>
    for (const QString &name : itemsToMove) {
        const QString src = srcRoot + QLatin1Char('/') + name;
        const QString dst = targetPath + QLatin1Char('/') + name;
        m_qTotalSize += QFileInfo(src).isDir() ? 0 : QFileInfo(src).size();
        moveList << qMakePair(src, dst);
    }
    // 总字节数为 0（纯目录）时按条目计数推进进度
    const bool countByItems = (m_qTotalSize <= 0);
    if (countByItems) {
        m_qTotalSize = moveList.size();
    }

    for (const auto &pair : moveList) {
        emit signalCurFileName(QFileInfo(pair.first).fileName());
        // rename 成功后源路径已不存在，须在 move 前记录条目大小
        const qint64 entrySize = QFileInfo(pair.first).isDir() ? 0 : QFileInfo(pair.first).size();
        if (!moveToDest(pair.first, pair.second)) {
            return false;
        }
        if (countByItems) {
            m_qDoneSize += 1;
        } else {
            m_qDoneSize += entrySize;
        }
        emit signalprogress(100.0 * m_qDoneSize / m_qTotalSize);
    }
    return true;
}

PluginFinishType LayerPlugin::extractFiles(const QList<FileEntry> &files, const ExtractionOptions &options)
{
    qInfo() << "LayerPlugin解压";
    m_eErrorType = ET_NoError;
    m_bOverwriteAll = false;
    m_bSkipAll = false;

    const bool extractAll = files.isEmpty();
    QString targetPath = options.strTargetPath;
    if (targetPath.isEmpty()) {
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }

    // 目标目录存在性检查与创建（对齐 libarchive 插件行为）
    QFileInfo targetInfo(targetPath);
    if (targetInfo.exists() && !targetInfo.isDir()) {
        m_eErrorType = ET_FileWriteError;
        qWarning() << "LayerPlugin target is not a directory:" << targetPath;
        return PFT_Error;
    }
    if (!targetInfo.exists() && !QDir().mkpath(targetPath)) {
        if (isInsufficientDiskSpace(targetPath)) {
            m_eErrorType = ET_InsufficientDiskSpace;
        } else {
            m_eErrorType = ET_FileWriteError;
        }
        return PFT_Error;
    }

    // 暂存目录放在目标目录内（保证同文件系统，rename 高效）
    QTemporaryDir staging(targetPath + QStringLiteral("/.deepin-compressor-linglong-XXXXXX"));
    if (!staging.isValid()) {
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }
    const QString stageDir = staging.path() + QStringLiteral("/layer");
    if (!QDir().mkpath(stageDir)) {
        m_eErrorType = ET_FileWriteError;
        return PFT_Error;
    }

    const QString strError = extractPackage(stageDir);
    if (!strError.isEmpty()) {
        qWarning() << "LayerPlugin extract failed:" << strError;
        m_eErrorType = ET_ArchiveDamaged;
        return PFT_Error;
    }

    if (!moveStagedToTarget(stageDir, targetPath, extractAll, files, options.strDestination)) {
        return (m_eErrorType == ET_UserCancelOpertion) ? PFT_Cancel : PFT_Error;
    }

    return PFT_Nomral;
}
