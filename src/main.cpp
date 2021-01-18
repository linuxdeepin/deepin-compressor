/*
* Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
*
* Author:     gaoxiang <gaoxiang@uniontech.com>
*
* Maintainer: gaoxiang <gaoxiang@uniontech.com>
*
* This program is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* any later version.
*
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "mainwindow.h"
#include "compressorapplication.h"
#include "environments.h"
#include "DebugTimeManager.h"
#include "processopenthread.h"

#include <DWidgetUtil>
#include <DLog>
#include <DApplicationSettings>

#include <QCommandLineParser>
#include <QMessageBox>
#include <QGuiApplication>

DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
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
    parser.process(app);

    // 文件名过滤
    QStringList newfilelist;
    foreach (QString file, parser.positionalArguments()) {
        if (file.contains("file://")) {
            file.remove("file://");
        }

        newfilelist.append(file);
    }

    qInfo() << "传入参数：" << newfilelist;


    // 创建主界面
    MainWindow w;

    // 默认居中显示
    if (app.setSingleInstance("deepin-compressor")) {
        Dtk::Widget::moveToCenter(&w);
    }

    /*  说明：文件(可以是多个) + 参数
     *  参数为：compress：压缩，例/home/.../xx.txt /home/.../xx1.txt compress
     *  参数为：compress_specifyFormat：压缩成指定格式（倒数第二个参数为生成的压缩包全路径），例/home/.../xx.txt /home/.../xx1.txt /home/.../xx.zip compress_specifyFormat
     *  参数为：extract：解压缩，例/home/.../xx.zip /home/.../xx1.zip extract
     *  参数为：extract_here：解压到当前文件夹，例/home/.../xx.zip /home/.../xx1.zip extract_here
     *  参数为：extract_to_path：解压到xx文件夹（倒数第二个参数为解压路径），例/home/.../xx.zip /home/.../xx1.zip /home/.../xx extract_to_path
     *  参数为：dragdropadd：文管拖拽压缩，例/home/.../xx.zip /home/.../xx.txt /home/.../xx1.txt dragdropadd
     *  其余情况为同时打开多个文件（开启多个归档管理器）
    */
    if (!newfilelist.isEmpty()) {
        MainWindow::ArgumentType eType;
        int iParamCount = newfilelist.count();      // 获取参数数目

        if (iParamCount == 1) {
            // 只有一个参数（即直接当作压缩包打开）
            eType = MainWindow::AT_Open;
            // 对文件类型进行检查处理
            if (!w.checkSettings(newfilelist[0])) {
                app.exit();
                return 0;
            }
        } else {
            // 多个参数的情况下，将最后一个参数作为标识，判断即将进行的是何种操作
            QString strType = newfilelist.last();

            if (strType == "dragdropadd") {
                // 最后一个参数为“dragdropadd”时，说明是拖拽追加
                eType = MainWindow::AT_DragDropAdd;
            } else if (strType == "compress" || strType == "compress_specifyFormat" ||
                       strType == "extract" || strType == "extract_here" || strType == "extract_to_path") {
                // 右键操作
                eType = MainWindow::AT_RightMenu;

                // 不对压缩逻辑进行格式处理
                if (strType != "compress" && strType != "compress_specifyFormat") {
                    // 对文件类型进行检查处理
                    if (!w.checkSettings(newfilelist[0])) {
                        app.exit();
                        return 0;
                    }
                }

                // 检查解压到当前文件夹路径是否有权限，若没有权限，给出提示并退出应用
                if (!w.checkHerePath(QFileInfo(newfilelist[0]).path())) {
                    app.quit();
                    return 0;
                }

            } else {
                // 在线程中执行外部应用打开的命令
                for (int i = 1; i < iParamCount; i++) {
                    ProcessOpenThread *p = new ProcessOpenThread;
                    p->setProgramPath(QStandardPaths::findExecutable("deepin-compressor"));
                    p->setArguments(QStringList() << newfilelist[i]);
                    p->start();
                }

                // 打开第一个文件
                qInfo() << "打开文件";
                eType = MainWindow::AT_Open;
                QString strOpenFile = newfilelist[0];
                newfilelist.clear();
                newfilelist << strOpenFile;
            }
        }

        QObject::connect(&w, &MainWindow::sigquitApp, &app, &DApplication::quit);
        QMetaObject::invokeMethod(&w, "slotHandleArguments", Qt::DirectConnection, Q_ARG(QStringList, newfilelist), Q_ARG(MainWindow::ArgumentType, eType));
    } else {
        // 无参数打开应用
        w.show();
    }

    PERF_PRINT_END("POINT-01");
    /*
    PERF_PRINT_BEGIN("POINT-01", "打开时间");
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

    // 初始化DTK应用程序属性
    CompressorApplication app(argc, argv);

    // add command line parser to app.
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Compressor.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", "File path.", "file [file..]");
    parser.process(app);

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

    MainWindow w;

    QString lastStr = argv[argc - 1];

    for (int i = 0; i < argc; i++) {
        qDebug() << "*********************" << argv[i];
    }

    if (argc >= 2) {
        if (argc >= 3) {
            if (lastStr != "extract_here" && lastStr != "extract_here_multi" && lastStr != "extract" && lastStr != "extract_multi"
                    && lastStr != "compress" && lastStr != "extract_here_split" && lastStr != "extract_split" && lastStr != "extract_here_split_multi"\
                    && lastStr != "extract_split_multi") {
                for (int i = 1; i < argc; i++) {
                    // 在线程中执行外部应用打开的命令
                    ProcessOpenThread *p = new ProcessOpenThread;
                    p->setProgramPath(QStandardPaths::findExecutable("deepin-compressor"));
                    p->setArguments(QStringList() << argv[i]);
                    p->start();
                }
                return 0;
            }
        }

        // 不对压缩逻辑进行格式处理
        if (lastStr != "compress") {
            // 对文件类型进行检查处理
            if (!w.checkSettings(argv[1])) {
                app.exit();
                return 0;
            }
        }
    }

    QIcon appIcon = QIcon::fromTheme("deepin-compressor");

    if (appIcon.isNull()) {
        appIcon = QIcon(":assets/icons/deepin/builtin/icons/deepin-compressor.svg");
    }

    app.setProductIcon(appIcon);
    app.setWindowIcon(appIcon);

    if (app.setSingleInstance("deepin-compressor")) {
        Dtk::Widget::moveToCenter(&w);
    }

    const QStringList fileList = parser.positionalArguments();
    QStringList newfilelist;
    foreach (QString file, fileList) {
        if (file.contains("file://")) {
            file.remove("file://");
        }

        newfilelist.append(file);
    }

    QStringList multilist;
    if (newfilelist.count() > 0 && ((newfilelist.last() == QStringLiteral("extract_here_split_multi") || newfilelist.last() == QStringLiteral("extract_split_multi")))) {
        multilist.append(newfilelist.at(0));
        multilist.append(newfilelist.last().remove("_multi"));
        newfilelist = multilist;
    }

    if (!newfilelist.isEmpty()) {
        // 检查解压到当前文件夹路径是否有权限，若没有权限，给出提示并退出应用
        QString strType = newfilelist.last();
        if (strType == QStringLiteral("extract_here") || strType == QStringLiteral("extract_multi")
                || strType == QStringLiteral("extract_here_multi") || strType == QStringLiteral("extract_here_split")) {
            if (!w.checkHerePath(QFileInfo(newfilelist[0]).path())) {
                app.quit();
                return 0;
            }
        }

        QObject::connect(&w, &MainWindow::sigquitApp, &app, &DApplication::quit);
        QMetaObject::invokeMethod(&w, "slotHandleRightMenuSelected", Qt::DirectConnection, Q_ARG(QStringList, newfilelist));
    } else {
        w.show();
        PERF_PRINT_END("POINT-01");
    }

    PERF_PRINT_END("POINT-01");
    */
    return app.exec();
}
