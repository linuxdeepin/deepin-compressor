// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DBUS_ADAPTOR
#define DBUS_ADAPTOR

#include <QtDBus/QDBusAbstractAdaptor>

#include "QApplication"
#include <QWidget>
#include <QSettings>

/**
 * @file d-bus适配器，开放归档管理器d-bus接口
 */
class ApplicationAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.compressor")
    Q_CLASSINFO("D-Bus Introspection", ""
                "  <interface name=\"com.deepin.compressor\">\n"

                "    <method name=\"raise\">\n"
                "      <arg direction=\"in\" type=\"none\" name=\"raise\"/>\n"
                "    </method>\n"

                "  </interface>\n")

public:
    ApplicationAdaptor(QApplication *application);
    /**
     * @brief setCurOpenFile    设置当前的压缩包文件名
     * @param sFile    压缩包文件名
     */
    void setCompressFile(const QString &sFile);


public Q_SLOTS:
    /**
     * @brief raise    dbus接口
     * @param sFile    压缩包文件名的窗口激活
     */
    void raise(const QString &sFile);

private slots:
    /**
     * @brief focusChangedSlot    激活窗口切换
     * @param pwgt    激活的窗口
     */
     void focusChangedSlot(QWidget *pwgt, QWidget *)
     {
         if(pwgt)
            m_curShowWidget = pwgt;
     }
private:
    QApplication *app = nullptr;            //运行应用程序实例
    QWidget *m_curShowWidget = nullptr;     //当前显示窗口
    QString m_sFile;                        //压缩包文件名
};


class CompressSetting: public QObject {
private:
    /**
     * @brief isExistPid    压缩包文件名是否存在打开窗口
     * @param sFile         压缩包文件名
     * @param pid           进程号
     */
    bool isExistPid(const QString &sFile, const int &pid);
public:
    explicit CompressSetting(QObject *pMainWid);
    /**
     * @brief isExistSetFile    压缩包文件名是否存在打开窗口
     * @param sFile    压缩包文件名
     * @return true表示存在，其他表示不存在
     */
    bool isExistSetFile(const QString &sFile);
    /**
     * @brief appendDataSetting    seting中添加压缩包文件名与进程号绑定
     * @param sFile     压缩包文件名
     * @param pid       进程号
     */
    void appendDataSetting(const QString &sFile, const int &pid);
    /**
     * @brief removeDataSetting    seting中移除压缩包文件名绑定
     * @param sFile     压缩包文件名
     */
    void removeDataSetting(const QString &sFile);
    /**
     * @brief dataSetting   获取seting中绑定压缩包文件名的进程号
     * @param sFile         压缩包文件名
     * @return              绑定的进程号
     */
    QString dataSetting(const QString &sFile);
private:
     QSettings *m_pSettings;   ///QSetting指针
};

#endif /* ifndef _DMR_DBUS_ADAPTOR */
