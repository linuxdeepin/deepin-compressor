// Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mimetypes.h"

#include <QFileInfo>
#include <QMimeDatabase>
#include <QRegularExpression>
#include <QDebug>
#include <QFile>
#include <QProcess>
#include <QtEndian>

namespace {
/**
 * @brief 检测 ELF64 小端文件是否包含指定 section 名
 *
 * 玲珑 uab 包本质是 ELF 容器，内容识别命中 x-executable 及其子类型（如新版
 * mime db 中的 x-pie-executable），与普通 ELF 可执行文件仅能靠 section 特征
 * 区分，mime magic 无法安全表达（ELF magic 会误伤所有可执行文件），因此在此
 * 精确检测。
 */
bool elfContainsSection(const QString &path, const QByteArray &sectionName)
{
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    const QByteArray ehdr = file.read(64);
    if (ehdr.size() != 64) {
        return false;
    }
    const uchar *e = reinterpret_cast<const uchar *>(ehdr.constData());
    if (memcmp(e, "\x7f""ELF", 4) != 0 || e[4] != 2 || e[5] != 1) {   // \x7fELF, 64-bit LE
        return false;
    }
    const quint64 shoff = qFromLittleEndian<quint64>(e + 40);          // e_shoff
    const quint16 shentsize = qFromLittleEndian<quint16>(e + 58);      // e_shentsize
    const quint16 shnum = qFromLittleEndian<quint16>(e + 60);          // e_shnum
    const quint16 shstrndx = qFromLittleEndian<quint16>(e + 62);       // e_shstrndx
    if (shoff == 0 || shentsize != 64 || shnum == 0 || shstrndx >= shnum) {
        return false;
    }
    if (shoff > static_cast<quint64>(file.size())
        || static_cast<qint64>(shnum) * shentsize > file.size() - static_cast<qint64>(shoff)) {
        return false;
    }
    file.seek(static_cast<qint64>(shoff));
    const QByteArray shdrs = file.read(static_cast<qint64>(shnum) * shentsize);
    if (shdrs.size() != static_cast<qint64>(shnum) * shentsize) {
        return false;
    }
    const uchar *strSh = reinterpret_cast<const uchar *>(shdrs.constData()) + qint64(shstrndx) * 64;
    const quint64 strOff = qFromLittleEndian<quint64>(strSh + 24);     // sh_offset
    const quint64 strSize = qFromLittleEndian<quint64>(strSh + 32);    // sh_size
    if (strSize == 0 || strSize > 16 * 1024 * 1024
        || strSize > static_cast<quint64>(file.size())
        || strOff > static_cast<quint64>(file.size()) - strSize) {
        return false;
    }
    file.seek(static_cast<qint64>(strOff));
    const QByteArray shstrtab = file.read(static_cast<qint64>(strSize));
    for (int i = 0; i < shnum; ++i) {
        const uchar *sh = reinterpret_cast<const uchar *>(shdrs.constData()) + qint64(i) * 64;
        const quint32 nameOff = qFromLittleEndian<quint32>(sh);        // sh_name
        if (nameOff >= static_cast<quint32>(shstrtab.size())) {
            continue;
        }
        if (shstrtab.mid(int(nameOff), sectionName.size() + 1) == sectionName + '\0') {
            return true;
        }
    }
    return false;
}
}   // namespace

CustomMimeType determineMimeType(const QString &filename)
{
    qDebug() << "Determining MIME type for:" << filename;
    QMimeDatabase db;
    CustomMimeType stMimeType;

    QFileInfo fileinfo(filename);
    QString inputFile = filename;

    // #328815: since detection-by-content does not work for compressed tar archives (see below why)
    // we cannot rely on it when the archive extension is wrong; we need to validate by hand.
    if (fileinfo.completeSuffix().toLower().remove(QRegularExpression(QStringLiteral("[^a-z\\.]"))).contains(QStringLiteral("tar."))) {
        inputFile.chop(fileinfo.completeSuffix().length());
        QString cleanExtension(fileinfo.completeSuffix().toLower());

        // tar.bz2 and tar.lz4 need special treatment since they contain numbers.
        bool isBZ2 = false;
        bool isLZ4 = false;
        bool is7Z = false;
        if (fileinfo.completeSuffix().toLower().endsWith(QStringLiteral("tar.bz2"))) {
            cleanExtension.remove(QStringLiteral("bz2"));
            isBZ2 = true;
        }

        if (fileinfo.completeSuffix().toLower().endsWith(QStringLiteral("tar.lz4"))) {
            cleanExtension.remove(QStringLiteral("lz4"));
            isLZ4 = true;
        }

        if (fileinfo.completeSuffix().toLower().endsWith(QStringLiteral("tar.7z"))) {
            cleanExtension.remove(QStringLiteral("7z"));
            is7Z = true;
        }

        // We remove non-alpha chars from the filename extension, but not periods.
        // If the filename is e.g. "foo.tar.gz.1", we get the "foo.tar.gz." string,
        // so we need to manually drop the last period character from it.
        cleanExtension.remove(QRegularExpression(QStringLiteral("[^a-z\\.]")));
        if (cleanExtension.endsWith(QLatin1Char('.'))) {
            cleanExtension.chop(1);
        }

        // Re-add extension for tar.bz2 and tar.lz4.
        if (isBZ2) {
            cleanExtension.append(QStringLiteral(".bz2"));
        }

        if (isLZ4) {
            cleanExtension.append(QStringLiteral(".lz4"));
        }

        if (is7Z) {
            cleanExtension.append(QStringLiteral(".7z"));
        }

        inputFile += cleanExtension;
    }

//    QMimeType mimeFromDefault = db.mimeTypeForFile(inputFile, QMimeDatabase::MatchDefault);
    QMimeType mimeFromExtension = db.mimeTypeForFile(inputFile, QMimeDatabase::MatchExtension);
    QMimeType mimeFromContent = db.mimeTypeForFile(filename, QMimeDatabase::MatchContent);


//    qInfo() << "mimeFromDefault******************" << mimeFromDefault.name() << mimeFromDefault.parentMimeTypes();
//    qInfo() << "mimeFromExtension******************" << mimeFromExtension.name() << mimeFromExtension.parentMimeTypes();
//    qInfo() << "mimeFromContent****************" << mimeFromContent.name() << mimeFromContent.parentMimeTypes();

    // mimeFromContent will be "application/octet-stream" when file is
    // unreadable, so use extension.
    if (!fileinfo.isReadable()) {
        qWarning() << "File is not readable, using extension-based MIME type:" << filename;
        stMimeType.m_mimeType = mimeFromExtension;
        return stMimeType;
    }

    // Compressed tar-archives are detected as single compressed files when
    // detecting by content. The following code fixes detection of tar.gz, tar.bz2, tar.xz,
    // tar.lzo, tar.lz, tar.lrz and tar.zst.
    if ((mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-compressed-tar"))
            && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/gzip")))
            || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-bzip-compressed-tar"))
                && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-bzip")))
            || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-xz-compressed-tar"))
                && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-xz")))
            || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-tarz"))
                && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-compress")))
            || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-tzo"))
                && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-lzop")))
            || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-lzip-compressed-tar"))
                && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-lzip")))
            || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-lrzip-compressed-tar"))
                && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-lrzip")))
            || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-lz4-compressed-tar"))
                && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/x-lz4")))
            || (mimeFromExtension == db.mimeTypeForName(QStringLiteral("application/x-zstd-compressed-tar"))
                && mimeFromContent == db.mimeTypeForName(QStringLiteral("application/zstd")))) {
        stMimeType.m_mimeType = mimeFromExtension;
        return stMimeType;
    }

    /* 内容为默认格式，即"application/octet-stream"，使用"file --mime-type"再次进行检测，主要针对zip格式再次进行判断
    *  zip空压缩包：内容检测为"application/octet-stream"，后缀检测为"application/zip"，file命令探测为"application/zip"
    *  谷歌插件zip：内容检测为"application/octet-stream"，后缀检测为"application/zip"，file命令探测为"application/x-chrome-extension"
    *  谷歌插件crx：内容检测为"application/octet-stream"，后缀检测为"application/octet-stream"，file命令探测为"application/x-chrome-extension"
    *  zip分卷包：内容检测为"application/octet-stream"，后缀检测为"application/zip"，file命令探测为"application/octet-stream"
    *  iso：内容检测为"application/octet-stream"，后缀检测为"application/x-cd-image"，file命令探测为"application/x-iso9660-image"
    *  WinZip分卷ZIP：扩展名检测为"application/zip"，内容检测可能不是zip（如误识别为pdf），file命令探测为"application/zip"
    */
    const QString &extName = mimeFromExtension.name();
    bool isZipExtension = (extName == QStringLiteral("application/zip"));
    bool isContentZip = mimeFromContent.inherits(QStringLiteral("application/zip"));
    bool needFileCommandFallback = mimeFromContent.isDefault() || (isZipExtension && !isContentZip);
    if (needFileCommandFallback) {
        qDebug() << "Using file command to detect MIME type for:" << filename;
        QProcess process;
        QStringList args;
        args << "--mime-type" << filename;
        process.setProgram("file");
        process.setArguments(args);
        process.start();
        process.waitForFinished();
        const QString output = QString::fromUtf8(process.readAllStandardOutput());

        stMimeType.m_bUnKnown = true;
        if (output.contains("application/octet-stream")) {
            qDebug() << "File command detected octet-stream, using extension type";
            stMimeType.m_strTypeName = mimeFromExtension.name();
            return stMimeType;
        } else if (output.contains("application/x-chrome-extension")) {
            qDebug() << "Detected Chrome extension";
            stMimeType.m_strTypeName = "application/x-chrome-extension";
            return stMimeType;
        } else if (output.contains("application/zip")) {
            qDebug() << "Detected ZIP archive";
            stMimeType.m_strTypeName = "application/zip";
            return stMimeType;
        } else if (output.contains("application/x-iso9660-image")) {
            qDebug() << "Detected ISO image";
            stMimeType.m_strTypeName = "application/x-iso9660-image";
            return stMimeType;
        } else {        // 对于其余情况，使用已识别出的后缀识别即可
            qDebug() << "Using extension-based MIME type as fallback";
            stMimeType.m_bUnKnown = false;
            stMimeType.m_mimeType = mimeFromExtension;
            return stMimeType;
        }
    }

    stMimeType.m_bUnKnown = false;
    // 对于内容和后缀不一致的情况进行的处理
    if (mimeFromExtension != mimeFromContent) {
        // 玲珑 uab 包：系统 mimetype 归属为官方 linglong-bin 注册的
        // application/vnd.linyaps.uab（ELF 容器，sub-class-of x-executable），此处
        // 用 ELF section 特征精确确认；inherits 同时匹配自身与子类型
        // （x-pie-executable 等），检测失败则保持内容识别结果（非 uab 的 ELF）
        if (extName == QStringLiteral("application/vnd.linyaps.uab")
                && mimeFromContent.inherits(QStringLiteral("application/x-executable"))
                && elfContainsSection(filename, QByteArrayLiteral("linglong.meta"))) {
            stMimeType.m_mimeType = mimeFromExtension;
            return stMimeType;
        }
        if ((mimeFromContent.inherits(QStringLiteral("text/x-qml")) && fileinfo.completeSuffix().toLower().contains(QStringLiteral("rar")))
                || (mimeFromContent.name() == QStringLiteral("image/svg+xml") && mimeFromExtension.name() == QStringLiteral("application/zip"))) {
            stMimeType.m_mimeType = mimeFromExtension;
            return stMimeType;
        }

    }

    stMimeType.m_mimeType = mimeFromContent;
    qDebug() << "Determined MIME type:" << stMimeType.m_mimeType.name();
    return stMimeType;
}
