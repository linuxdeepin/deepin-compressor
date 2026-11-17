// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "gz_plugin.h"

GzPluginFactory::GzPluginFactory()
{
    registerPlugin<SingleFileBackend>();
}

GzPluginFactory::~GzPluginFactory() = default;
