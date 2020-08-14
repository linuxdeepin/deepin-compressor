#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QVector>

#define MS_FILEWATCHER (1000) //文件检测时间间隔
#define MS_TIMERWATCHER (800) //超时时间

class FileWatcher : public QObject
{
    Q_OBJECT

public:
    FileWatcher(QObject *parent = nullptr);
    ~FileWatcher();
    void handleTimeout();
    void watch(QStringList *fullPath);
    void beginWork();
    void finishWork();
signals:
    void sigFileChanged(QString fileChanged);

protected:
    virtual void timerEvent(QTimerEvent *event);

private:
    int m_nTimerID = -1;
    QVector<QString> *mFileList = nullptr;
};

/**
 * define a type named pMember_callback which is member function of class QObject(the format : bool ()  )
 */
typedef bool (QObject::*pMember_callback)();

/**
 * @brief The TimerWatcher class
 * @see 如果需要一个定时器，或者某个延时处理，可以使用该类
 */
class TimerWatcher : public QObject
{
    Q_OBJECT

public:
    TimerWatcher(QObject *parent = nullptr);
    ~TimerWatcher();
    void beginWork(int ms = 1000);
    void finishWork();

    /**
     * @brief bindFunction
     * @param pObj
     * @param callback
     * @see 绑定一个目标对象pObj和定时处理函数pMember_callback
     */
    void bindFunction(QObject *pObj, pMember_callback callback);

signals:
    void sigBindFuncDone(bool result);

public:
    pMember_callback callback = nullptr;
    QObject *pCaller = nullptr;

protected:
    virtual void timerEvent(QTimerEvent *event);

private:
    int m_nTimerID = -1;
};

#endif // FILEWATCHER_H
