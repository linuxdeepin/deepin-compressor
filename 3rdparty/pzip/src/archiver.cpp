// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pzip/archiver.h"
#include "pzip/fast_deflate.h"
#include "pzip/utils.h"
#include <fstream>
#include <cstring>
#include <sys/stat.h>
#include <zlib.h>

#ifdef USE_LIBDEFLATE
#include <libdeflate.h>
#endif

namespace pzip {

// ============================================================================
// Archiver 实现
// ============================================================================

Archiver::Archiver(const fs::path& archive, const ArchiverOptions& options)
    : archivePath_(archive)
    , options_(options)
{
    std::error_code ec;
    absoluteArchivePath_ = fs::absolute(archive, ec);
    if (ec) {
        absoluteArchivePath_ = archive;
    }
    
    // 创建 ZIP 写入器
    writer_ = std::make_unique<ZipWriter>(archivePath_);
    
    // 确定并发数
    size_t concurrency = options_.concurrency;
    if (concurrency == 0) {
        concurrency = std::thread::hardware_concurrency();
    }
    
    // 创建文件处理线程池（并行压缩）
    auto processExecutor = [this](FileTask* task) -> Error {
        return compressFile(task);
    };
    fileProcessPool_ = std::make_unique<WorkerPool<FileTask>>(
        processExecutor, concurrency, concurrency * 2
    );
    
    // 创建文件写入线程池（顺序写入，concurrency = 1）
    auto writeExecutor = [this](FileTask* task) -> Error {
        return archiveFile(task);
    };
    fileWriterPool_ = std::make_unique<WorkerPool<FileTask>>(
        writeExecutor, 1, concurrency * 2
    );
}

Archiver::~Archiver() {
    // 资源由智能指针自动管理
}

Error Archiver::archive(const std::vector<fs::path>& paths) {
    // 打开 ZIP 文件
    Error err = writer_->open();
    if (err) return err;
    
    // 启动线程池
    fileProcessPool_->start();
    fileWriterPool_->start();
    
    // 遍历所有路径
    for (const auto& path : paths) {
        if (cancelled_) break;
        
        std::error_code ec;
        auto status = fs::status(path, ec);
        if (ec) {
            return Error(ErrorCode::FILE_NOT_FOUND, "Cannot stat: " + path.string());
        }
        
        if (fs::is_directory(status)) {
            err = walkDirectory(path);
            if (err) return err;
        } else {
            // 单个文件
            chroot_.clear();
            
            auto task = FileTaskPool::instance().acquire();
            err = task->reset(path);
            if (err) {
                FileTaskPool::instance().release(std::move(task));
                return err;
            }
            
            // 跳过输出文件本身
            if (fs::equivalent(task->path, absoluteArchivePath_, ec)) {
                FileTaskPool::instance().release(std::move(task));
                continue;
            }
            
            totalFiles_++;
            fileProcessPool_->enqueue(task.release());
        }
    }
    
    // 等待处理完成
    err = fileProcessPool_->close();
    if (err) return err;
    
    err = fileWriterPool_->close();
    if (err) return err;
    
    return Error();
}

Error Archiver::walkDirectory(const fs::path& root) {
    std::error_code ec;
    chroot_ = fs::absolute(root, ec);
    if (ec) {
        return Error(ErrorCode::FILE_NOT_FOUND, "Cannot get absolute path: " + root.string());
    }
    
    for (auto it = fs::recursive_directory_iterator(chroot_, ec);
         it != fs::recursive_directory_iterator();
         ++it) {
        if (cancelled_) break;
        if (ec) {
            return Error(ErrorCode::FILE_READ_ERROR, "Directory iteration error: " + ec.message());
        }
        
        const auto& entry = *it;
        
        // 跳过输出文件本身
        if (fs::equivalent(entry.path(), absoluteArchivePath_, ec)) {
            continue;
        }
        
        auto task = FileTaskPool::instance().acquire();
        Error err = task->reset(entry.path(), chroot_.parent_path());
        if (err) {
            FileTaskPool::instance().release(std::move(task));
            continue;  // 跳过无法处理的文件
        }
        
        totalFiles_++;
        fileProcessPool_->enqueue(task.release());
    }
    
    // 也要添加根目录本身
    auto task = FileTaskPool::instance().acquire();
    Error err = task->reset(chroot_, chroot_.parent_path());
    if (!err) {
        totalFiles_++;
        fileProcessPool_->enqueue(task.release());
    } else {
        FileTaskPool::instance().release(std::move(task));
    }
    
    return Error();
}

Error Archiver::compressFile(FileTask* task) {
    if (cancelled_) {
        return Error(ErrorCode::CANCELLED, "Operation cancelled");
    }
    
    // 压缩文件内容
    Error err = compress(task);
    if (err) return err;
    
    // 填充头信息
    populateHeader(task);
    
    // 送入写入队列
    fileWriterPool_->enqueue(task);
    
    return Error();
}

Error Archiver::compress(FileTask* task) {
    // 目录不需要压缩
    if (fs::is_directory(task->status)) {
        return Error();
    }
    
    // 打开源文件
    std::ifstream file(task->path, std::ios::binary);
    if (!file.is_open()) {
        return Error(ErrorCode::FILE_OPEN_ERROR, "Cannot open file: " + task->path.string());
    }
    
    // 读取整个文件到内存
    std::vector<uint8_t> fileData(task->fileSize);
    file.read(reinterpret_cast<char*>(fileData.data()), task->fileSize);
    if (static_cast<size_t>(file.gcount()) != task->fileSize) {
        return Error(ErrorCode::FILE_READ_ERROR, "Failed to read file: " + task->path.string());
    }
    file.close();
    
#ifdef USE_LIBDEFLATE
    // 使用 libdeflate（高性能）
    // 注意：libdeflate level 1 最快，level 12 压缩率最高
    // 默认使用 level 1（最快），用户可以通过 -6 等参数调整
    task->header.crc32 = libdeflate_crc32(0, fileData.data(), fileData.size());
    
    int level = options_.compressionLevel;
    if (level < 1 || level > 12) level = 1;  // 默认使用最快级别
    
    struct libdeflate_compressor* compressor = libdeflate_alloc_compressor(level);
    if (!compressor) {
        return Error(ErrorCode::COMPRESSION_ERROR, "Failed to create compressor");
    }
    
    size_t maxCompressedSize = libdeflate_deflate_compress_bound(compressor, fileData.size());
    std::vector<uint8_t> compressed(maxCompressedSize);
    
    size_t compressedSize = libdeflate_deflate_compress(
        compressor, 
        fileData.data(), fileData.size(),
        compressed.data(), compressed.size()
    );
    
    libdeflate_free_compressor(compressor);
    
    if (compressedSize == 0 && !fileData.empty()) {
        return Error(ErrorCode::COMPRESSION_ERROR, "Compression failed");
    }
    
    task->write(compressed.data(), compressedSize);
#else
    // 使用内置压缩器 - 使用 thread_local 避免每次创建新对象
    task->header.crc32 = ::crc32(0L, fileData.data(), fileData.size());
    
    // thread_local 压缩器（使用最快级别）和输出缓冲区，避免重复分配
    thread_local FastDeflate deflate(CompressionLevel::BestSpeed);
    thread_local std::vector<uint8_t> compressed;
    
    // 重置压缩器状态并清空缓冲区
    deflate.reset();
    compressed.clear();
    
    size_t compressedSize = deflate.compress(fileData.data(), fileData.size(), compressed);
    
    if (compressedSize == 0 && !fileData.empty()) {
        return Error(ErrorCode::COMPRESSION_ERROR, "Compression failed");
    }
    
    task->write(compressed.data(), compressed.size());
#endif
    
    return Error();
}

void Archiver::populateHeader(FileTask* task) {
    auto& h = task->header;
    
    // UTF-8 检测
    auto [validUtf8, requireUtf8] = utils::detectUTF8(h.name);
    if (requireUtf8 && validUtf8) {
        h.flags |= ZIP_FLAG_UTF8;
    }
    
    // 版本信息
    h.versionMadeBy = (3 << 8) | ZIP_VERSION_20;  // Unix + ZIP 2.0
    h.versionNeeded = ZIP_VERSION_20;
    
    // 修改时间
    time_t modTime = utils::getModTime(task->path);
    ExtendedTimestamp ext;
    ext.modTime = modTime;
    auto extData = ext.encode();
    h.extra.insert(h.extra.end(), extData.begin(), extData.end());
    
    // DOS 时间
    struct tm* tm = localtime(&modTime);
    if (tm) {
        h.modTime = ((tm->tm_hour & 0x1F) << 11) |
                    ((tm->tm_min & 0x3F) << 5) |
                    ((tm->tm_sec / 2) & 0x1F);
        h.modDate = (((tm->tm_year - 80) & 0x7F) << 9) |
                    (((tm->tm_mon + 1) & 0x0F) << 5) |
                    (tm->tm_mday & 0x1F);
    }
    
    // 目录处理
    if (fs::is_directory(task->status)) {
        if (!h.name.empty() && h.name.back() != '/') {
            h.name += '/';
        }
        h.method = ZIP_METHOD_STORE;
        h.flags &= ~ZIP_FLAG_DATA_DESCRIPTOR;
        h.uncompressedSize = 0;
        h.compressedSize = 0;
        h.crc32 = 0;
    } else {
        h.method = ZIP_METHOD_DEFLATE;
        h.flags |= ZIP_FLAG_DATA_DESCRIPTOR;
        h.uncompressedSize = task->fileSize;
        h.compressedSize = task->written();
    }
}

Error Archiver::archiveFile(FileTask* task) {
    if (cancelled_) {
        FileTaskPool::instance().release(std::unique_ptr<FileTask>(task));
        return Error(ErrorCode::CANCELLED, "Operation cancelled");
    }
    
    // 写入 ZIP
    Error err = writer_->createRaw(task->header, 
        [task](std::function<void(const uint8_t*, size_t)> writer) {
            task->readCompressedData(writer);
        });
    
    // 更新进度
    processedFiles_++;
    if (options_.progress) {
        options_.progress(processedFiles_, totalFiles_);
    }
    
    // 释放任务
    FileTaskPool::instance().release(std::unique_ptr<FileTask>(task));
    
    return err;
}

void Archiver::cancel() {
    cancelled_ = true;
    if (fileProcessPool_) {
        fileProcessPool_->cancel();
    }
    if (fileWriterPool_) {
        fileWriterPool_->cancel();
    }
}

Error Archiver::close() {
    if (writer_ && writer_->isOpen()) {
        return writer_->close();
    }
    return Error();
}

void Archiver::setProgressCallback(ProgressCallback callback) {
    options_.progress = std::move(callback);
}

} // namespace pzip
