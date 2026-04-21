// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <mutex>
#include <condition_variable>

namespace pzip {

// 常量定义
constexpr size_t DEFAULT_BUFFER_SIZE = 2 * 1024 * 1024;  // 2MB 缓冲区
constexpr size_t READ_BUFFER_SIZE = 32 * 1024;           // 32KB 读取缓冲
constexpr int DEFAULT_COMPRESSION_LEVEL = -1;            // zlib 默认压缩级别
constexpr uint16_t ZIP_VERSION_20 = 20;

// ZIP 压缩方法
constexpr uint16_t ZIP_METHOD_STORE = 0;
constexpr uint16_t ZIP_METHOD_DEFLATE = 8;

// ZIP 标志位
constexpr uint16_t ZIP_FLAG_DATA_DESCRIPTOR = 0x0008;
constexpr uint16_t ZIP_FLAG_UTF8 = 0x0800;
constexpr uint16_t ZIP_FLAG_ENCRYPTED = 0x0001;

// ZIP 加密方法
constexpr uint16_t ZIP_ENCRYPTION_NONE = 0;
constexpr uint16_t ZIP_ENCRYPTION_WINZIP_AES = 99;  // WinZip AES 标识

// WinZip AES 参数
constexpr uint16_t WINZIP_AES_EXTRA_ID = 0x9901;    // WinZip AES Extra Field ID
constexpr uint16_t WINZIP_AES_VERSION_1 = 0x0001;   // AE-1 
constexpr uint16_t WINZIP_AES_VERSION_2 = 0x0002;   // AE-2 
constexpr size_t WINZIP_AES_AUTH_CODE_SIZE = 10;    // HMAC-SHA1 认证码大小
constexpr size_t WINZIP_AES_PV_SIZE = 2;            // 密码验证值大小
constexpr uint32_t PBKDF2_ITERATIONS = 1000;        // PBKDF2 迭代次数

// 错误码
enum class ErrorCode {
    OK = 0,
    FILE_NOT_FOUND,
    FILE_OPEN_ERROR,
    FILE_READ_ERROR,
    FILE_WRITE_ERROR,
    COMPRESSION_ERROR,
    DECOMPRESSION_ERROR,
    INVALID_ARCHIVE,
    MEMORY_ERROR,
    CANCELLED,
    UNKNOWN_ERROR,

    // 加密相关错误码
    ENCRYPTION_NOT_SUPPORTED,       // 不支持的加密方法
    ENCRYPTION_KEY_DERIVATION_ERROR,// 密钥派生失败
    ENCRYPTION_ERROR,               // 加密失败
    DECRYPTION_ERROR,               // 解密失败
    WRONG_PASSWORD,                 // 密码错误
    MISSING_PASSWORD,               // 缺少密码
    AUTHENTICATION_FAILED           // 认证码校验失败
};

// 加密方法枚举
enum class EncryptionMethod {
    None = 0,
    AES128 = 1,
    AES192 = 2,
    AES256 = 3
};

constexpr size_t AES128_KEY_SIZE = 16;
constexpr size_t AES192_KEY_SIZE = 24;
constexpr size_t AES256_KEY_SIZE = 32;

constexpr size_t AES128_SALT_SIZE = 8;
constexpr size_t AES192_SALT_SIZE = 12;
constexpr size_t AES256_SALT_SIZE = 16;

// 获取加密方法对应的密钥大小
inline size_t encryptionKeySize(EncryptionMethod method) {
    switch (method) {
        case EncryptionMethod::AES128: return AES128_KEY_SIZE;
        case EncryptionMethod::AES192: return AES192_KEY_SIZE;
        case EncryptionMethod::AES256: return AES256_KEY_SIZE;
        default: return 0;
    }
}

// 获取加密方法对应的盐值大小
inline size_t encryptionSaltSize(EncryptionMethod method) {
    switch (method) {
        case EncryptionMethod::AES128: return AES128_SALT_SIZE;
        case EncryptionMethod::AES192: return AES192_SALT_SIZE;
        case EncryptionMethod::AES256: return AES256_SALT_SIZE;
        default: return 0;
    }
}

// 获取加密方法对应的 AES 强度标识
inline uint8_t encryptionStrength(EncryptionMethod method) {
    return static_cast<uint8_t>(method);
}

// 错误信息
struct Error {
    ErrorCode code;
    std::string message;
    
    Error() : code(ErrorCode::OK) {}
    Error(ErrorCode c, const std::string& msg = "") : code(c), message(msg) {}
    
    explicit operator bool() const { return code != ErrorCode::OK; }
};

// 进度回调
using ProgressCallback = std::function<void(size_t current, size_t total)>;

// 前向声明
class FileTask;
template<typename T> class WorkerPool;
class Archiver;
class Extractor;
class ZipWriter;
class ZipReader;
struct ExtractTask;

} // namespace pzip

