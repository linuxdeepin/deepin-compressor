// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "pzip/common.h"

#include <cstdint>
#include <vector>

namespace pzip {

// WinZip AES extra 的 Vendor ID：ASCII "AE"（按 ZIP 小端存储为 0x4541）
constexpr uint16_t WINZIP_AES_VENDOR_ID_AE = 0x4541;

/**
 * @brief WinZip AES Extra Field (0x9901) 编码/解码
 *
 * 与 ZIP Local/CD Header 中的 extra 字段布局对应；供 ZipWriter 等使用。
 */
struct WinZipAESExtra {
    uint16_t vendorVersion = WINZIP_AES_VERSION_2; // AE-1 / AE-2
    uint16_t vendorId = WINZIP_AES_VENDOR_ID_AE;  // "AE"
    uint8_t aesStrength = 0;                      // 1=AES-128, 2=AES-192, 3=AES-256
    uint16_t actualCompressionMethod = ZIP_METHOD_STORE;

    std::vector<uint8_t> encode() const;
    static WinZipAESExtra decode(const uint8_t* data, size_t size);
};

} // namespace pzip
