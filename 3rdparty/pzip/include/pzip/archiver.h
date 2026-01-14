// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common.h"
#include "worker_pool.h"
#include "file_task.h"
#include "zip_writer.h"

namespace pzip {

/**
 * @brief 压缩器选项
 */
struct ArchiverOptions {
    size_t concurrency = 0;         // 并发数（0 = CPU 核心数）
    int compressionLevel = -1;       // 压缩级别（-1 = 默认，0-9）
    bool preservePermissions = true; // 保留文件权限
    ProgressCallback progress;       // 进度回调
};

/**
 * @brief 并行 ZIP 压缩器
 * 
 * 核心实现类，对应 Go 版 pzip 的 archiver
 * 
 * 架构：
 * - fileProcessPool: 多线程并行压缩文件
 * - fileWriterPool: 单线程顺序写入 ZIP（因为 ZIP 格式要求顺序写入）
 */
class Archiver {
public:
    /**
     * @brief 构造函数
     * @param archive ZIP 文件路径
     * @param options 选项
     */
    explicit Archiver(const fs::path& archive, const ArchiverOptions& options = {});
    ~Archiver();

    // 禁止拷贝
    Archiver(const Archiver&) = delete;
    Archiver& operator=(const Archiver&) = delete;

    /**
     * @brief 压缩文件或目录
     * @param paths 要压缩的文件/目录列表
     * @return 错误信息
     */
    Error archive(const std::vector<fs::path>& paths);

    /**
     * @brief 取消压缩
     */
    void cancel();

    /**
     * @brief 关闭压缩器
     * @return 错误信息
     */
    Error close();

    /**
     * @brief 设置进度回调
     */
    void setProgressCallback(ProgressCallback callback);

    /**
     * @brief 获取已处理的文件数
     */
    size_t processedFiles() const { return processedFiles_; }

    /**
     * @brief 获取总文件数
     */
    size_t totalFiles() const { return totalFiles_; }

private:
    // 压缩单个文件
    Error compressFile(FileTask* task);
    
    // 写入单个文件到 ZIP
    Error archiveFile(FileTask* task);
    
    // 遍历目录
    Error walkDirectory(const fs::path& root);
    
    // 压缩文件内容
    Error compress(FileTask* task);
    
    // 填充 ZIP 文件头
    void populateHeader(FileTask* task);
    
    // 设置相对路径名
    void setRelativeName(FileTask* task, const fs::path& relativeTo);

    fs::path archivePath_;
    fs::path absoluteArchivePath_;
    ArchiverOptions options_;
    
    std::unique_ptr<ZipWriter> writer_;
    std::unique_ptr<WorkerPool<FileTask>> fileProcessPool_;
    std::unique_ptr<WorkerPool<FileTask>> fileWriterPool_;
    
    fs::path chroot_;  // 当前根目录
    
    std::atomic<size_t> processedFiles_{0};
    std::atomic<size_t> totalFiles_{0};
    std::atomic<bool> cancelled_{false};
    
};

} // namespace pzip


