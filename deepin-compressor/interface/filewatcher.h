#ifndef FILEWATCHER_H
#define FILEWATCHER_H

#include <QObject>
#include <QVector>

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

#endif // FILEWATCHER_H
