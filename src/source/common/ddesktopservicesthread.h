#ifndef DDESKTOPSERVICESTHREAD_H
#define DDESKTOPSERVICESTHREAD_H

#include <QThread>

#include <DDesktopServices>
#include <QDebug>
#include <QFileInfo>
DWIDGET_USE_NAMESPACE

// 文管打开文件目录线程
class DDesktopServicesThread : public QThread
{
public:
    explicit DDesktopServicesThread(QObject *parent = nullptr);

    /**
     * @brief setOpenFile   设置打开路径或文件
     * @param strFullPath   打开目录
     */
    void setOpenFile(const QString &strFullPath);

protected:
    /**
     * @brief run   运行
     */
    void run();

private:
    QString m_strFullPath;      // 打开目录
};

#endif // DDESKTOPSERVICESTHREAD_H
