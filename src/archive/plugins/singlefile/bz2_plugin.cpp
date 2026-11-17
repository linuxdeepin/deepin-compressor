// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "bz2_plugin.h"

Bz2PluginFactory::Bz2PluginFactory()
{
    registerPlugin<SingleFileBackend>();
}

Bz2PluginFactory::~Bz2PluginFactory() = default;
