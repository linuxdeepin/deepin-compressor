// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later
#ifndef DBUS_ADAPTOR
#define DBUS_ADAPTOR

#include <QDBusAbstractAdaptor>
#include <QApplication>
#include <QWidget>
#include <QSettings>
#include <QStringList>

class MainWindow;

/**
 * @file d-bus适配器，开放归档管理器d-bus接口
 */
class ApplicationAdaptor: public QDBusAbstractAdaptor
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "com.deepin.Compressor")
    Q_CLASSINFO("D-Bus Introspection", ""
                "  <interface name=\"com.deepin.Compressor\">\n"

                "    <method name=\"showWindow\">\n"
                "      <arg direction=\"out\" type=\"b\"/>\n"
                "    </method>\n"
                "    <method name=\"hideWindow\">\n"
                "      <arg direction=\"out\" type=\"b\"/>\n"
                "    </method>\n"
                "    <method name=\"raiseWindow\">\n"
                "      <arg direction=\"out\" type=\"b\"/>\n"
                "    </method>\n"
                "    <method name=\"quitWindow\">\n"
                "      <arg direction=\"out\" type=\"b\"/>\n"
                "    </method>\n"
                "    <method name=\"compressFiles\">\n"
                "      <arg direction=\"in\" type=\"as\" name=\"filePaths\"/>\n"
                "      <arg direction=\"out\" type=\"b\"/>\n"
                "    </method>\n"
                "    <method name=\"extractFiles\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"archivePath\"/>\n"
                "      <arg direction=\"in\" type=\"s\" name=\"destinationPath\"/>\n"
                "      <arg direction=\"out\" type=\"b\"/>\n"
                "    </method>\n"
                "    <method name=\"previewArchive\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"archivePath\"/>\n"
                "      <arg direction=\"out\" type=\"b\"/>\n"
                "    </method>\n"
                "    <method name=\"raise\">\n"
                "      <arg direction=\"in\" type=\"s\" name=\"sFile\"/>\n"
                "      <arg direction=\"out\" type=\"b\"/>\n"
                "    </method>\n"
                "  </interface>\n")

public:
    ApplicationAdaptor(MainWindow *mainwindow);
    /**
     * @brief setCurOpenFile    设置当前的压缩包文件名
     * @param sFile    压缩包文件名
     */
    void setCompressFile(const QString &sFile);
    void setMainWindow(MainWindow *mainWindow);

public Q_SLOTS:
    /**
     * @brief raise    dbus接口
     * @param sFile    压缩包文件名的窗口激活
     */
    bool raise(const QString &sFile);
    /**
     * @brief onActiveWindow    激活窗口
     * @param pid    对应进程id
     */
    void onActiveWindow(qint64 pid);

    // Window management methods
    bool showWindow();
    bool hideWindow();
    bool raiseWindow();
    bool quitWindow();
    
    // Compression related methods
    bool compressFiles(const QStringList &filePaths);
    bool extractFiles(const QString &archivePath, const QString &destinationPath);
    bool previewArchive(const QString &archivePath);

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
    MainWindow *m_mainWindow = nullptr;     //主窗口指针
};

#endif /* ifndef _DMR_DBUS_ADAPTOR */
