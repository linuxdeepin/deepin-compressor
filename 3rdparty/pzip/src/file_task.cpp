// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pzip/file_task.h"
#include "pzip/utils.h"
#include <cstring>
#include <sys/stat.h>

namespace pzip {

// ============================================================================
// FileTask 实现
// ============================================================================

FileTask::FileTask() {
    buffer_.resize(DEFAULT_BUFFER_SIZE);
}

FileTask::~FileTask() {
    // 清理临时文件
    if (overflow_) {
        overflow_->close();
        if (!overflowPath_.empty() && fs::exists(overflowPath_)) {
            std::error_code ec;
            fs::remove(overflowPath_, ec);
        }
    }
    
    // 清理压缩器（由外部管理，这里不释放）
}

FileTask::FileTask(FileTask&& other) noexcept
    : path(std::move(other.path))
    , status(other.status)
    , fileSize(other.fileSize)
    , header(std::move(other.header))
    , compressor(other.compressor)
    , buffer_(std::move(other.buffer_))
    , bufferUsed_(other.bufferUsed_)
    , overflow_(std::move(other.overflow_))
    , overflowPath_(std::move(other.overflowPath_))
    , written_(other.written_)
{
    other.compressor = nullptr;
    other.bufferUsed_ = 0;
    other.written_ = 0;
}

FileTask& FileTask::operator=(FileTask&& other) noexcept {
    if (this != &other) {
        path = std::move(other.path);
        status = other.status;
        fileSize = other.fileSize;
        header = std::move(other.header);
        compressor = other.compressor;
        buffer_ = std::move(other.buffer_);
        bufferUsed_ = other.bufferUsed_;
        overflow_ = std::move(other.overflow_);
        overflowPath_ = std::move(other.overflowPath_);
        written_ = other.written_;
        
        other.compressor = nullptr;
        other.bufferUsed_ = 0;
        other.written_ = 0;
    }
    return *this;
}

Error FileTask::reset(const fs::path& filePath, const fs::path& relativeTo) {
    // 清理之前的状态
    if (overflow_) {
        overflow_->close();
        if (!overflowPath_.empty() && fs::exists(overflowPath_)) {
            std::error_code ec;
            fs::remove(overflowPath_, ec);
        }
        overflow_.reset();
        overflowPath_.clear();
    }
    
    bufferUsed_ = 0;
    written_ = 0;
    
    // 设置新文件信息
    path = filePath;
    
    std::error_code ec;
    status = fs::status(path, ec);
    if (ec) {
        return Error(ErrorCode::FILE_NOT_FOUND, "Cannot stat file: " + path.string());
    }
    
    if (fs::is_regular_file(status)) {
        fileSize = fs::file_size(path, ec);
        if (ec) {
            return Error(ErrorCode::FILE_READ_ERROR, "Cannot get file size: " + path.string());
        }
    } else {
        fileSize = 0;
    }
    
    // 初始化头信息
    header = ZipFileHeader();
    
    // 设置相对路径名
    if (!relativeTo.empty()) {
        fs::path relPath = fs::relative(path, relativeTo, ec);
        if (ec) {
            // 如果无法计算相对路径，使用文件名
            header.name = path.filename().string();
        } else {
            // 加上基础目录名
            fs::path baseName = relativeTo.filename();
            header.name = utils::toZipPath(baseName / relPath);
        }
    } else {
        header.name = utils::toZipPath(path.filename());
    }
    
    // 如果是目录，确保名称以 / 结尾
    if (fs::is_directory(status) && !header.name.empty() && header.name.back() != '/') {
        header.name += '/';
    }
    
    // 设置修改时间
    auto modTime = utils::getModTime(path);
    
    // 设置文件权限（Unix）
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        header.externalAttr = utils::modeToZipAttr(st.st_mode);
    }
    
    return Error();
}

size_t FileTask::write(const uint8_t* data, size_t size) {
    size_t totalWritten = 0;
    
    // 先写入内存缓冲区
    size_t available = buffer_.size() - bufferUsed_;
    if (available > 0) {
        size_t toWrite = std::min(available, size);
        std::memcpy(buffer_.data() + bufferUsed_, data, toWrite);
        bufferUsed_ += toWrite;
        totalWritten += toWrite;
        data += toWrite;
        size -= toWrite;
    }
    
    // 剩余数据写入临时文件
    if (size > 0) {
        if (!overflow_) {
            overflowPath_ = utils::createTempFile("pzip-overflow");
            overflow_ = std::make_unique<std::fstream>(
                overflowPath_, 
                std::ios::in | std::ios::out | std::ios::binary | std::ios::trunc
            );
            
            if (!overflow_->is_open()) {
                return totalWritten;
            }
        }
        
        overflow_->write(reinterpret_cast<const char*>(data), size);
        totalWritten += size;
    }
    
    written_ += totalWritten;
    return totalWritten;
}

void FileTask::readCompressedData(std::function<void(const uint8_t*, size_t)> callback) {
    // 先读取内存缓冲区
    if (bufferUsed_ > 0) {
        callback(buffer_.data(), bufferUsed_);
    }
    
    // 再读取溢出文件
    if (overflow_) {
        overflow_->seekg(0, std::ios::beg);
        
        std::vector<uint8_t> readBuf(READ_BUFFER_SIZE);
        while (overflow_->good() && !overflow_->eof()) {
            overflow_->read(reinterpret_cast<char*>(readBuf.data()), readBuf.size());
            auto bytesRead = overflow_->gcount();
            if (bytesRead > 0) {
                callback(readBuf.data(), bytesRead);
            }
        }
    }
}

// ============================================================================
// FileTaskPool 实现
// ============================================================================

FileTaskPool& FileTaskPool::instance() {
    static FileTaskPool pool;
    return pool;
}

std::unique_ptr<FileTask> FileTaskPool::acquire() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (!pool_.empty()) {
        auto task = std::move(pool_.back());
        pool_.pop_back();
        return task;
    }
    
    return std::make_unique<FileTask>();
}

void FileTaskPool::release(std::unique_ptr<FileTask> task) {
    if (!task) return;
    
    std::lock_guard<std::mutex> lock(mutex_);
    
    // 限制池大小
    if (pool_.size() < 32) {
        pool_.push_back(std::move(task));
    }
}

} // namespace pzip


