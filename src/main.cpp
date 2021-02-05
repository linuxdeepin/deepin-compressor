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
     *  参数为：compress_to_7z：压缩成7z格式，例/home/.../xx.txt /home/.../xx1.txt compress_to_7z
     *  参数为：compress_to_zip：压缩成zip格式，例/home/.../xx.txt /home/.../xx1.txt compress_to_zip
     *  参数为：extract：解压缩，例/home/.../xx.zip /home/.../xx1.zip extract
     *  参数为：extract_here：解压到当前文件夹，例/home/.../xx.zip /home/.../xx1.zip extract_here
     *  参数为：extract_to_specifypath：解压到指定文件夹，例/home/.../xx.zip /home/.../xx1.zip extract_to_specifypath
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
            } else if (strType == "compress" || strType == "compress_to_7z" || strType == "compress_to_zip" ||
                       strType == "extract" || strType == "extract_here" || strType == "extract_to_specifypath") {
                // 右键操作
                eType = MainWindow::AT_RightMenu;

                // 不对压缩逻辑进行格式处理
                if (strType != "compress" && strType != "compress_to_7z" && strType != "compress_to_zip") {
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

    return app.exec();
}
