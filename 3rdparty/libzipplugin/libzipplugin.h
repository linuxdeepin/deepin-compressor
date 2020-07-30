#ifndef LIBZIPPLUGIN_H
#define LIBZIPPLUGIN_H

#include "archiveinterface.h"
#include "kpluginfactory.h"

#include <QFileDevice>

#include <zip.h>
#include <minizip/unzip.h>


struct FileProgressInfo {
    float fileProgressProportion = 0.0;
    float fileProgressStart;
    QString fileName;
};

/**
 * @brief The enum_extractEntryStatus enum
 * @see 解压单个entry的三种可能结果
 */
enum enum_extractEntryStatus {
    FAIL,//解压失败
    SUCCESS,//解压成功
    PSD_NEED//需要密码
};

enum enum_checkEntryPsd {
    NOTCHECK,//未检测
    PSDWRONG,//密码错误
    RIGHT,//打开正确
    PSDNEED//需要输入密码
};

class LibzipPluginFactory : public KPluginFactory
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID "org.kde.KPluginFactory" FILE "kerfuffle_libzip.json")
    Q_INTERFACES(KPluginFactory)
public:
    explicit LibzipPluginFactory();
    ~LibzipPluginFactory();
};

class LibzipPlugin : public ReadWriteArchiveInterface
{
    Q_OBJECT

public:
    explicit LibzipPlugin(QObject *parent, const QVariantList &args);
    ~LibzipPlugin() override;

    bool list(bool isbatch = false) override;
    bool doKill() override;
    bool extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options) override;
    bool addFiles(const QVector<Archive::Entry *> &files, const Archive::Entry *destination, const CompressionOptions &options, uint numberOfEntriesToAdd = 0) override;
    bool deleteFiles(const QVector<Archive::Entry *> &files) override;
    bool moveFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options) override;
    bool copyFiles(const QVector<Archive::Entry *> &files, Archive::Entry *destination, const CompressionOptions &options) override;
    bool addComment(const QString &comment) override;
    bool testArchive() override;
    void cleanIfCanceled()override;
    void watchFileList(QStringList *strList)override;

    /**
     * @brief checkArchivePsd:首次解压Archive需要判断一次密码
     * @param archive:归档对象
     * @return 如果返回false，结束当前解压
     */
    bool checkArchivePsd(zip_t *archive, int &iCodecIndex);

    /**
     * @brief checkEntriesPsd:首次提取Entry树需要判断一次密码
     * @param archive
     * @param selectedEnV:选中的Entry树
     * @return 如果返回false，结束当前解压
     */
    //bool checkEntriesPsd(zip_t *archive, const QVector<Archive::Entry *> &selectedEnV);
    bool checkEntriesPsd(zip_t *archive, QList<int> listExtractIndex);

    /**
     * @brief checkEntryPsd
     * @param archive:归档对象
     * @param pCur:检测密码的Entry节点
     * @param stop:是否停止当前job
     */
    //void checkEntryPsd(zip_t *archive, Archive::Entry *pCur, enum_checkEntryPsd &status);
    void checkEntryPsd(zip_t *archive, int iIndex, enum_checkEntryPsd &status);

    int ChartDet_DetectingTextCoding(const char *str, QString &encoding, float &confidence);

    virtual void showEntryListFirstLevel(const QString &directory) override;
    virtual void RefreshEntryFileCount(Archive::Entry *file) override;

    virtual qint64 extractSize(const QVector<Archive::Entry *> &files) override;
    virtual void updateListMap() override;

private Q_SLOTS:
    void slotRestoreWorkingDir();

private:
//    bool deleteEntry(Archive::Entry *pEntry, zip_t *archive/*, int &curNo, int count = -1*/);
    bool deleteEntry(QString file/*Archive::Entry *pCurEntry*/, zip_t *archive/*, int &curNo, int count = -1*/);
    enum_extractEntryStatus extractEntry(zip_t *archive, int index, const QString &entry, const QString &rootNode, const QString &destDir, bool preservePaths, bool removeRootNode, FileProgressInfo &pi);
    bool writeEntry(zip_t *archive, const QString &entry, const Archive::Entry *destination, const CompressionOptions &options, bool isDir = false, const QString &strRoot = "");
    bool emitEntryForIndex(zip_t *archive, qlonglong index);
    void emitProgress(double percentage);
    int cancelResult();
    QString permissionsToString(const mode_t &perm);
    static void progressCallback(zip_t *, double progress, void *that);
    static int cancelCallback(zip_t *, void *that);
    QByteArray detectEncode(const QByteArray &data, const QString &fileName = QString());
    QByteArray textCodecDetect(const QByteArray &data, const QString &fileName);
    void detectAllfile(zip_t *archive, int num);
    QString  trans2uft8(const char *str);

    const char *passwordUnicode(const QString &strPassword, int iIndex);

    /*user minizip*/
    bool minizip_list(bool isbatch = false);
    bool minizip_emitEntryForIndex(unzFile zipfile);
    bool minizip_extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options);
    bool minizip_extractEntry(unzFile zipfile, unz_file_info file_info, const QString &entry, const QString &rootNode, const QString &destDir, bool preservePaths, bool removeRootNode, FileProgressInfo &pi);

    Archive::Entry *setEntryData(const zip_stat_t &statBuffer, qlonglong index, const QString &name, bool isMutilFolderFile = false);
    Archive::Entry *setEntryDataA(const zip_stat_t &statBuffer, qlonglong index, const QString &name);
    void setEntryVal(const zip_stat_t &statBuffer, int &index, const QString &name, QString &dirRecord);

    void setEntryVal1(const zip_stat_t &statBuffer, int &index, const QString &name, QString &dirRecord);

private:
    QVector<Archive::Entry *> m_emittedEntries;
    bool m_overwriteAll;
    bool m_skipAll;
    bool m_listAfterAdd;
    int m_filesize;
    zip_t *m_addarchive;
    QByteArray m_codecstr;
    QByteArray m_codecname;
    ExtractionOptions m_extractionOptions;
    bool isWrongPassword = false;
    QString m_extractDestDir;
    QString m_extractFile;

    QStringList m_listCodecs;
    QMap<QString, QPair<zip_stat_t, qlonglong>> m_listMap;
    QString m_DirRecord;
    QString m_SigDirRecord;
    int m_indexCount = 0;

    QList<int> m_listExtractIndex;
    QString m_strRootNode;

    bool m_bCancel = false;
};

#endif // LIBZIPPLUGIN_H
