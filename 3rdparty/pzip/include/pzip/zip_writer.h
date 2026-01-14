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
 * @brief Extended Timestamp Extra Field (0x5455)
 * 
 * 用于存储 UTC 时间戳
 */
struct ExtendedTimestamp {
    time_t modTime;
    
    std::vector<uint8_t> encode() const;
    static ExtendedTimestamp decode(const uint8_t* data, size_t size);
};

/**
 * @brief ZIP 文件写入器
 * 
 * 支持 CreateRaw 方式写入预压缩数据
 * 类似于 Go archive/zip 的 Writer
 */
class ZipWriter {
public:
    explicit ZipWriter(const fs::path& path);
    ~ZipWriter();

    // 禁止拷贝
    ZipWriter(const ZipWriter&) = delete;
    ZipWriter& operator=(const ZipWriter&) = delete;

    /**
     * @brief 打开文件准备写入
     * @return 错误信息
     */
    Error open();

    /**
     * @brief 写入预压缩的数据（CreateRaw 风格）
     * @param header ZIP 文件头
     * @param compressedData 压缩后的数据
     * @param compressedSize 压缩数据大小
     * @return 错误信息
     */
    Error createRaw(const ZipFileHeader& header, 
                    std::function<void(std::function<void(const uint8_t*, size_t)>)> dataProvider);

    /**
     * @brief 写入文件（会自动压缩）
     * @param header ZIP 文件头
     * @param data 原始数据
     * @param size 数据大小
     * @return 错误信息
     */
    Error create(const ZipFileHeader& header, const uint8_t* data, size_t size);

    /**
     * @brief 写入目录条目
     * @param header ZIP 文件头（name 应以 / 结尾）
     * @return 错误信息
     */
    Error createDirectory(const ZipFileHeader& header);

    /**
     * @brief 关闭并写入中央目录
     * @return 错误信息
     */
    Error close();

    /**
     * @brief 是否已打开
     */
    bool isOpen() const { return file_.is_open(); }

    /**
     * @brief 设置注释
     */
    void setComment(const std::string& comment) { comment_ = comment; }

private:
    // 中央目录条目
    struct CentralDirEntry {
        ZipFileHeader header;
        uint64_t localHeaderOffset;
    };

    Error writeLocalFileHeader(const ZipFileHeader& header);
    Error writeDataDescriptor(const ZipFileHeader& header);
    Error writeCentralDirectory();
    Error writeEndOfCentralDirectory();

    // DOS 时间转换
    static void timeToDos(time_t t, uint16_t& date, uint16_t& time);

    fs::path path_;
    std::ofstream file_;
    std::vector<CentralDirEntry> centralDir_;
    std::string comment_;
    uint64_t currentOffset_ = 0;
    std::mutex writeMutex_;
    
    // 大缓冲区减少系统调用
    static constexpr size_t WRITE_BUFFER_SIZE = 256 * 1024;
    std::vector<char> writeBuffer_;
};

} // namespace pzip


