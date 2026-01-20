// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common.h"
#include "file_task.h"
#include <fstream>

namespace pzip {

/**
 * @brief ZIP 文件条目
 */
struct ZipEntry {
    ZipFileHeader header;
    uint64_t dataOffset;        // 压缩数据在文件中的偏移
    uint64_t localHeaderOffset; // 本地文件头偏移
    
    bool isDirectory() const { return header.isDirectory(); }
};

/**
 * @brief ZIP 文件读取器
 * 
 * 类似于 Go archive/zip 的 Reader
 */
class ZipReader {
public:
    explicit ZipReader(const fs::path& path);
    ~ZipReader();

    // 禁止拷贝
    ZipReader(const ZipReader&) = delete;
    ZipReader& operator=(const ZipReader&) = delete;

    /**
     * @brief 打开并读取 ZIP 文件目录
     * @return 错误信息
     */
    Error open();

    /**
     * @brief 关闭文件
     */
    void close();

    /**
     * @brief 获取所有条目
     */
    const std::vector<ZipEntry>& entries() const { return entries_; }

    /**
     * @brief 读取条目的压缩数据
     * @param entry 条目
     * @param buffer 输出缓冲区
     * @return 错误信息
     */
    Error readCompressed(const ZipEntry& entry, std::vector<uint8_t>& buffer);

    /**
     * @brief 解压并读取条目数据
     * @param entry 条目
     * @param buffer 输出缓冲区
     * @return 错误信息
     */
    Error readDecompressed(const ZipEntry& entry, std::vector<uint8_t>& buffer);

    /**
     * @brief 解压条目到文件
     * @param entry 条目
     * @param outputPath 输出路径
     * @return 错误信息
     */
    Error extractTo(const ZipEntry& entry, const fs::path& outputPath);

    /**
     * @brief 获取注释
     */
    const std::string& comment() const { return comment_; }

    /**
     * @brief 是否已打开
     */
    bool isOpen() const { return file_.is_open(); }

private:
    Error readEndOfCentralDirectory();
    Error readCentralDirectory();
    Error readLocalFileHeader(ZipEntry& entry);
    
    // DOS 时间转换
    static time_t dosToTime(uint16_t date, uint16_t time);

    fs::path path_;
    std::ifstream file_;
    std::vector<ZipEntry> entries_;
    std::string comment_;
    
    uint64_t centralDirOffset_ = 0;
    uint64_t centralDirSize_ = 0;
    uint32_t totalEntries_ = 0;
};

} // namespace pzip


