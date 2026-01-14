// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pzip/zip_writer.h"
#include "pzip/utils.h"
#include <cstring>
#include <ctime>

namespace pzip {

// ZIP 签名常量
constexpr uint32_t LOCAL_FILE_HEADER_SIG = 0x04034b50;
constexpr uint32_t DATA_DESCRIPTOR_SIG = 0x08074b50;
constexpr uint32_t CENTRAL_DIR_HEADER_SIG = 0x02014b50;
constexpr uint32_t END_OF_CENTRAL_DIR_SIG = 0x06054b50;
constexpr uint32_t ZIP64_END_OF_CENTRAL_DIR_SIG = 0x06064b50;
constexpr uint32_t ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIG = 0x07064b50;

// Extra field IDs
constexpr uint16_t EXTRA_ID_ZIP64 = 0x0001;
constexpr uint16_t EXTRA_ID_EXTENDED_TIMESTAMP = 0x5455;

// ============================================================================
// ExtendedTimestamp 实现
// ============================================================================

std::vector<uint8_t> ExtendedTimestamp::encode() const {
    std::vector<uint8_t> data;
    data.reserve(9);
    
    // Header ID
    data.push_back(EXTRA_ID_EXTENDED_TIMESTAMP & 0xFF);
    data.push_back((EXTRA_ID_EXTENDED_TIMESTAMP >> 8) & 0xFF);
    
    // Data size (1 byte flags + 4 bytes mtime)
    data.push_back(5);
    data.push_back(0);
    
    // Flags (bit 0 = mtime present)
    data.push_back(0x01);
    
    // Modification time (4 bytes, little endian)
    uint32_t t = static_cast<uint32_t>(modTime);
    data.push_back(t & 0xFF);
    data.push_back((t >> 8) & 0xFF);
    data.push_back((t >> 16) & 0xFF);
    data.push_back((t >> 24) & 0xFF);
    
    return data;
}

ExtendedTimestamp ExtendedTimestamp::decode(const uint8_t* data, size_t size) {
    ExtendedTimestamp ts;
    ts.modTime = 0;
    
    if (size >= 5 && (data[0] & 0x01)) {
        ts.modTime = data[1] | (data[2] << 8) | (data[3] << 16) | (data[4] << 24);
    }
    
    return ts;
}

// ============================================================================
// ZipWriter 实现
// ============================================================================

ZipWriter::ZipWriter(const fs::path& path) : path_(path) {}

ZipWriter::~ZipWriter() {
    if (file_.is_open()) {
        close();
    }
}

Error ZipWriter::open() {
    if (file_.is_open()) {
        return Error(ErrorCode::FILE_OPEN_ERROR, "File already open");
    }
    
    // 设置大缓冲区减少系统调用（必须在 open 之前）
    writeBuffer_.resize(WRITE_BUFFER_SIZE);
    file_.rdbuf()->pubsetbuf(writeBuffer_.data(), writeBuffer_.size());
    
    file_.open(path_, std::ios::binary | std::ios::out | std::ios::trunc);
    if (!file_.is_open()) {
        return Error(ErrorCode::FILE_OPEN_ERROR, "Cannot create file: " + path_.string());
    }
    
    currentOffset_ = 0;
    centralDir_.clear();
    
    return Error();
}

void ZipWriter::timeToDos(time_t t, uint16_t& date, uint16_t& dosTime) {
    struct tm* tm = localtime(&t);
    if (!tm) {
        date = 0;
        dosTime = 0;
        return;
    }
    
    // DOS time: bits 0-4 = seconds/2, bits 5-10 = minute, bits 11-15 = hour
    dosTime = ((tm->tm_hour & 0x1F) << 11) |
              ((tm->tm_min & 0x3F) << 5) |
              ((tm->tm_sec / 2) & 0x1F);
    
    // DOS date: bits 0-4 = day, bits 5-8 = month, bits 9-15 = year - 1980
    date = (((tm->tm_year - 80) & 0x7F) << 9) |
           (((tm->tm_mon + 1) & 0x0F) << 5) |
           (tm->tm_mday & 0x1F);
}

Error ZipWriter::writeLocalFileHeader(const ZipFileHeader& header) {
    std::vector<uint8_t> buf;
    buf.reserve(30 + header.name.size() + header.extra.size());
    
    auto write16 = [&buf](uint16_t v) {
        buf.push_back(v & 0xFF);
        buf.push_back((v >> 8) & 0xFF);
    };
    
    auto write32 = [&buf](uint32_t v) {
        buf.push_back(v & 0xFF);
        buf.push_back((v >> 8) & 0xFF);
        buf.push_back((v >> 16) & 0xFF);
        buf.push_back((v >> 24) & 0xFF);
    };
    
    // Signature
    write32(LOCAL_FILE_HEADER_SIG);
    
    // Version needed
    write16(header.versionNeeded);
    
    // Flags
    write16(header.flags);
    
    // Compression method
    write16(header.method);
    
    // Modification time and date
    write16(header.modTime);
    write16(header.modDate);
    
    // CRC32 (0 if using data descriptor)
    if (header.flags & ZIP_FLAG_DATA_DESCRIPTOR) {
        write32(0);
    } else {
        write32(header.crc32);
    }
    
    // Compressed size (0 if using data descriptor)
    if (header.flags & ZIP_FLAG_DATA_DESCRIPTOR) {
        write32(0);
    } else {
        write32(static_cast<uint32_t>(header.compressedSize));
    }
    
    // Uncompressed size (0 if using data descriptor)
    if (header.flags & ZIP_FLAG_DATA_DESCRIPTOR) {
        write32(0);
    } else {
        write32(static_cast<uint32_t>(header.uncompressedSize));
    }
    
    // Filename length
    write16(static_cast<uint16_t>(header.name.size()));
    
    // Extra field length
    write16(static_cast<uint16_t>(header.extra.size()));
    
    // Filename
    buf.insert(buf.end(), header.name.begin(), header.name.end());
    
    // Extra field
    buf.insert(buf.end(), header.extra.begin(), header.extra.end());
    
    file_.write(reinterpret_cast<const char*>(buf.data()), buf.size());
    
    if (!file_.good()) {
        return Error(ErrorCode::FILE_WRITE_ERROR, "Failed to write local file header");
    }
    
    currentOffset_ += buf.size();
    return Error();
}

Error ZipWriter::writeDataDescriptor(const ZipFileHeader& header) {
    std::vector<uint8_t> buf;
    buf.reserve(16);
    
    auto write32 = [&buf](uint32_t v) {
        buf.push_back(v & 0xFF);
        buf.push_back((v >> 8) & 0xFF);
        buf.push_back((v >> 16) & 0xFF);
        buf.push_back((v >> 24) & 0xFF);
    };
    
    // Signature (optional but recommended)
    write32(DATA_DESCRIPTOR_SIG);
    
    // CRC32
    write32(header.crc32);
    
    // Compressed size
    write32(static_cast<uint32_t>(header.compressedSize));
    
    // Uncompressed size
    write32(static_cast<uint32_t>(header.uncompressedSize));
    
    file_.write(reinterpret_cast<const char*>(buf.data()), buf.size());
    
    if (!file_.good()) {
        return Error(ErrorCode::FILE_WRITE_ERROR, "Failed to write data descriptor");
    }
    
    currentOffset_ += buf.size();
    return Error();
}

Error ZipWriter::createRaw(const ZipFileHeader& header,
                           std::function<void(std::function<void(const uint8_t*, size_t)>)> dataProvider) {
    std::lock_guard<std::mutex> lock(writeMutex_);
    
    if (!file_.is_open()) {
        return Error(ErrorCode::FILE_OPEN_ERROR, "File not open");
    }
    
    // 保存本地文件头偏移
    CentralDirEntry entry;
    entry.header = header;
    entry.localHeaderOffset = currentOffset_;
    
    // 写入本地文件头
    Error err = writeLocalFileHeader(header);
    if (err) return err;
    
    // 写入压缩数据
    dataProvider([this](const uint8_t* data, size_t size) {
        file_.write(reinterpret_cast<const char*>(data), size);
        currentOffset_ += size;
    });
    
    if (!file_.good()) {
        return Error(ErrorCode::FILE_WRITE_ERROR, "Failed to write compressed data");
    }
    
    // 如果使用数据描述符，写入它
    if (header.flags & ZIP_FLAG_DATA_DESCRIPTOR) {
        err = writeDataDescriptor(header);
        if (err) return err;
    }
    
    centralDir_.push_back(entry);
    return Error();
}

Error ZipWriter::create(const ZipFileHeader& header, const uint8_t* data, size_t size) {
    // 这个简化实现直接存储，实际应该压缩
    ZipFileHeader h = header;
    h.method = ZIP_METHOD_STORE;
    h.compressedSize = size;
    h.uncompressedSize = size;
    h.crc32 = utils::crc32(data, size);
    h.flags &= ~ZIP_FLAG_DATA_DESCRIPTOR;
    
    return createRaw(h, [data, size](std::function<void(const uint8_t*, size_t)> writer) {
        writer(data, size);
    });
}

Error ZipWriter::createDirectory(const ZipFileHeader& header) {
    ZipFileHeader h = header;
    
    // 确保名称以 / 结尾
    if (!h.name.empty() && h.name.back() != '/') {
        h.name += '/';
    }
    
    h.method = ZIP_METHOD_STORE;
    h.compressedSize = 0;
    h.uncompressedSize = 0;
    h.crc32 = 0;
    h.flags &= ~ZIP_FLAG_DATA_DESCRIPTOR;
    
    return createRaw(h, [](std::function<void(const uint8_t*, size_t)>) {
        // 目录没有数据
    });
}

Error ZipWriter::writeCentralDirectory() {
    auto write16 = [this](uint16_t v) {
        uint8_t buf[2] = {
            static_cast<uint8_t>(v & 0xFF),
            static_cast<uint8_t>((v >> 8) & 0xFF)
        };
        file_.write(reinterpret_cast<const char*>(buf), 2);
        currentOffset_ += 2;
    };
    
    auto write32 = [this](uint32_t v) {
        uint8_t buf[4] = {
            static_cast<uint8_t>(v & 0xFF),
            static_cast<uint8_t>((v >> 8) & 0xFF),
            static_cast<uint8_t>((v >> 16) & 0xFF),
            static_cast<uint8_t>((v >> 24) & 0xFF)
        };
        file_.write(reinterpret_cast<const char*>(buf), 4);
        currentOffset_ += 4;
    };
    
    for (const auto& entry : centralDir_) {
        const auto& h = entry.header;
        
        // Signature
        write32(CENTRAL_DIR_HEADER_SIG);
        
        // Version made by
        write16(h.versionMadeBy);
        
        // Version needed
        write16(h.versionNeeded);
        
        // Flags
        write16(h.flags);
        
        // Compression method
        write16(h.method);
        
        // Modification time and date
        write16(h.modTime);
        write16(h.modDate);
        
        // CRC32
        write32(h.crc32);
        
        // Compressed size
        write32(static_cast<uint32_t>(h.compressedSize));
        
        // Uncompressed size
        write32(static_cast<uint32_t>(h.uncompressedSize));
        
        // Filename length
        write16(static_cast<uint16_t>(h.name.size()));
        
        // Extra field length
        write16(static_cast<uint16_t>(h.extra.size()));
        
        // Comment length
        write16(0);
        
        // Disk number start
        write16(0);
        
        // Internal file attributes
        write16(0);
        
        // External file attributes
        write32(h.externalAttr);
        
        // Relative offset of local header
        write32(static_cast<uint32_t>(entry.localHeaderOffset));
        
        // Filename
        file_.write(h.name.c_str(), h.name.size());
        currentOffset_ += h.name.size();
        
        // Extra field
        if (!h.extra.empty()) {
            file_.write(reinterpret_cast<const char*>(h.extra.data()), h.extra.size());
            currentOffset_ += h.extra.size();
        }
    }
    
    if (!file_.good()) {
        return Error(ErrorCode::FILE_WRITE_ERROR, "Failed to write central directory");
    }
    
    return Error();
}

Error ZipWriter::writeEndOfCentralDirectory() {
    uint64_t centralDirStart = currentOffset_;
    
    // 先计算中央目录的开始偏移
    for (const auto& entry : centralDir_) {
        centralDirStart -= 46 + entry.header.name.size() + entry.header.extra.size();
    }
    centralDirStart = centralDir_.empty() ? currentOffset_ : 
                      centralDir_[0].localHeaderOffset;
    
    // 实际上我们需要记录中央目录开始位置
    // 这里简化处理，假设 centralDirStart 在写入中央目录前记录
    
    auto write16 = [this](uint16_t v) {
        uint8_t buf[2] = {
            static_cast<uint8_t>(v & 0xFF),
            static_cast<uint8_t>((v >> 8) & 0xFF)
        };
        file_.write(reinterpret_cast<const char*>(buf), 2);
    };
    
    auto write32 = [this](uint32_t v) {
        uint8_t buf[4] = {
            static_cast<uint8_t>(v & 0xFF),
            static_cast<uint8_t>((v >> 8) & 0xFF),
            static_cast<uint8_t>((v >> 16) & 0xFF),
            static_cast<uint8_t>((v >> 24) & 0xFF)
        };
        file_.write(reinterpret_cast<const char*>(buf), 4);
    };
    
    // 计算中央目录大小
    uint64_t centralDirSize = 0;
    for (const auto& entry : centralDir_) {
        centralDirSize += 46 + entry.header.name.size() + entry.header.extra.size();
    }
    
    // Signature
    write32(END_OF_CENTRAL_DIR_SIG);
    
    // Number of this disk
    write16(0);
    
    // Disk where central directory starts
    write16(0);
    
    // Number of central directory records on this disk
    write16(static_cast<uint16_t>(centralDir_.size()));
    
    // Total number of central directory records
    write16(static_cast<uint16_t>(centralDir_.size()));
    
    // Size of central directory
    write32(static_cast<uint32_t>(centralDirSize));
    
    // Offset of start of central directory
    write32(static_cast<uint32_t>(currentOffset_ - centralDirSize));
    
    // Comment length
    write16(static_cast<uint16_t>(comment_.size()));
    
    // Comment
    if (!comment_.empty()) {
        file_.write(comment_.c_str(), comment_.size());
    }
    
    if (!file_.good()) {
        return Error(ErrorCode::FILE_WRITE_ERROR, "Failed to write end of central directory");
    }
    
    return Error();
}

Error ZipWriter::close() {
    if (!file_.is_open()) {
        return Error();
    }
    
    // 记录中央目录开始位置
    uint64_t centralDirOffset = currentOffset_;
    
    // 写入中央目录
    Error err = writeCentralDirectory();
    if (err) return err;
    
    // 写入结束记录
    err = writeEndOfCentralDirectory();
    if (err) return err;
    
    file_.close();
    return Error();
}

} // namespace pzip


