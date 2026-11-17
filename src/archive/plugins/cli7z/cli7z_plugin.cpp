// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "cli7z_plugin.h"

Cli7zFactory::Cli7zFactory()
{
    registerPlugin<Cli7zBackend>();
}

Cli7zFactory::~Cli7zFactory() = default;
