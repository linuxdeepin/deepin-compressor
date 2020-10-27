/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#ifndef OPTIONS_H
#define OPTIONS_H

#include <QDebug>

class Options
{
public:
    bool encryptedArchiveHint() const;
    void setEncryptedArchiveHint(bool encrypted);

private:
    bool m_encryptedArchiveHint = false;
};

class CompressionOptions : public Options
{
public:
    bool isCompressionLevelSet() const;

    bool isVolumeSizeSet() const;

    int compressionLevel() const;
    void setCompressionLevel(int level);
    ulong volumeSize() const;
    void setVolumeSize(ulong size);
    QString compressionMethod() const;
    void setCompressionMethod(const QString &method);
    QString encryptionMethod() const;
    void setEncryptionMethod(const QString &method);

    QString globalWorkDir() const;

    void setGlobalWorkDir(const QString &workDir);

    bool isTar7z() const;
    void setIsTar7z(bool bTar7z);

    qint64 getfilesSize() const;
    void setFilesSize(const qint64 &filesSize);

    QString getWriteComment() const;
    void setWriteComment(const QString &writeComment);

private:
    bool m_isTar7z = false;
    int m_compressionLevel = -1;
    qint64 m_filesSize = 1; //选择需要压缩的文件大小，默认1M
    ulong m_volumeSize = 0;
    QString m_compressionMethod;
    QString m_encryptionMethod;
    QString m_globalWorkDir;
    QString m_writeComment; //注释内容
};

class Settings_Extract_Info;
class ExtractionOptions : public Options
{
public:
    bool preservePaths() const;
    void setPreservePaths(bool preservePaths);
    bool isDragAndDropEnabled() const;
    void setDragAndDropEnabled(bool enabled);
    bool alwaysUseTempDir() const;
    void setAlwaysUseTempDir(bool alwaysUseTempDir);
    bool isBatchExtract();
    void setBatchExtract(bool status);
    bool isAutoCreatDir() const;
    void setAutoCreatDir(bool bAutoCreatDir);
    bool isRightMenuExtractHere() const;
    void setRightMenuExtractHere(bool rightMenuExtractHere);

public:
    Settings_Extract_Info *pSettingInfo = nullptr;

private:
    bool m_preservePaths = true;
    bool m_dragAndDrop = false;
    bool m_alwaysUseTempDir = false;
    bool m_isBatch = false;
    bool m_isAutoCreatDir = true;
    bool m_isRightMenuExtractHere = false;

};

QDebug operator<<(QDebug d, const CompressionOptions &options);
QDebug operator<<(QDebug d, const ExtractionOptions &options);

Q_DECLARE_METATYPE(CompressionOptions)
Q_DECLARE_METATYPE(ExtractionOptions)

#endif
