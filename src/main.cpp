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
                    && lastStr != "extract_split_multi" && lastStr != "dragdropadd") {
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
    //Dtk::Widget::moveToCenter(&w);  // 居中显示

    PERF_PRINT_END("POINT-01");

    return app.exec();
}
