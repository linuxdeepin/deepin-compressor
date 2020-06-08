/*
 * Copyright (C) 2019 ~ 2019 Deepin Technology Co., Ltd.
 *
 * Author:     dongsen <dongsen@deepin.com>
 *
 * Maintainer: dongsen <dongsen@deepin.com>
 *             AaronZhang <ya.zhang@archermind.com>
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
#include <DWidgetUtil>
#include <QCommandLineParser>
#include <DLog>
#include "utils.h"
#include <DApplicationSettings>
#include "openwithdialog/openwithdialog.h"

int main(int argc, char *argv[])
{
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    // load dtk xcb plugin.
    DApplication::loadDXcbPlugin();

    // init Dtk application's attrubites.
    CompressorApplication app(argc, argv);

    app.loadTranslator();
    app.setOrganizationName("deepin");
    app.setApplicationName("deepin-compressor");
    app.setApplicationVersion(DApplication::buildVersion(QDate::currentDate().toString("yyyyMMdd")));
    app.setApplicationAcknowledgementPage("https://www.deepin.org/original/deepin-compressor/");
    app.setProductIcon(QIcon::fromTheme("deepin-compressor"));
    app.setProductName(DApplication::translate("Main", "Archive Manager"));
    app.setApplicationDescription(DApplication::translate("Main", "Archive Manager is a fast and lightweight application for creating and extracting archives."));
    DApplicationSettings settings(&app);

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    // add command line parser to app.
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Compressor.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", "File path.", "file [file..]");
    parser.process(app);

    // init modules.
    MainWindow w;
    app.setMainWindow(&w);

    if (!w.checkSettings(argv[1])) {
        return  app.exec();
    }

    QIcon appIcon = QIcon::fromTheme("deepin-compressor");

    if (appIcon.isNull()) {
        appIcon = QIcon(":assets/icons/deepin/builtin/icons/deepin-compressor.svg");
    }

    app.setProductIcon(appIcon);
    app.setWindowIcon(appIcon);
    //w.titlebar()->setIcon(appIcon);

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

    QObject::connect(&w, &MainWindow::sigquitApp, &app, DApplication::quit);
    // handle command line parser.
    if (!fileList.isEmpty()) {
        QMetaObject::invokeMethod(&w, "onRightMenuSelected", Qt::DirectConnection, Q_ARG(QStringList, newfilelist));
    }

    w.show();


    return app.exec();
}
