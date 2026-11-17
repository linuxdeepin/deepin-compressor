// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pigz_plugin.h"

PigzFactory::PigzFactory()
{
    registerPlugin<PigzBackend>();
}

PigzFactory::~PigzFactory() = default;
