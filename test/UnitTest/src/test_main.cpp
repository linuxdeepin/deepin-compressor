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

#include <gtest/gtest.h>
#include <gtest/src/stub.h>
//#include <gmock/gmock-matchers.h>

#include "pluginmanager.h"
#include "kpluginloader.h"
#include "config.h"

#include <QSet>
#include <QApplication>
#include <iostream>

void loadPlugins_stub(void *obj)
{
    PluginManager *mythis = static_cast<PluginManager *>(obj);

    QCoreApplication::addLibraryPath(BUILD_PATH);
    const QVector<KPluginMetaData> plugins = KPluginLoader::findPlugins(QStringLiteral("./lib"));
    QSet<QString> addedPlugins;
    for (const KPluginMetaData &metaData : plugins) {
        const auto pluginId = metaData.pluginId();
        // Filter out duplicate plugins.
        if (addedPlugins.contains(pluginId)) {
            continue;
        }

        Plugin *plugin = new Plugin(mythis, metaData);
        plugin->setEnabled(true);
        addedPlugins << pluginId;
        mythis->m_plugins << plugin;
    }
}

int main(int argc, char *argv[])
{
    std::cout << "Starting UnitTest" << std::endl;
    qputenv("QT_QPA_PLATFORM", "offscreen");
    testing::InitGoogleTest(&argc, argv);
    QApplication a(argc, argv);

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

    return RUN_ALL_TESTS();
}
