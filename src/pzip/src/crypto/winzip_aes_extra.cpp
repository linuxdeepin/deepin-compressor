// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pzip/crypto/winzip_aes_extra.h"

namespace pzip {

namespace {
constexpr size_t WINZIP_AES_EXTRA_TOTAL_SIZE = 11;
constexpr uint16_t WINZIP_AES_EXTRA_PAYLOAD_SIZE = 7;
constexpr uint16_t BYTE_MASK_U16 = 0x00FF;
constexpr int BYTE_SHIFT = 8;

inline uint8_t u16Lo(uint16_t v) { return static_cast<uint8_t>(v & BYTE_MASK_U16); }
inline uint8_t u16Hi(uint16_t v) { return static_cast<uint8_t>((v >> BYTE_SHIFT) & BYTE_MASK_U16); }
inline uint16_t le16At(const uint8_t* p) { return static_cast<uint16_t>(p[0] | (static_cast<uint16_t>(p[1]) << BYTE_SHIFT)); }

// WinZip AES extra (0x9901) layout (total 11 bytes):
// [0..1]  headerId (0x9901)
// [2..3]  dataSize (7)
// [4..5]  vendorVersion (AE-1/AE-2)
// [6..7]  vendorId ("AE" => 0x4541 little-endian)
// [8]     aesStrength (1/2/3)
// [9..10] actualCompressionMethod (Store/Deflate/...)
constexpr size_t WINZIP_AES_EXTRA_OFF_VENDOR_VERSION = 4;
constexpr size_t WINZIP_AES_EXTRA_OFF_VENDOR_ID = 6;
constexpr size_t WINZIP_AES_EXTRA_OFF_AES_STRENGTH = 8;
constexpr size_t WINZIP_AES_EXTRA_OFF_ACTUAL_METHOD = 9;
}

std::vector<uint8_t> WinZipAESExtra::encode() const {
    std::vector<uint8_t> data;
    data.reserve(WINZIP_AES_EXTRA_TOTAL_SIZE);

    data.push_back(u16Lo(WINZIP_AES_EXTRA_ID));
    data.push_back(u16Hi(WINZIP_AES_EXTRA_ID));

    data.push_back(u16Lo(WINZIP_AES_EXTRA_PAYLOAD_SIZE));
    data.push_back(u16Hi(WINZIP_AES_EXTRA_PAYLOAD_SIZE));

    data.push_back(u16Lo(vendorVersion));
    data.push_back(u16Hi(vendorVersion));

    data.push_back(u16Lo(vendorId));
    data.push_back(u16Hi(vendorId));

    data.push_back(aesStrength);

    data.push_back(u16Lo(actualCompressionMethod));
    data.push_back(u16Hi(actualCompressionMethod));

    return data;
}

WinZipAESExtra WinZipAESExtra::decode(const uint8_t* data, size_t size) {
    WinZipAESExtra extra;
    extra.vendorVersion = 0;
    extra.vendorId = WINZIP_AES_VENDOR_ID_AE;
    extra.aesStrength = 0;
    extra.actualCompressionMethod = ZIP_METHOD_STORE;

    // Full WinZip AES extra record is 11 bytes: 2 id + 2 size + 2 ver + 2 vendor + 1 strength + 2 method
    if (size >= WINZIP_AES_EXTRA_TOTAL_SIZE) {
        extra.vendorVersion = le16At(data + WINZIP_AES_EXTRA_OFF_VENDOR_VERSION);
        extra.vendorId = le16At(data + WINZIP_AES_EXTRA_OFF_VENDOR_ID);
        extra.aesStrength = data[WINZIP_AES_EXTRA_OFF_AES_STRENGTH];
        extra.actualCompressionMethod = le16At(data + WINZIP_AES_EXTRA_OFF_ACTUAL_METHOD);
    }

    return extra;
}

} // namespace pzip
