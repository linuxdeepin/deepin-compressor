// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CLIRAR_BACKEND_H
#define CLIRAR_BACKEND_H

#include "cli_runner.h"

// unrar 命令行后端：参数模板 + 输出解析
class CliRarBackend : public CliRunner
{
    Q_OBJECT

public:
    explicit CliRarBackend(QObject *parent, const QVariantList &args);
    ~CliRarBackend() override;

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

    bool readListLine(const QString &line);

    ParseState m_parseState = ParseStateTitle;
    FileEntry m_fileEntry;
    QString m_comment;
};

#endif // CLIRAR_BACKEND_H
