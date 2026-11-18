// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLI7Z_BACKEND_H
#define CLI7Z_BACKEND_H

#include "cli_runner.h"

// 7z 命令行后端：参数模板 + 输出解析
class Cli7zBackend : public CliRunner
{
    Q_OBJECT

public:
    explicit Cli7zBackend(QObject *parent, const QVariantList &args);
    ~Cli7zBackend() override;

    bool isPasswordPrompt(const QString &line) override;
    bool isWrongPasswordMsg(const QString &line) override;
    bool isCorruptArchiveMsg(const QString &line) override;
    bool isDiskFullMsg(const QString &line) override;
    bool isFileExistsMsg(const QString &line) override;
    bool isFileExistsFileName(const QString &line) override;
    bool isMultiPasswordPrompt(const QString &line) override;
    bool isOpenFileFailed(const QString &line) override;

    void killProcess(bool emitFinished = true) override;

protected:
    bool handleLine(const QString &line, WorkType workStatus) override;

protected:
    bool supportsLongNameRename() const override;
    QString programFor(WorkType work) const override;
    QStringList argumentsFor(WorkType work, const QString &archive,
                             const QStringList &files, const CompressOptions *options) const override;

private:
    enum ParseState {
        ParseStateTitle = 0,
        ParseStateHeader,
        ParseStateArchiveInformation,
        ParseStateEntryInformation
    };

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
    };

    bool readListLine(const QString &line);
    bool isNoFilesArchive(const QString &line);

    ArchiveType m_archiveType = ArchiveType7z;
    ParseState m_parseState = ParseStateTitle;
    FileEntry m_fileEntry;
};

#endif // CLI7Z_BACKEND_H
