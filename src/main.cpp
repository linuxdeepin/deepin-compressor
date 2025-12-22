// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mainwindow.h"
#include "compressorapplication.h"
#include "environments.h"
#include "DebugTimeManager.h"
#include "processopenthread.h"
#include "uitools.h"

#include <DWidgetUtil>
#include <DLog>
#include <DApplicationSettings>

#include <QCommandLineParser>
#include <QMessageBox>
#include <QGuiApplication>
#include <QDBusConnection>
#include <QDBusInterface>
#include <QDBusPendingCall>
#include <QJsonParseError>
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QByteArray>
#include "common/dbusadpator.h"
#include "eventlogutils.h"
#ifdef DTKWIDGET_CLASS_DWaterMarkHelper
#include <DWaterMarkHelper>
DWIDGET_USE_NAMESPACE
#endif
DCORE_USE_NAMESPACE

/**
 * @brief showWatermark     显示窗体水印
 * @param sJson             json字符串
 * @param w                 窗体
 * font 使用的字体名称
 * fontSize 字体PT字号(0,∞)
 * color 000000～FFFFFF 16进制颜色字符串
 * opacity 0~255透明度，0为全透明，255为不透明
 * layout 0,1布局类型，0为居中，1为平铺
 * angle 0,359水印倾度，顺时针旋转角度
 * rowSpacing 平铺时使用，不同行间距像素(0,∞)
 * columnSpacing 平铺时使用，行内水印间接像素(0,∞)
 * text 水印文本，utf-8编码
 */
void showWatermark(const QString &sJson, MainWindow *w)
{
    QJsonParseError error;
    QJsonObject metaData = QJsonDocument::fromJson(sJson.toLocal8Bit().data(), &error).object();
    QVariantMap mapwaterMark, mapdata = metaData.toVariantMap();
    if(mapdata.contains("wndWatermark")) {
        mapwaterMark = mapdata.value("wndWatermark").toMap();
    }
    if(mapwaterMark.isEmpty()) return;

#ifdef DTKWIDGET_CLASS_DWaterMarkHelper
    auto ins = DWaterMarkHelper::instance();
    WaterMarkData data = ins->data();
    // DTKWidget 主线和定制线的水印接口不同，通过版本进行区分
    // 主线水印接口在 5.6.9 之后引入.
    // 因此，判断定制线：存在水印接口，版本不低于 5.4.42.7 且低于 5.6.9
#if DTK_VERSION_CHECK(5, 4, 42, 7) <= DTK_VERSION && DTK_VERSION < DTK_VERSION_CHECK(5, 6, 9, 0)
    //接口变动为结构体
    data.text = mapwaterMark.value("text").toString();
    data.type = WaterMarkType::Text;
    data.layout = mapwaterMark.value("layout").toInt() == 1 ? WaterMarkLayout::Tiled : WaterMarkLayout::Center;
    data.rotation = mapwaterMark.value("angle").toInt();
    data.color = QColor(mapwaterMark.value("color").toString().toUInt(nullptr, 16));
    auto font = QFont(mapwaterMark.value("font").toString());
    font.setPointSize(mapwaterMark.value("fontSize").toInt());
    data.font = font;
    data.spacing = mapwaterMark.value("columnSpacing").toInt();
    data.lineSpacing = mapwaterMark.value("rowSpacing").toInt();
    data.opacity = mapwaterMark.value("opacity").toInt()/255.0;
    data.grayScale = true;
    //显示放大倍数
    if(mapwaterMark.contains("ignoreDevicePixelRatio") && mapwaterMark.value("ignoreDevicePixelRatio").toBool()){
        qreal deviceRatio = qApp->devicePixelRatio();
        data.font.setPointSizeF(data.font.pointSizeF()/deviceRatio);
        data.spacing /= deviceRatio;
        data.lineSpacing /= deviceRatio;
    }

#else
    data.setText(mapwaterMark.value("text").toString());
    data.setType(WaterMarkData::Text);
    data.setLayout(mapwaterMark.value("layout").toInt() == 1 ? WaterMarkData::Tiled : WaterMarkData::Center);
    data.setRotation(mapwaterMark.value("angle").toInt());
    data.setColor(QColor(mapwaterMark.value("color").toString().toUInt(nullptr, 16)));
    auto font = QFont(mapwaterMark.value("font").toString());
    font.setPointSize(mapwaterMark.value("fontSize").toInt());
    data.setFont(font);
    data.setSpacing(mapwaterMark.value("columnSpacing").toInt());
    data.setLineSpacing(mapwaterMark.value("rowSpacing").toInt());
    data.setOpacity(mapwaterMark.value("opacity").toInt()/255.0);
    data.setGrayScale(true);
    //显示放大倍数
    if(mapwaterMark.contains("ignoreDevicePixelRatio") && mapwaterMark.value("ignoreDevicePixelRatio").toBool()){
        qreal deviceRatio = qApp->devicePixelRatio();
        QFont ft = data.font();
        ft.setPointSizeF(ft.pointSizeF()/deviceRatio);
        data.setFont(ft);
        data.setSpacing(data.spacing() / deviceRatio);
        data.setLineSpacing(data.lineSpacing() / deviceRatio);
    }
#endif
    ins->setData(data);
    ins->registerWidget(w);
#endif
}

int main(int argc, char *argv[])
{
    //for qt5platform-plugins load DPlatformIntegration or DPlatformIntegrationParent
    if (!QString(qgetenv("XDG_CURRENT_DESKTOP")).toLower().startsWith("deepin")) {
        setenv("XDG_CURRENT_DESKTOP", "Deepin", 1);
    }
    bool orderObject = false;
    QString sJsonStr;
    PERF_PRINT_BEGIN("POINT-01", "打开时间");

    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);    // 使用高分屏

    // 初始化DTK应用程序属性
    CompressorApplication app(argc, argv);
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-compressor");
    app.loadTranslator();
    app.setApplicationVersion(DApplication::buildVersion(QDate::currentDate().toString("yyyyMMdd")));
    app.setApplicationAcknowledgementPage("https://www.deepin.org/original/deepin-compressor/");
    app.setProductIcon(QIcon::fromTheme("deepin-compressor"));
    app.setApplicationVersion(VERSION);
    app.setProductName(DApplication::translate("Main", "Archive Manager"));
    app.setApplicationDisplayName(DApplication::translate("Main", "Archive Manager"));
    app.setApplicationDescription(DApplication::translate("Main", "Archive Manager is a fast and lightweight application for creating and extracting archives."));
    DApplicationSettings settings(&app);
    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    // 设置应用图标
    QIcon appIcon = QIcon::fromTheme("deepin-compressor");

    if (appIcon.isNull()) {
        appIcon = QIcon(":assets/icons/deepin/builtin/icons/deepin-compressor.svg");
    }

    app.setProductIcon(appIcon);
    app.setWindowIcon(appIcon);

    // 命令行参数的解析
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Compressor.");
    parser.addHelpOption();     //添加帮助信息(-h,--help)，这个方法由QCommandLineParser自动处理
    parser.addVersionOption();  //添加（-v,--version）方法，显示应用的版本，这个方法由QCommandLineParser自动处理
    parser.addPositionalArgument("filename", "File path.", "file [file..]");    // 添加选项
    /* 处理命令行参数。
       除了解析选项（如parse（））外，此函数还处理内置选项并处理错误。
       如果调用了addVersionOption，则内置选项为--version，如果调用了addHelpOption，则为--help     --help-all。
       当调用这些选项之一时，或者当发生错误（例如，传递了未知选项）时，当前进程将使用exit（）函数停止。
    */
    // 文件名过滤
    QStringList newfilelist;
    if(argc == 3 && QString(argv[2]).startsWith("--param="))
    {
        orderObject = true;
        QString slast = QString(argv[2]).remove(0,strlen("--param="));
        //接收时需要转换为可用数据
        qInfo() << "last:" << slast;
        sJsonStr = QByteArray::fromBase64(slast.toLatin1().data()).data();
//            sJsonStr = sList.last();
        qInfo() << "json:" << sJsonStr;
        QString file = argv[1];
        if (file.contains("file://")) {
            file.remove("file://");
        }
        newfilelist.append(file);
        newfilelist.append(QString(argv[2]));
    } else {
        parser.process(app);
        foreach (QString file, parser.positionalArguments()) {
            if (file.contains("file://")) {
                file.remove("file://");
            }
            newfilelist.append(file);
        }
    }

    qInfo() << "传入参数：" << newfilelist;
    if (orderObject) {
        //启动由wps控制，本地不做控制
        newfilelist.removeLast();
    }

    // 创建主界面
    MainWindow w;
    showWatermark(sJsonStr, &w);

    // 默认居中显示（使用dbus判断是否为第一个进程，第一个进程居中显示）
    ApplicationAdaptor adaptor(&app);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    if(!orderObject){
        if (dbus.registerService("com.deepin.compressor")) {
            Dtk::Widget::moveToCenter(&w);
        }
    } else {
        if (dbus.registerService("com.deepin.compressor"+QString::number(QGuiApplication::applicationPid()))) {
             dbus.registerObject("/"+QString::number(QGuiApplication::applicationPid()), &app);
             adaptor.setCompressFile(newfilelist.first());
             Dtk::Widget::moveToCenter(&w);
             w.setProperty(ORDER_JSON, sJsonStr);
        }
        QDBusConnection dbusConnection = QDBusConnection::sessionBus();

        if (dbusConnection.connect("com.wps.cryptfs"
                                   , "/com/wps/cryptfs"
                                   , "cryptfs.method.Type"
                                  , "activateProcess"
                                  , &adaptor
                                  , SLOT(onActiveWindow(qint64))
                                 ))
        {
            qInfo() << "DBus connect success!";
        } else {
            qInfo() << "DBus connect failed!";
        };
    }


    // 对文件名进行处理
    QStringList listTransFiles;
    QStringList listSameFile;
    if (newfilelist.count() != 0) {
        // 压缩时不处理文件名，其余情况处理
        if (newfilelist.last() != "compress" && newfilelist.last() != "compress_to_7z" && newfilelist.last() != "compress_to_zip") {
            for (int i = 0; i < newfilelist.count(); i++) {
                QString strFileName = newfilelist[i];
                UnCompressParameter::SplitType eSplitVolume;
                UiTools::transSplitFileName(strFileName, eSplitVolume);
                if (!listTransFiles.contains(strFileName)) {
                    listTransFiles << strFileName;
                }
            }
        } else {
            listTransFiles = newfilelist;
        }
    } else {
        listTransFiles = newfilelist;
    }



    /*  说明：文件(可以是多个) + 参数
     *  参数为：compress：压缩，例/home/.../xx.txt /home/.../xx1.txt compress
     *  参数为：compress_to_7z：压缩成7z格式，例/home/.../xx.txt /home/.../xx1.txt compress_to_7z
     *  参数为：compress_to_zip：压缩成zip格式，例/home/.../xx.txt /home/.../xx1.txt compress_to_zip
     *  参数为：extract：解压缩，例/home/.../xx.zip /home/.../xx1.zip extract
     *  参数为：extract_here：解压到当前文件夹，例/home/.../xx.zip /home/.../xx1.zip extract_here
     *  参数为：extract_to_specifypath：解压到指定文件夹，例/home/.../xx.zip /home/.../xx1.zip extract_to_specifypath
     *  参数为：dragdropadd：文管拖拽压缩，例/home/.../xx.zip /home/.../xx.txt /home/.../xx1.txt dragdropadd
     *  其余情况为同时打开多个文件（开启多个归档管理器）
    */
    if (!listTransFiles.isEmpty()) {
        MainWindow::ArgumentType eType;
        int iParamCount = listTransFiles.count();      // 获取参数数目


        if (iParamCount == 1) {
            // 只有一个参数（即直接当作压缩包打开）
            eType = MainWindow::AT_Open;
            listTransFiles << listTransFiles[0];
            // 对文件类型进行检查处理
            if (!w.checkSettings(listTransFiles[0])) {
                app.exit();
                return 0;
            }
        } else {
            // 多个参数的情况下，将最后一个参数作为标识，判断即将进行的是何种操作
            QString strType = listTransFiles.last();

            if (strType == "dragdropadd") {
                // 最后一个参数为“dragdropadd”时，说明是拖拽追加
                eType = MainWindow::AT_DragDropAdd;
                QFileInfo info(newfilelist.first());
                if (info.isSymLink()) {
                    QUrl fileName = QUrl::fromLocalFile(info.symLinkTarget());
                    if (!UiTools::isLocalDeviceFile(fileName.toLocalFile())) {
                        return -1;
                    }
                }
            } else if (strType == "compress" || strType == "compress_to_7z" || strType == "compress_to_zip" ||
                       strType == "extract" || strType == "extract_here" || strType == "extract_to_specifypath") {
                // 右键操作
                eType = MainWindow::AT_RightMenu;

                // 不对压缩逻辑进行格式处理
                if (strType != "compress" && strType != "compress_to_7z" && strType != "compress_to_zip") {
                    // 对文件类型进行检查处理
                    if (!w.checkSettings(listTransFiles[0])) {
                        app.exit();
                        return 0;
                    }
                }

                // 检查解压到当前文件夹路径是否有权限，若没有权限，给出提示并退出应用
                if (!w.checkHerePath(QFileInfo(listTransFiles[0]).path())) {
                    app.quit();
                    return 0;
                }

            } else {
                // 在线程中执行外部应用打开的命令
                for (int i = 1; i < iParamCount; i++) {
                    ProcessOpenThread *p = new ProcessOpenThread;
                    p->setProgramPath(QStandardPaths::findExecutable("deepin-compressor"));
                    p->setArguments(QStringList() << listTransFiles[i]);
                    p->start();
                }

                // 打开第一个文件
                qInfo() << "打开文件";
                eType = MainWindow::AT_Open;
                QString strOpenFile = listTransFiles[0];
                listTransFiles.clear();
                listTransFiles << strOpenFile;
            }
        }

        QObject::connect(&w, &MainWindow::sigquitApp, &app, &DApplication::quit);
        QMetaObject::invokeMethod(&w, "slotHandleArguments", Qt::DirectConnection, Q_ARG(QStringList, listTransFiles), Q_ARG(MainWindow::ArgumentType, eType));
    } else {
        // 无参数打开应用
        w.show();
    }
    QObject::connect(&app, &DApplication::aboutToQuit,[=](){
        QString strOpenFile;
        if(!listTransFiles.isEmpty())
            strOpenFile = listTransFiles[0];
        if(strOpenFile.isEmpty()) {
            QJsonObject obj{
                {"tid", EventLogUtils::closeCompressWnd},
                {"operate", "closeCompressWnd"},
                {"describe", QString("Close Compress Window.")}
            };
            EventLogUtils::get().writeLogs(obj);
        } else {
            QJsonObject obj{
                {"tid", EventLogUtils::closeCompressWnd},
                {"operate", "closeCompressWnd"},
                {"describe", QString("Close Compress Window. Close compress file : ") + strOpenFile}
            };
            EventLogUtils::get().writeLogs(obj);
        }
    });
    PERF_PRINT_END("POINT-01");

    return app.exec();
}
