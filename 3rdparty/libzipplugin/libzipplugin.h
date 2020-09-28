#ifndef LIBZIPPLUGIN_H
#define LIBZIPPLUGIN_H

#include "archiveinterface.h"
#include "kpluginfactory.h"
#include "../common/common.h"

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

    //int ChartDet_DetectingTextCoding(const char *str, QString &encoding, float &confidence);

    /**
    * show the package first level entry
    * @brief the input param is entry full path
    */
    virtual void showEntryListFirstLevel(const QString &directory) override;

    /**
     * current directory entry file count
     * @brief the input param is entry file
     */
    virtual void RefreshEntryFileCount(Archive::Entry *file) override;

    virtual qint64 extractSize(const QVector<Archive::Entry *> &files) override;
    virtual void updateListMap(QVector<Archive::Entry *> &files, int type) override;

private Q_SLOTS:
    void slotRestoreWorkingDir();

private:
//    bool deleteEntry(Archive::Entry *pEntry, zip_t *archive/*, int &curNo, int count = -1*/);
    /**
     * @brief deleteEntry   从压缩包中删除指定文件
     * @param file      文件名
     * @param index     文件索引
     * @param archive   压缩包数据
     * @return
     */
    bool deleteEntry(QString file, int index/*Archive::Entry *pCurEntry*/, zip_t *archive/*, int &curNo, int count = -1*/);

    /**
     * @brief extractEntry  解压指定文件
     * @param archive   压缩包数据
     * @param index     文件索引
     * @param entry     文件名
     * @param rootNode  父目录
     * @param destDir   解压路径
     * @param preservePaths
     * @param removeRootNode    是否移除父目录
     * @param pi    进度信息
     * @return
     */
    enum_extractEntryStatus extractEntry(zip_t *archive, int index, const QString &entry, const QString &rootNode, const QString &destDir, bool preservePaths, bool removeRootNode, FileProgressInfo &pi);

    /**
     * @brief writeEntry 添加新的Entry
     * @param archive 压缩包数据
     * @param entry 新文件
     * @param destination
     * @param options 压缩配置参数
     * @param isDir
     * @param strRoot
     * @return
     */
    bool writeEntry(zip_t *archive, const QString &entry, const Archive::Entry *destination, const CompressionOptions &options, bool isDir = false, const QString &strRoot = "");

    /**
     * @brief emitEntryForIndex     发送指定索引的数据
     * @param archive   压缩包数据
     * @param index 索引
     * @return
     */
    bool emitEntryForIndex(zip_t *archive, qlonglong index);

    /**
     * @brief emitProgress  发送进度信号
     * @param percentage    百分比
     */
    void emitProgress(double percentage);

    /**
     * @brief cancelResult  取消结果
     * @return
     */
    int cancelResult();

    /**
     * @brief permissionsToString   权限转换
     * @param perm  权限数据
     * @return
     */
    QString permissionsToString(const mode_t &perm);

    /**
     * @brief progressCallback  进度回调函数
     * @param progress  进度
     * @param that
     */
    static void progressCallback(zip_t *, double progress, void *that);

    /**
     * @brief cancelCallback    取消回调函数
     * @param that
     * @return
     */
    static int cancelCallback(zip_t *, void *that);
    //QByteArray detectEncode(const QByteArray &data, const QString &fileName = QString());
    //QByteArray textCodecDetect(const QByteArray &data, const QString &fileName);

    /**
     * @brief detectAllfile 探测压缩包数据
     * @param archive
     * @param num
     */
    void detectAllfile(zip_t *archive, int num);
//    QString  trans2uft8(const char *str);

    /**
     * @brief passwordUnicode   密码编码转换（中文密码）
     * @param strPassword   密码
     * @param iIndex    编码索引
     * @return
     */
    const char *passwordUnicode(const QString &strPassword, int iIndex);

    /*user minizip*/
    bool minizip_list(bool isbatch = false);
    bool minizip_emitEntryForIndex(unzFile zipfile);
    bool minizip_extractFiles(const QVector<Archive::Entry *> &files, const QString &destinationDirectory, const ExtractionOptions &options);
    bool minizip_extractEntry(unzFile zipfile, unz_file_info file_info, const QString &entry, const QString &rootNode, const QString &destDir, bool preservePaths, bool removeRootNode, FileProgressInfo &pi);

    /**
     * @brief setEntryData  设置Entry数据
     * @param statBuffer    文件数据信息
     * @param index     文件索引
     * @param name      文件名
     * @param isMutilFolderFile
     * @return
     */
    Archive::Entry *setEntryData(const zip_stat_t &statBuffer, qlonglong index, const QString &name, bool isMutilFolderFile = false);
    Archive::Entry *setEntryDataA(const zip_stat_t &statBuffer, qlonglong index, const QString &name);
    void setEntryVal(const zip_stat_t &statBuffer, int &index, const QString &name, QString &dirRecord);

    void setEntryVal1(const zip_stat_t &statBuffer, int &index, const QString &name, QString &dirRecord);

private:
    QVector<Archive::Entry *> m_emittedEntries;     // 存储entry
    bool m_overwriteAll;        //是否全部覆盖
    bool m_skipAll;             // 是否全部跳过
    bool m_listAfterAdd;        // 压缩之后是否list
    int m_filesize;             // 压缩的文件数目
    zip_t *m_addarchive;        // 压缩包
    // QByteArray m_codecstr;
    QByteArray m_codecname;     // 探测编码
    ExtractionOptions m_extractionOptions;  // 解压参数
    //bool isWrongPassword = false;
    QString m_extractDestDir;   // 解压路径
    QString m_extractFile;      // 解压文件名

    QStringList m_listCodecs;   // 中文编码格式
    QMap<QString, QPair<zip_stat_t, qlonglong>> m_listMap;  // 压缩包数据存储
    QString m_DirRecord;
    QString m_SigDirRecord;
    int m_indexCount = 0;

    QList<int> m_listExtractIndex;  // 需要解压的文件索引
    QString m_strRootNode;  // 父节点
    //    QMap<QString, QString> m_fileNameEncodeMap;

    bool m_bCancel = false;     // 是否取消
    Common *m_common = nullptr; // 通用工具类
};

#endif // LIBZIPPLUGIN_H
