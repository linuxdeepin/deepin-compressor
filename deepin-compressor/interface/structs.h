#ifndef STRUCTS_H
#define STRUCTS_H

#include <QDebug>
#include <QElapsedTimer>

/**
 * @brief The ProgressAssistant struct
 * @see 进度条助手
 * @author added by hsw 20200610
 */
class ProgressAssistant: public QObject
{
public:
    explicit ProgressAssistant(QObject *parent = nullptr);
    void startTimer();
    void restartTimer();
    /**
     * @brief resetProgress
     * @see 重置进度条各项数据（总大小，耗时，百分值）
     */
    void resetProgress();

    void setTotalSize(qint64 size);

    qint64 &getTotalSize();

    double getSpeed(unsigned long percent, bool isConvert = false);

    qint64 getLeftTime(unsigned long percent, bool isConvert = false);

private:
    qint64 consumeTime;                 //消耗时间
    QElapsedTimer m_timer;
    unsigned long m_lastPercent = 0;
    qint64 m_totalFileSize = 0;         //处理的文件总大小
};


/**
 * @brief The Settings_Extract_Info struct
 * @see 为了让job能自己处理取消删除的相关操作，所以用该类去传递相关必要数据到job层
 * @author added by hsw 20200619
 */

struct Settings_Extract_Info {
    QString str_defaultPath = ""; // 默认解压路径
    /**
     * @see 如果是自动创建文件夹，并且解压的是多个目录，那么该值是自动创建的文件夹名
     *      如果是自动创建文件夹，并且解压的是单个目录，那么该值是单个目录的名字
     *      如果非自动创建文件夹，并且解压的是多个目录，那么该值是空字符串
     *      如果是自动创建文件夹，并且解压的是单个目录，那么该值是单个目录的名字
     */
    QString str_CreateFolder = "";
    bool b_isAutoCreateDir = false; // true:自动创建文件夹（多目录解压）
//    bool b_openDirJobDone; // true:当解压完成后自动打开对应的文件夹
};

#endif // STRUCTS_H
