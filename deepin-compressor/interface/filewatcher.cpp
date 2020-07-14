#include "filewatcher.h"

#include <QDebug>
#include <QTimerEvent>
#include <QFileInfo>

#define TIMER_TIMEOUT   (1000)

FileWatcher::FileWatcher(QObject *parent)
    : QObject(parent)
{
}

FileWatcher::~FileWatcher()
{
    this->finishWork();

    if (mFileList != nullptr) {
        delete mFileList;
        mFileList = nullptr;
    }
}

void FileWatcher::beginWork()
{
    m_nTimerID = this->startTimer(MS_FILEWATCHER);
}

void FileWatcher::finishWork()
{
    qDebug() << "Enter timeout processing function\n";
    killTimer(m_nTimerID);
    qDebug() << m_nTimerID;
}

void FileWatcher::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_nTimerID) {
        handleTimeout();
    }
}

void FileWatcher::watch(QStringList *fullPath)
{
    if (this->mFileList != nullptr) {
        this->mFileList->clear();
        delete mFileList;
    }

    this->mFileList = new QVector<QString>(fullPath->toVector());
}

void FileWatcher::handleTimeout()
{
    for (int i = 0; i < mFileList->length(); i++) {
        QFileInfo info((*mFileList)[i]);
        if (info.exists() == false) {
            emit sigFileChanged((*mFileList)[i]);
            break;
        }
    }
}



TimerWatcher::TimerWatcher(QObject *parent)
    : QObject(parent)
{
}

TimerWatcher::~TimerWatcher()
{
    this->finishWork();
}

void TimerWatcher::beginWork(int ms)
{
    m_nTimerID = this->startTimer(ms);
}

void TimerWatcher::finishWork()
{
    killTimer(m_nTimerID);
    qDebug() << "finishWork";
}

void TimerWatcher::bindFunction(QObject *pWnd, pMember_callback callback)
{
    this->pCaller = pWnd;
    this->callback = callback;
}

void TimerWatcher::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_nTimerID) {
        if (this->pCaller == nullptr || this->callback == nullptr) {
            return;
        }
        bool result = (this->pCaller->*callback)();
        if (result == true) {
            emit sigBindFuncDone(result);
        } else {
            qDebug() << "condition not ok";
        }
        this->finishWork();//stop timer work
    }
}

