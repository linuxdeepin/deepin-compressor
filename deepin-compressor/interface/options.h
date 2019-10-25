#ifndef OPTIONS_H
#define OPTIONS_H


#include <QDebug>



class  Options
{
public:

    bool encryptedArchiveHint() const;
    void setEncryptedArchiveHint(bool encrypted);

private:

    bool m_encryptedArchiveHint = false;
};

class  CompressionOptions : public Options
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

private:
    int m_compressionLevel = -1;
    ulong m_volumeSize = 0;
    QString m_compressionMethod;
    QString m_encryptionMethod;
    QString m_globalWorkDir;
};

class  ExtractionOptions : public Options
{
public:

    bool preservePaths() const;
    void setPreservePaths(bool preservePaths);
    bool isDragAndDropEnabled() const;
    void setDragAndDropEnabled(bool enabled);
    bool alwaysUseTempDir() const;
    void setAlwaysUseTempDir(bool alwaysUseTempDir);

private:

    bool m_preservePaths = true;
    bool m_dragAndDrop = false;
    bool m_alwaysUseTempDir = false;
};

QDebug  operator<<(QDebug d, const CompressionOptions &options);
QDebug  operator<<(QDebug d, const ExtractionOptions &options);


Q_DECLARE_METATYPE(CompressionOptions)
Q_DECLARE_METATYPE(ExtractionOptions)

#endif
