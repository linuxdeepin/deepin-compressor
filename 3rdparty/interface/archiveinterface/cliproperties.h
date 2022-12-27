/*
 * ark -- archiver for the KDE project
 *
 * Copyright (C) 2016 Ragnar Thomsen <rthomsen6@gmail.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES ( INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION ) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * ( INCLUDING NEGLIGENCE OR OTHERWISE ) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef CLIPROPERTIES_H
#define CLIPROPERTIES_H

#include "archiveinterface.h"
#include <QVariant>

typedef QHash<QString, QVariant> Hash;

class CliProperties : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString addProgram MEMBER m_addProgram)
    Q_PROPERTY(QString deleteProgram MEMBER m_deleteProgram)
    Q_PROPERTY(QString extractProgram MEMBER m_extractProgram)
    Q_PROPERTY(QString listProgram MEMBER m_listProgram)
    Q_PROPERTY(QString moveProgram MEMBER m_moveProgram)
    Q_PROPERTY(QString testProgram MEMBER m_testProgram)

    Q_PROPERTY(QString progressarg MEMBER m_progressarg)
    Q_PROPERTY(QStringList addSwitch MEMBER m_addSwitch)
    Q_PROPERTY(QStringList commentSwitch MEMBER m_commentSwitch)
    Q_PROPERTY(QString deleteSwitch MEMBER m_deleteSwitch)
    Q_PROPERTY(QStringList extractSwitch MEMBER m_extractSwitch)
    Q_PROPERTY(QStringList extractSwitchNoPreserve MEMBER m_extractSwitchNoPreserve)
    Q_PROPERTY(QStringList listSwitch MEMBER m_listSwitch)
    Q_PROPERTY(QString moveSwitch MEMBER m_moveSwitch)
    Q_PROPERTY(QStringList testSwitch MEMBER m_testSwitch)

    Q_PROPERTY(QStringList passwordSwitch MEMBER m_passwordSwitch)
    Q_PROPERTY(QStringList passwordSwitchHeaderEnc MEMBER m_passwordSwitchHeaderEnc)
    Q_PROPERTY(QString compressionLevelSwitch MEMBER m_compressionLevelSwitch)
    Q_PROPERTY(Hash compressionMethodSwitch MEMBER m_compressionMethodSwitch)
    Q_PROPERTY(Hash encryptionMethodSwitch MEMBER m_encryptionMethodSwitch)
    Q_PROPERTY(QString multiVolumeSwitch MEMBER m_multiVolumeSwitch)

    Q_PROPERTY(QStringList testPassedPatterns MEMBER m_testPassedPatterns)
    Q_PROPERTY(QStringList fileExistsFileNameRegExp MEMBER m_fileExistsFileNameRegExp)

    Q_PROPERTY(QStringList fileExistsInput MEMBER m_fileExistsInput)
    Q_PROPERTY(QStringList multiVolumeSuffix MEMBER m_multiVolumeSuffix)

    Q_PROPERTY(bool captureProgress MEMBER m_captureProgress)

public:
    explicit CliProperties(QObject *parent, const KPluginMetaData &metaData, const CustomMimeType &archiveType);

    QStringList addArgs(const QString &archive,
                        const QStringList &files,
                        const QString &password,
                        bool headerEncryption,
                        int compressionLevel,
                        const QString &compressionMethod,
                        const QString &encryptionMethod,
                        int volumeSize,
                        bool isTar7z,
                        const QString &globalWorkDir);
    QStringList commentArgs(const QString &archive, const QString &commentfile);
    QStringList deleteArgs(const QString &archive, const QList<FileEntry> &files, const QString &password);
    QStringList extractArgs(const QString &archive, const QStringList &files, bool preservePaths, const QString &password);
    QStringList listArgs(const QString &archive, const QString &password);
    QStringList moveArgs(const QString &archive, const QList<FileEntry> &entries, QString &destination, const QString &password);
    QStringList testArgs(const QString &archive, const QString &password);

    bool isTestPassedMsg(const QString &line);

private:
    QStringList substituteCommentSwitch(const QString &commentfile) const;
    QStringList substitutePasswordSwitch(const QString &password, bool headerEnc = false) const;
    QString substituteCompressionLevelSwitch(int level) const;
    QString substituteCompressionMethodSwitch(const QString &method) const;
    QString substituteEncryptionMethodSwitch(const QString &method) const;
    QString substituteMultiVolumeSwitch(int volumeSize) const;

    QString m_addProgram;
    QString m_deleteProgram;
    QString m_extractProgram;
    QString m_listProgram;
    QString m_moveProgram;
    QString m_testProgram;

    QStringList m_addSwitch;
    QStringList m_commentSwitch;
    QString m_deleteSwitch;
    QStringList m_extractSwitch;
    QStringList m_extractSwitchNoPreserve;
    QStringList m_listSwitch;
    QString m_moveSwitch;
    QStringList m_testSwitch;

    QStringList m_passwordSwitch;
    QStringList m_passwordSwitchHeaderEnc;
    QString m_compressionLevelSwitch;
    QHash<QString, QVariant> m_compressionMethodSwitch;
    QHash<QString, QVariant> m_encryptionMethodSwitch;
    QString m_multiVolumeSwitch;

    QStringList m_testPassedPatterns;
    QStringList m_fileExistsFileNameRegExp;

    QStringList m_fileExistsInput;
    QStringList m_multiVolumeSuffix;
    QString m_progressarg;

    bool m_captureProgress = false;

    CustomMimeType m_mimeType;
    KPluginMetaData m_metaData;
};

#endif // CLIPROPERTIES_H
