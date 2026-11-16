// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "legacy_backend_registry.h"
#include "uitools.h"
#include "archiveinterface.h"

namespace archive {

// Production factory: delegates to UiTools::createInterface().
// This file is compiled only in the application build (src/CMakeLists.txt
// glob-recurses archive/*.cpp).  In test builds a stub is linked instead,
// keeping the test binary free of UiTools/KDE plugin dependencies.
LegacyInterfaceFactory defaultLegacyInterfaceFactory()
{
    return [](const QString &archivePath, bool bWrite) -> ReadOnlyArchiveInterface * {
        return UiTools::createInterface(archivePath, bWrite);
    };
}

} // namespace archive
