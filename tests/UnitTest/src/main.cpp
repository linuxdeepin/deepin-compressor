// Copyright (C) 2019 ~ 2020 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pluginmanager.h"
#include "kpluginloader.h"
#include "compressorapplication.h"

#include <gtest/gtest.h>
#include <gtest/src/stub.h>

#include <QSet>
#include <QApplication>
#include <iostream>

#if defined(CMAKE_SAFETYTEST_ARG_ON)
#include <sanitizer/asan_interface.h>
#endif

void loadPlugins_stub(void *obj)
{
    PluginManager *mythis = static_cast<PluginManager *>(obj);

    const QString sourceDir = QStringLiteral(_SOURCEDIR);
    const QStringList metadataFiles{
        sourceDir + QStringLiteral("/3rdparty/cli7zplugin/kerfuffle_cli7z.json"),
        sourceDir + QStringLiteral("/3rdparty/clipzipplugin/kerfuffle_clipzip.json"),
        sourceDir + QStringLiteral("/3rdparty/clirarplugin/kerfuffle_clirar.json"),
        sourceDir + QStringLiteral("/3rdparty/cliunarchiverplugin/kerfuffle_cliunarchiver.json"),
        sourceDir + QStringLiteral("/3rdparty/libarchive/readonlylibarchiveplugin/kerfuffle_libarchive_readonly.json"),
        sourceDir + QStringLiteral("/3rdparty/libarchive/readwritelibarchiveplugin/kerfuffle_libarchive.json"),
        sourceDir + QStringLiteral("/3rdparty/libminizipplugin/kerfuffle_libminizip.json"),
        sourceDir + QStringLiteral("/3rdparty/libpigzplugin/kerfuffle_libpigz.json"),
        sourceDir + QStringLiteral("/3rdparty/libsinglefileplugin/bz2plugin/kerfuffle_libbz2.json"),
        sourceDir + QStringLiteral("/3rdparty/libsinglefileplugin/gzplugin/kerfuffle_libgz.json"),
        sourceDir + QStringLiteral("/3rdparty/libsinglefileplugin/xzplugin/kerfuffle_libxz.json"),
        sourceDir + QStringLiteral("/3rdparty/libzipplugin/kerfuffle_libzip.json"),
    };
    QVector<KPluginMetaData> plugins;
    for (const QString &file : metadataFiles) {
        KPluginMetaData metadata(file);
        if (metadata.isValid())
            plugins.append(metadata);
    }
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
    CompressorApplication a(argc, argv);

    Stub stub;
    stub.set(ADDR(PluginManager, loadPlugins), loadPlugins_stub);

#if defined(CMAKE_SAFETYTEST_ARG_ON)
    __sanitizer_set_report_path("asan.log");
#endif

    return RUN_ALL_TESTS();
}
