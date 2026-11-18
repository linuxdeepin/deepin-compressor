// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "libarchive_ro_plugin.h"

LibarchiveReadOnlyFactory::LibarchiveReadOnlyFactory()
{
    registerPlugin<ArchiveBackend>();
}

LibarchiveReadOnlyFactory::~LibarchiveReadOnlyFactory() = default;
