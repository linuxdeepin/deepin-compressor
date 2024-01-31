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
    /**
     * @brief onActiveWindow    激活窗口
     * @param pid    对应进程id
     */
    void onActiveWindow(qint64 pid);

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

#endif /* ifndef _DMR_DBUS_ADAPTOR */
