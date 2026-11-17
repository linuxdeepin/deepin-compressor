// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "minizip_plugin.h"

MinizipFactory::MinizipFactory()
{
    registerPlugin<MinizipBackend>();
}

MinizipFactory::~MinizipFactory() = default;
