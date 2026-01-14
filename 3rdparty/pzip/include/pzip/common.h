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
    UNKNOWN_ERROR
};

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

