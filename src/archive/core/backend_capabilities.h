// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef BACKEND_CAPABILITIES_H
#define BACKEND_CAPABILITIES_H

namespace dcarchive {

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
};

} // namespace dcarchive

#endif // BACKEND_CAPABILITIES_H
