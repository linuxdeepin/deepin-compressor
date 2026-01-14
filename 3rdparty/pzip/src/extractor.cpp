// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pzip/extractor.h"
#include "pzip/utils.h"
#include <fstream>
#include <sys/stat.h>

namespace pzip {

// ============================================================================
// Extractor 实现
// ============================================================================

Extractor::Extractor(const fs::path& outputDir, const ExtractorOptions& options)
    : outputDir_(outputDir)
    , options_(options)
{
    std::error_code ec;
    absoluteOutputDir_ = fs::absolute(outputDir, ec);
    if (ec) {
        absoluteOutputDir_ = outputDir;
    }
    
    // 确定并发数
    size_t concurrency = options_.concurrency;
    if (concurrency == 0) {
        concurrency = std::thread::hardware_concurrency();
    }
    
    // 创建解压线程池
    auto executor = [this](ExtractTask* task) -> Error {
        return extractFile(task);
    };
    fileWorkerPool_ = std::make_unique<WorkerPool<ExtractTask>>(
        executor, concurrency, concurrency * 2
    );
}

Extractor::~Extractor() = default;

Error Extractor::extract(const fs::path& archivePath) {
    // 创建读取器
    reader_ = std::make_unique<ZipReader>(archivePath);
    
    // 打开 ZIP 文件
    Error err = reader_->open();
    if (err) return err;
    
    // 确保输出目录存在
    std::error_code ec;
    fs::create_directories(absoluteOutputDir_, ec);
    if (ec) {
        return Error(ErrorCode::FILE_WRITE_ERROR, "Cannot create output directory");
    }
    
    // 获取所有条目
    const auto& entries = reader_->entries();
    totalFiles_ = entries.size();
    
    // 创建任务
    std::vector<std::unique_ptr<ExtractTask>> tasks;
    tasks.reserve(entries.size());
    
    for (const auto& entry : entries) {
        auto task = std::make_unique<ExtractTask>();
        task->entry = &entry;
        task->outputPath = outputPath(entry.header.name);
        tasks.push_back(std::move(task));
    }
    
    // 启动线程池
    fileWorkerPool_->start();
    
    // 入队任务
    for (auto& task : tasks) {
        if (cancelled_) break;
        fileWorkerPool_->enqueue(task.release());
    }
    
    // 等待完成
    err = fileWorkerPool_->close();
    
    return err;
}

Error Extractor::extractFile(ExtractTask* task) {
    if (cancelled_) {
        delete task;
        return Error(ErrorCode::CANCELLED, "Operation cancelled");
    }
    
    const auto& entry = *task->entry;
    const auto& path = task->outputPath;
    
    Error err;
    
    // 确保父目录存在
    std::error_code ec;
    fs::create_directories(path.parent_path(), ec);
    
    if (entry.isDirectory()) {
        err = writeDirectory(path, entry);
    } else {
        err = writeFile(path, entry);
    }
    
    if (!err) {
        task->success = true;
        
        // 设置权限
        setPermissions(path, entry);
    }
    
    // 更新进度
    processedFiles_++;
    if (options_.progress) {
        options_.progress(processedFiles_, totalFiles_);
    }
    
    delete task;
    return err;
}

Error Extractor::writeDirectory(const fs::path& path, const ZipEntry& entry) {
    std::error_code ec;
    
    if (fs::exists(path, ec)) {
        if (!fs::is_directory(path, ec)) {
            if (!options_.overwrite) {
                return Error();  // 跳过
            }
            fs::remove(path, ec);
        }
    }
    
    fs::create_directories(path, ec);
    if (ec) {
        return Error(ErrorCode::FILE_WRITE_ERROR, "Cannot create directory: " + path.string());
    }
    
    return Error();
}

Error Extractor::writeFile(const fs::path& path, const ZipEntry& entry) {
    std::error_code ec;
    
    // 检查是否存在
    if (fs::exists(path, ec)) {
        if (!options_.overwrite) {
            return Error();  // 跳过
        }
    }
    
    // 解压到文件
    return reader_->extractTo(entry, path);
}

Error Extractor::setPermissions(const fs::path& path, const ZipEntry& entry) {
    if (!options_.preservePermissions) {
        return Error();
    }
    
    mode_t mode = utils::zipAttrToMode(entry.header.externalAttr);
    if (mode != 0) {
        chmod(path.c_str(), mode);
    }
    
    return Error();
}

fs::path Extractor::outputPath(const std::string& name) const {
    // 转换 ZIP 路径到本地路径
    fs::path localPath = utils::fromZipPath(name);
    return absoluteOutputDir_ / localPath;
}

void Extractor::cancel() {
    cancelled_ = true;
    if (fileWorkerPool_) {
        fileWorkerPool_->cancel();
    }
}

Error Extractor::close() {
    if (reader_) {
        reader_->close();
    }
    return Error();
}

void Extractor::setProgressCallback(ProgressCallback callback) {
    options_.progress = std::move(callback);
}

} // namespace pzip

