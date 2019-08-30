/*
 * Copyright (C) 2017 ~ 2018 Deepin Technology Co., Ltd.
 *
 * Author:     rekols <rekols@foxmail.com>
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
#include <DApplication>
#include <DWidgetUtil>
#include <QCommandLineParser>
#include <QDebug>
#include <DLog>
#include "utils.h"
#include <DObject>

DWIDGET_USE_NAMESPACE
DCORE_USE_NAMESPACE

int main(int argc, char *argv[])
{
    // load dtk xcb plugin.
    DApplication::loadDXcbPlugin();

    // init Dtk application's attrubites.
    DApplication app(argc, argv);
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
    app.loadTranslator();
    app.setOrganizationName("deepin");
    app.setApplicationVersion(DApplication::buildVersion("20190828"));
    app.setApplicationAcknowledgementPage("https://www.deepin.org/original/deepin-XXX-XXX/");
    app.setProductIcon(QIcon(":/images/deepin-compressor.svg"));
    app.setProductName(DApplication::translate("Main", "Deepin Compressor"));
    app.setApplicationDescription(DApplication::translate("Main","Deepin Compressor is used to XXXX."));

    DLogManager::registerConsoleAppender();
    DLogManager::registerFileAppender();

    // add command line parser to app.
    QCommandLineParser parser;
    parser.setApplicationDescription("Deepin Compressor.");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("filename", "Font file path.", "file [file..]");
    parser.process(app);

    // init modules.
    MainWindow w;
    w.setMinimumSize(520, 440);
    w.setWindowIcon(QIcon(":/images/deepin-compressor.svg"));
    w.show();

    if (app.setSingleInstance("deepin-compressor")) {
        Dtk::Widget::moveToCenter(&w);
    }

    const QStringList fileList = parser.positionalArguments();

    // handle command line parser.
    if (!fileList.isEmpty()) {
        QMetaObject::invokeMethod(&w, "onSelected", Qt::QueuedConnection, Q_ARG(QStringList, fileList));
    }

    QObject::connect(&w, &MainWindow::sigquitApp, &app, DApplication::quit);

    return app.exec();
}
