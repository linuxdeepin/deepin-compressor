// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "libzip_plugin.h"

LibzipFactory::LibzipFactory()
{
    registerPlugin<LibzipBackend>();
}

LibzipFactory::~LibzipFactory() = default;
