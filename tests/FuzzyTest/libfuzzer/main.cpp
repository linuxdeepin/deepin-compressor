// SPDX-FileCopyrightText: 2022-2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "encoding_detector.h"

#include <cstdint>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size)
{
    const encdet::Result result = encdet::detect(
            reinterpret_cast<const char *>(data), size);
    (void)result;
    return 0;
}
