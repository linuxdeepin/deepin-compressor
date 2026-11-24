// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef CONTRACT_BACKEND_FACTORY_H
#define CONTRACT_BACKEND_FACTORY_H

#include "archive_backend.h"

#include <functional>
#include <memory>

namespace dcarchive {

using BackendFactory = std::function<std::unique_ptr<ArchiveBackend>()>;

} // namespace dcarchive

#endif // CONTRACT_BACKEND_FACTORY_H
