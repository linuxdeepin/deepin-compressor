// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common.h"
#include <filesystem>
#include <fstream>
#include <cstring>
#include <zlib.h>

namespace pzip {

namespace fs = std::filesystem;

/**
 * @brief ZIP 文件头信息
 */
// ZIP64 阈值常量 (对应 Go 的 uint32max)
constexpr uint32_t ZIP_UINT32_MAX = 0xFFFFFFFF;
constexpr uint16_t ZIP_UINT16_MAX = 0xFFFF;
constexpr uint16_t ZIP_VERSION_45 = 45;  // ZIP64 需要版本 4.5

struct ZipFileHeader {
    std::string name;           // 文件名（相对路径）
    uint16_t versionMadeBy = 0;
    uint16_t versionNeeded = ZIP_VERSION_20;
    uint16_t flags = 0;
    uint16_t method = ZIP_METHOD_DEFLATE;
    uint16_t modTime = 0;
    uint16_t modDate = 0;
    uint32_t crc32 = 0;
    uint64_t compressedSize = 0;
    uint64_t uncompressedSize = 0;
    uint32_t externalAttr = 0;   // Unix 权限等
    std::vector<uint8_t> extra; // 扩展字段
    
    bool isDirectory() const {
        return !name.empty() && name.back() == '/';
    }
    
    // 对应 Go 的 isZip64() 方法
    bool isZip64() const {
        return compressedSize >= ZIP_UINT32_MAX || uncompressedSize >= ZIP_UINT32_MAX;
    }
};

/**
 * @brief 文件任务类
 * 
 * 对应 Go 版 pzip 的 pool.File 结构
 * 管理文件的压缩数据缓冲区
 */
class FileTask {
public:
    FileTask();
    ~FileTask();
    
    // 禁止拷贝，允许移动
    FileTask(const FileTask&) = delete;
    FileTask& operator=(const FileTask&) = delete;
    FileTask(FileTask&&) noexcept;
    FileTask& operator=(FileTask&&) noexcept;

    /**
     * @brief 重置任务，准备处理新文件
     * @param path 文件完整路径
     * @param relativeTo 相对路径基准目录
     * @return 错误信息
     */
    Error reset(const fs::path& path, const fs::path& relativeTo = "");

    /**
     * @brief 写入压缩数据
     * @param data 数据指针
     * @param size 数据大小
     * @return 实际写入的字节数
     */
    size_t write(const uint8_t* data, size_t size);

    /**
     * @brief 获取已写入的总字节数
     */
    size_t written() const { return written_; }

    /**
     * @brief 是否有溢出数据（写入了临时文件）
     */
    bool overflowed() const { return overflow_ != nullptr; }

    /**
     * @brief 获取压缩数据缓冲区
     */
    const std::vector<uint8_t>& compressedData() const { return buffer_; }
    
    /**
     * @brief 获取压缩数据的指针和大小（包括溢出部分）
     * @param callback 回调函数，接收数据块
     */
    void readCompressedData(std::function<void(const uint8_t*, size_t)> callback);

    // 公共成员
    fs::path path;              // 文件完整路径
    fs::file_status status;     // 文件状态
    uintmax_t fileSize = 0;     // 原始文件大小
    ZipFileHeader header;       // ZIP 头信息
    
    // 压缩器（由 Archiver 管理）
    z_stream* compressor = nullptr;

private:
    std::vector<uint8_t> buffer_;           // 内存缓冲区
    size_t bufferUsed_ = 0;                 // 已使用的缓冲区大小
    std::unique_ptr<std::fstream> overflow_; // 溢出临时文件
    fs::path overflowPath_;                  // 临时文件路径
    size_t written_ = 0;                     // 总写入字节数
};

/**
 * @brief 文件任务对象池
 * 
 * 类似于 Go 的 sync.Pool，复用 FileTask 对象减少内存分配
 */
class FileTaskPool {
public:
    static FileTaskPool& instance();
    
    std::unique_ptr<FileTask> acquire();
    void release(std::unique_ptr<FileTask> task);

private:
    FileTaskPool() = default;
    
    std::mutex mutex_;
    std::vector<std::unique_ptr<FileTask>> pool_;
};

} // namespace pzip

