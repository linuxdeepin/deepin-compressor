// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xz_plugin.h"

XzPluginFactory::XzPluginFactory()
{
    registerPlugin<SingleFileBackend>();
}

XzPluginFactory::~XzPluginFactory() = default;
