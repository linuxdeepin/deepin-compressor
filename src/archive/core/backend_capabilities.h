// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKEND_CAPABILITIES_H
#define BACKEND_CAPABILITIES_H

#include <QStringList>

#include "archive_types.h"

namespace archive {

struct BackendCapabilities {
    bool list = false;
    bool test = false;
    bool extract = false;
    bool create = false;
    bool append = false;
    bool move = false;
    bool copy = false;
    bool remove = false;
    bool rename = false;
    bool comment = false;
    bool update = false;
    bool password = false;
    bool headerPassword = false;
    bool multiVolume = false;
    bool progress = false;
    bool cancel = false;
    bool pause = false;

    // Check whether this capability set supports the given operation.
    // Maps each ArchiveOperation to the corresponding capability flag.
    bool supports(ArchiveOperation op) const
    {
        switch (op) {
        case ArchiveOperation::Open:    return true; // all backends can open
        case ArchiveOperation::List:    return list;
        case ArchiveOperation::Test:    return test;
        case ArchiveOperation::Extract: return extract;
        case ArchiveOperation::Add:     return create || append;
        case ArchiveOperation::Move:    return move;
        case ArchiveOperation::Copy:    return copy;
        case ArchiveOperation::Delete:  return remove;
        case ArchiveOperation::Rename:  return rename;
        case ArchiveOperation::Comment: return comment;
        case ArchiveOperation::Update:  return update;
        }
        return false;
    }
};

} // namespace archive

#endif // BACKEND_CAPABILITIES_H
