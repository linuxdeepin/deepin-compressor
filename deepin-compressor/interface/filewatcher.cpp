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
    m_nTimerID = this->startTimer(TIMER_TIMEOUT);
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
