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
    // 对应 Go archive/zip 的 CreateRaw/CreateHeader 写入本地文件头的逻辑
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
    
    // Version needed (ZIP64 需要版本 4.5)
    // 对应 Go: if fh.isZip64() { fh.ReaderVersion = zipVersion45 }
    write16(header.isZip64() ? ZIP_VERSION_45 : header.versionNeeded);
    
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
    } else if (header.isZip64()) {
        write32(ZIP_UINT32_MAX);
    } else {
        write32(static_cast<uint32_t>(header.compressedSize));
    }
    
    // Uncompressed size (0 if using data descriptor)
    if (header.flags & ZIP_FLAG_DATA_DESCRIPTOR) {
        write32(0);
    } else if (header.isZip64()) {
        write32(ZIP_UINT32_MAX);
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
    // 对应 Go archive/zip 的 writeDataDescriptor
    // 如果 isZip64()，使用 24 字节（4+4+8+8），否则使用 16 字节（4+4+4+4）
    std::vector<uint8_t> buf;
    
    auto write32 = [&buf](uint32_t v) {
        buf.push_back(v & 0xFF);
        buf.push_back((v >> 8) & 0xFF);
        buf.push_back((v >> 16) & 0xFF);
        buf.push_back((v >> 24) & 0xFF);
    };
    
    auto write64 = [&buf](uint64_t v) {
        buf.push_back(v & 0xFF);
        buf.push_back((v >> 8) & 0xFF);
        buf.push_back((v >> 16) & 0xFF);
        buf.push_back((v >> 24) & 0xFF);
        buf.push_back((v >> 32) & 0xFF);
        buf.push_back((v >> 40) & 0xFF);
        buf.push_back((v >> 48) & 0xFF);
        buf.push_back((v >> 56) & 0xFF);
    };
    
    if (header.isZip64()) {
        buf.reserve(24);  // dataDescriptor64Len
    } else {
        buf.reserve(16);  // dataDescriptorLen
    }
    
    // Signature (de-facto standard, required by OS X)
    write32(DATA_DESCRIPTOR_SIG);
    
    // CRC32
    write32(header.crc32);
    
    if (header.isZip64()) {
        // 64 位大小
        write64(header.compressedSize);
        write64(header.uncompressedSize);
    } else {
        // 32 位大小
        write32(static_cast<uint32_t>(header.compressedSize));
        write32(static_cast<uint32_t>(header.uncompressedSize));
    }
    
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
    // 对应 Go archive/zip 的 Close() 中写入中央目录的逻辑
    std::vector<uint8_t> buf;
    buf.reserve(46);  // directoryHeaderLen
    
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
    
    auto write64 = [&buf](uint64_t v) {
        buf.push_back(v & 0xFF);
        buf.push_back((v >> 8) & 0xFF);
        buf.push_back((v >> 16) & 0xFF);
        buf.push_back((v >> 24) & 0xFF);
        buf.push_back((v >> 32) & 0xFF);
        buf.push_back((v >> 40) & 0xFF);
        buf.push_back((v >> 48) & 0xFF);
        buf.push_back((v >> 56) & 0xFF);
    };
    
    for (auto& entry : centralDir_) {
        auto& h = entry.header;
        buf.clear();
        
        // 检查是否需要 ZIP64 extra 字段
        bool needZip64 = h.isZip64() || entry.localHeaderOffset >= ZIP_UINT32_MAX;
        
        // Signature
        write32(CENTRAL_DIR_HEADER_SIG);
        
        // Version made by
        write16(h.versionMadeBy);
        
        // Version needed (ZIP64 需要版本 4.5)
        write16(needZip64 ? ZIP_VERSION_45 : h.versionNeeded);
        
        // Flags
        write16(h.flags);
        
        // Compression method
        write16(h.method);
        
        // Modification time and date
        write16(h.modTime);
        write16(h.modDate);
        
        // CRC32
        write32(h.crc32);
        
        if (needZip64) {
            // 对应 Go: "the file needs a zip64 header. store maxint in both
            // 32 bit size fields (and offset later) to signal that the
            // zip64 extra header should be used."
            write32(ZIP_UINT32_MAX);  // compressed size
            write32(ZIP_UINT32_MAX);  // uncompressed size
        } else {
            write32(static_cast<uint32_t>(h.compressedSize));
            write32(static_cast<uint32_t>(h.uncompressedSize));
        }
        
        // Filename length
        write16(static_cast<uint16_t>(h.name.size()));
        
        // 构建 ZIP64 extra 字段
        std::vector<uint8_t> zip64Extra;
        if (needZip64) {
            // 28 bytes: 2x uint16 + 3x uint64
            // Header ID
            zip64Extra.push_back(EXTRA_ID_ZIP64 & 0xFF);
            zip64Extra.push_back((EXTRA_ID_ZIP64 >> 8) & 0xFF);
            // Data size = 24 (3x uint64)
            zip64Extra.push_back(24);
            zip64Extra.push_back(0);
            // Uncompressed size
            for (int i = 0; i < 8; i++) {
                zip64Extra.push_back((h.uncompressedSize >> (i * 8)) & 0xFF);
            }
            // Compressed size
            for (int i = 0; i < 8; i++) {
                zip64Extra.push_back((h.compressedSize >> (i * 8)) & 0xFF);
            }
            // Local header offset
            for (int i = 0; i < 8; i++) {
                zip64Extra.push_back((entry.localHeaderOffset >> (i * 8)) & 0xFF);
            }
        }
        
        // Extra field length (原有 extra + ZIP64 extra)
        size_t extraLen = h.extra.size() + zip64Extra.size();
        write16(static_cast<uint16_t>(extraLen));
        
        // Comment length
        write16(0);
        
        // Disk number start
        write16(0);
        
        // Internal file attributes
        write16(0);
        
        // External file attributes
        write32(h.externalAttr);
        
        // Relative offset of local header
        if (entry.localHeaderOffset > ZIP_UINT32_MAX) {
            write32(ZIP_UINT32_MAX);
        } else {
            write32(static_cast<uint32_t>(entry.localHeaderOffset));
        }
        
        // 写入固定部分
        file_.write(reinterpret_cast<const char*>(buf.data()), buf.size());
        currentOffset_ += buf.size();
        
        // Filename
        file_.write(h.name.c_str(), h.name.size());
        currentOffset_ += h.name.size();
        
        // 原有 Extra field
        if (!h.extra.empty()) {
            file_.write(reinterpret_cast<const char*>(h.extra.data()), h.extra.size());
            currentOffset_ += h.extra.size();
        }
        
        // ZIP64 extra field
        if (!zip64Extra.empty()) {
            file_.write(reinterpret_cast<const char*>(zip64Extra.data()), zip64Extra.size());
            currentOffset_ += zip64Extra.size();
        }
    }
    
    if (!file_.good()) {
        return Error(ErrorCode::FILE_WRITE_ERROR, "Failed to write central directory");
    }
    
    return Error();
}

Error ZipWriter::writeEndOfCentralDirectory(uint64_t centralDirOffset, uint64_t centralDirSize) {
    // 对应 Go archive/zip 的 Close() 中写入 EOCD 的逻辑
    
    uint64_t records = centralDir_.size();
    uint64_t size = centralDirSize;
    uint64_t offset = centralDirOffset;
    
    std::vector<uint8_t> buf;
    
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
    
    auto write64 = [&buf](uint64_t v) {
        buf.push_back(v & 0xFF);
        buf.push_back((v >> 8) & 0xFF);
        buf.push_back((v >> 16) & 0xFF);
        buf.push_back((v >> 24) & 0xFF);
        buf.push_back((v >> 32) & 0xFF);
        buf.push_back((v >> 40) & 0xFF);
        buf.push_back((v >> 48) & 0xFF);
        buf.push_back((v >> 56) & 0xFF);
    };
    
    // 检查是否需要 ZIP64 EOCD
    // 对应 Go: if records >= uint16max || size >= uint32max || offset >= uint32max
    bool needZip64 = (records >= ZIP_UINT16_MAX || 
                      size >= ZIP_UINT32_MAX || 
                      offset >= ZIP_UINT32_MAX);
    
    if (needZip64) {
        // 写入 ZIP64 End of Central Directory Record (56 bytes)
        // directory64EndLen = 56
        buf.clear();
        buf.reserve(56 + 20);  // directory64EndLen + directory64LocLen
        
        // ZIP64 EOCD signature
        write32(ZIP64_END_OF_CENTRAL_DIR_SIG);
        
        // Size of ZIP64 EOCD record (excluding signature and this field)
        // = 56 - 12 = 44
        write64(44);
        
        // Version made by
        write16(ZIP_VERSION_45);
        
        // Version needed to extract
        write16(ZIP_VERSION_45);
        
        // Number of this disk
        write32(0);
        
        // Number of the disk with the start of the central directory
        write32(0);
        
        // Total number of entries in the central directory on this disk
        write64(records);
        
        // Total number of entries in the central directory
        write64(records);
        
        // Size of the central directory
        write64(size);
        
        // Offset of start of central directory
        write64(offset);
        
        // 写入 ZIP64 End of Central Directory Locator (20 bytes)
        // directory64LocLen = 20
        
        // ZIP64 EOCD Locator signature
        write32(ZIP64_END_OF_CENTRAL_DIR_LOCATOR_SIG);
        
        // Number of the disk with the start of the zip64 end of central directory
        write32(0);
        
        // Relative offset of the zip64 end of central directory record
        // (currentOffset_ 已经指向 central directory 结束位置)
        write64(currentOffset_);
        
        // Total number of disks
        write32(1);
        
        file_.write(reinterpret_cast<const char*>(buf.data()), buf.size());
        currentOffset_ += buf.size();
        
        // 在普通 EOCD 中使用 max 值表示应使用 ZIP64 值
        records = ZIP_UINT16_MAX;
        size = ZIP_UINT32_MAX;
        offset = ZIP_UINT32_MAX;
    }
    
    // 写入普通 End of Central Directory Record (22 bytes)
    // directoryEndLen = 22
    buf.clear();
    buf.reserve(22 + comment_.size());
    
    // Signature
    write32(END_OF_CENTRAL_DIR_SIG);
    
    // Number of this disk
    write16(0);
    
    // Disk where central directory starts
    write16(0);
    
    // Number of central directory records on this disk
    write16(static_cast<uint16_t>(records));
    
    // Total number of central directory records
    write16(static_cast<uint16_t>(records));
    
    // Size of central directory
    write32(static_cast<uint32_t>(size));
    
    // Offset of start of central directory
    write32(static_cast<uint32_t>(offset));
    
    // Comment length
    write16(static_cast<uint16_t>(comment_.size()));
    
    file_.write(reinterpret_cast<const char*>(buf.data()), buf.size());
    
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
    
    // 记录中央目录开始位置 (对应 Go 的 start := w.cw.count)
    uint64_t centralDirOffset = currentOffset_;
    
    // 写入中央目录
    Error err = writeCentralDirectory();
    if (err) return err;
    
    // 计算中央目录大小 (对应 Go 的 size := uint64(end - start))
    uint64_t centralDirSize = currentOffset_ - centralDirOffset;
    
    // 写入结束记录
    err = writeEndOfCentralDirectory(centralDirOffset, centralDirSize);
    if (err) return err;
    
    file_.close();
    return Error();
}

} // namespace pzip


