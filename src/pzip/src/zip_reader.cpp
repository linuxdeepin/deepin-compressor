// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pzip/zip_reader.h"
#include "pzip/utils.h"
#include <cstring>
#include <zlib.h>
#include <sys/stat.h>

namespace pzip {

// ZIP 签名常量
constexpr uint32_t LOCAL_FILE_HEADER_SIG = 0x04034b50;
constexpr uint32_t CENTRAL_DIR_HEADER_SIG = 0x02014b50;
constexpr uint32_t END_OF_CENTRAL_DIR_SIG = 0x06054b50;

// ============================================================================
// ZipReader 实现
// ============================================================================

ZipReader::ZipReader(const fs::path& path) : path_(path) {}

ZipReader::~ZipReader() {
    close();
}

time_t ZipReader::dosToTime(uint16_t date, uint16_t time) {
    struct tm tm = {};
    
    tm.tm_sec = (time & 0x1F) * 2;
    tm.tm_min = (time >> 5) & 0x3F;
    tm.tm_hour = (time >> 11) & 0x1F;
    tm.tm_mday = date & 0x1F;
    tm.tm_mon = ((date >> 5) & 0x0F) - 1;
    tm.tm_year = ((date >> 9) & 0x7F) + 80;
    
    return mktime(&tm);
}

Error ZipReader::open() {
    if (file_.is_open()) {
        return Error(ErrorCode::FILE_OPEN_ERROR, "File already open");
    }
    
    file_.open(path_, std::ios::binary | std::ios::in);
    if (!file_.is_open()) {
        return Error(ErrorCode::FILE_OPEN_ERROR, "Cannot open file: " + path_.string());
    }
    
    // 读取结束记录以获取中央目录位置
    Error err = readEndOfCentralDirectory();
    if (err) {
        file_.close();
        return err;
    }
    
    // 读取中央目录
    err = readCentralDirectory();
    if (err) {
        file_.close();
        return err;
    }
    
    return Error();
}

void ZipReader::close() {
    if (file_.is_open()) {
        file_.close();
    }
    entries_.clear();
}

Error ZipReader::readEndOfCentralDirectory() {
    // 从文件末尾向前搜索结束签名
    file_.seekg(0, std::ios::end);
    auto fileSize = file_.tellg();
    
    // 最大搜索范围（包括可能的注释）
    const size_t maxSearch = std::min(static_cast<size_t>(fileSize), size_t(65536 + 22));
    
    std::vector<uint8_t> buf(maxSearch);
    file_.seekg(-static_cast<std::streamoff>(maxSearch), std::ios::end);
    file_.read(reinterpret_cast<char*>(buf.data()), maxSearch);
    
    // 从后向前搜索签名
    int64_t sigOffset = -1;
    for (int64_t i = maxSearch - 22; i >= 0; --i) {
        if (buf[i] == 0x50 && buf[i+1] == 0x4b && 
            buf[i+2] == 0x05 && buf[i+3] == 0x06) {
            sigOffset = i;
            break;
        }
    }
    
    if (sigOffset < 0) {
        return Error(ErrorCode::INVALID_ARCHIVE, "Cannot find end of central directory");
    }
    
    // 解析结束记录
    const uint8_t* p = buf.data() + sigOffset + 4;
    
    auto read16 = [&p]() -> uint16_t {
        uint16_t v = p[0] | (p[1] << 8);
        p += 2;
        return v;
    };
    
    auto read32 = [&p]() -> uint32_t {
        uint32_t v = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
        p += 4;
        return v;
    };
    
    /* diskNumber = */ read16();
    /* diskWithCD = */ read16();
    /* entriesOnDisk = */ read16();
    totalEntries_ = read16();
    centralDirSize_ = read32();
    centralDirOffset_ = read32();
    uint16_t commentLen = read16();
    
    if (commentLen > 0 && sigOffset + 22 + commentLen <= maxSearch) {
        comment_.assign(reinterpret_cast<const char*>(p), commentLen);
    }
    
    return Error();
}

Error ZipReader::readCentralDirectory() {
    entries_.clear();
    entries_.reserve(totalEntries_);
    
    file_.seekg(centralDirOffset_, std::ios::beg);
    
    for (uint32_t i = 0; i < totalEntries_; ++i) {
        // 读取中央目录头
        uint8_t header[46];
        file_.read(reinterpret_cast<char*>(header), 46);
        
        if (!file_.good()) {
            return Error(ErrorCode::FILE_READ_ERROR, "Failed to read central directory");
        }
        
        // 验证签名
        uint32_t sig = header[0] | (header[1] << 8) | (header[2] << 16) | (header[3] << 24);
        if (sig != CENTRAL_DIR_HEADER_SIG) {
            return Error(ErrorCode::INVALID_ARCHIVE, "Invalid central directory signature");
        }
        
        ZipEntry entry;
        auto& h = entry.header;
        
        h.versionMadeBy = header[4] | (header[5] << 8);
        h.versionNeeded = header[6] | (header[7] << 8);
        h.flags = header[8] | (header[9] << 8);
        h.method = header[10] | (header[11] << 8);
        h.modTime = header[12] | (header[13] << 8);
        h.modDate = header[14] | (header[15] << 8);
        h.crc32 = header[16] | (header[17] << 8) | (header[18] << 16) | (header[19] << 24);
        h.compressedSize = header[20] | (header[21] << 8) | (header[22] << 16) | (header[23] << 24);
        h.uncompressedSize = header[24] | (header[25] << 8) | (header[26] << 16) | (header[27] << 24);
        
        uint16_t nameLen = header[28] | (header[29] << 8);
        uint16_t extraLen = header[30] | (header[31] << 8);
        uint16_t commentLen = header[32] | (header[33] << 8);
        
        h.externalAttr = header[38] | (header[39] << 8) | (header[40] << 16) | (header[41] << 24);
        entry.localHeaderOffset = header[42] | (header[43] << 8) | (header[44] << 16) | (header[45] << 24);
        
        // 读取文件名
        if (nameLen > 0) {
            std::vector<char> nameBuf(nameLen);
            file_.read(nameBuf.data(), nameLen);
            h.name.assign(nameBuf.data(), nameLen);
        }
        
        // 读取扩展字段
        if (extraLen > 0) {
            h.extra.resize(extraLen);
            file_.read(reinterpret_cast<char*>(h.extra.data()), extraLen);
        }
        
        // 跳过注释
        if (commentLen > 0) {
            file_.seekg(commentLen, std::ios::cur);
        }
        
        entries_.push_back(entry);
    }
    
    // 读取每个条目的本地文件头以获取数据偏移
    for (auto& entry : entries_) {
        Error err = readLocalFileHeader(entry);
        if (err) return err;
    }
    
    return Error();
}

Error ZipReader::readLocalFileHeader(ZipEntry& entry) {
    file_.seekg(entry.localHeaderOffset, std::ios::beg);
    
    uint8_t header[30];
    file_.read(reinterpret_cast<char*>(header), 30);
    
    if (!file_.good()) {
        return Error(ErrorCode::FILE_READ_ERROR, "Failed to read local file header");
    }
    
    // 验证签名
    uint32_t sig = header[0] | (header[1] << 8) | (header[2] << 16) | (header[3] << 24);
    if (sig != LOCAL_FILE_HEADER_SIG) {
        return Error(ErrorCode::INVALID_ARCHIVE, "Invalid local file header signature");
    }
    
    uint16_t nameLen = header[26] | (header[27] << 8);
    uint16_t extraLen = header[28] | (header[29] << 8);
    
    // 数据偏移 = 本地头偏移 + 30 + 文件名长度 + 扩展字段长度
    entry.dataOffset = entry.localHeaderOffset + 30 + nameLen + extraLen;
    
    return Error();
}

Error ZipReader::readCompressed(const ZipEntry& entry, std::vector<uint8_t>& buffer) {
    buffer.resize(entry.header.compressedSize);
    
    file_.seekg(entry.dataOffset, std::ios::beg);
    file_.read(reinterpret_cast<char*>(buffer.data()), entry.header.compressedSize);
    
    if (!file_.good()) {
        return Error(ErrorCode::FILE_READ_ERROR, "Failed to read compressed data");
    }
    
    return Error();
}

Error ZipReader::readDecompressed(const ZipEntry& entry, std::vector<uint8_t>& buffer) {
    if (entry.header.method == ZIP_METHOD_STORE) {
        // 存储方式，直接读取
        return readCompressed(entry, buffer);
    }
    
    if (entry.header.method != ZIP_METHOD_DEFLATE) {
        return Error(ErrorCode::DECOMPRESSION_ERROR, 
                     "Unsupported compression method: " + std::to_string(entry.header.method));
    }
    
    // 读取压缩数据
    std::vector<uint8_t> compressed;
    Error err = readCompressed(entry, compressed);
    if (err) return err;
    
    // 解压
    buffer.resize(entry.header.uncompressedSize);
    
    z_stream strm = {};
    strm.next_in = compressed.data();
    strm.avail_in = compressed.size();
    strm.next_out = buffer.data();
    strm.avail_out = buffer.size();
    
    // -MAX_WBITS 表示 raw deflate（无 zlib 头）
    if (inflateInit2(&strm, -MAX_WBITS) != Z_OK) {
        return Error(ErrorCode::DECOMPRESSION_ERROR, "Failed to initialize decompressor");
    }
    
    int ret = inflate(&strm, Z_FINISH);
    inflateEnd(&strm);
    
    if (ret != Z_STREAM_END) {
        return Error(ErrorCode::DECOMPRESSION_ERROR, "Decompression failed");
    }
    
    // 验证 CRC32
    uint32_t crc = utils::crc32(buffer.data(), buffer.size());
    if (crc != entry.header.crc32) {
        return Error(ErrorCode::DECOMPRESSION_ERROR, "CRC32 mismatch");
    }
    
    return Error();
}

Error ZipReader::extractTo(const ZipEntry& entry, const fs::path& outputPath) {
    std::error_code ec;
    
    if (entry.isDirectory()) {
        fs::create_directories(outputPath, ec);
        if (ec) {
            return Error(ErrorCode::FILE_WRITE_ERROR, "Cannot create directory: " + outputPath.string());
        }
        return Error();
    }
    
    // 确保父目录存在
    fs::create_directories(outputPath.parent_path(), ec);
    
    // 解压数据
    std::vector<uint8_t> data;
    Error err = readDecompressed(entry, data);
    if (err) return err;
    
    // 写入文件
    std::ofstream outFile(outputPath, std::ios::binary | std::ios::trunc);
    if (!outFile.is_open()) {
        return Error(ErrorCode::FILE_WRITE_ERROR, "Cannot create file: " + outputPath.string());
    }
    
    outFile.write(reinterpret_cast<const char*>(data.data()), data.size());
    outFile.close();
    
    // 设置权限
    mode_t mode = utils::zipAttrToMode(entry.header.externalAttr);
    if (mode != 0) {
        chmod(outputPath.c_str(), mode);
    }
    
    // 设置修改时间
    time_t modTime = dosToTime(entry.header.modDate, entry.header.modTime);
    utils::setModTime(outputPath, modTime);
    
    return Error();
}

} // namespace pzip

