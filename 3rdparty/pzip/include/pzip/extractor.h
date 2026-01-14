// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common.h"
#include "worker_pool.h"
#include "zip_reader.h"

namespace pzip {

/**
 * @brief 解压器选项
 */
struct ExtractorOptions {
    size_t concurrency = 0;          // 并发数（0 = CPU 核心数）
    bool preservePermissions = true; // 保留文件权限
    bool overwrite = true;           // 是否覆盖已存在的文件
    ProgressCallback progress;       // 进度回调
};

/**
 * @brief 解压任务
 */
struct ExtractTask {
    const ZipEntry* entry;
    fs::path outputPath;
    bool success = false;
};

/**
 * @brief 并行 ZIP 解压器
 * 
 * 对应 Go 版 pzip 的 extractor
 */
class Extractor {
public:
    /**
     * @brief 构造函数
     * @param outputDir 解压目标目录
     * @param options 选项
     */
    explicit Extractor(const fs::path& outputDir, const ExtractorOptions& options = {});
    ~Extractor();

    // 禁止拷贝
    Extractor(const Extractor&) = delete;
    Extractor& operator=(const Extractor&) = delete;

    /**
     * @brief 解压 ZIP 文件
     * @param archivePath ZIP 文件路径
     * @return 错误信息
     */
    Error extract(const fs::path& archivePath);

    /**
     * @brief 取消解压
     */
    void cancel();

    /**
     * @brief 关闭解压器
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
    // 解压单个文件
    Error extractFile(ExtractTask* task);
    
    // 写入目录
    Error writeDirectory(const fs::path& path, const ZipEntry& entry);
    
    // 写入文件
    Error writeFile(const fs::path& path, const ZipEntry& entry);
    
    // 设置文件权限
    Error setPermissions(const fs::path& path, const ZipEntry& entry);
    
    // 计算输出路径
    fs::path outputPath(const std::string& name) const;

    fs::path outputDir_;
    fs::path absoluteOutputDir_;
    ExtractorOptions options_;
    
    std::unique_ptr<ZipReader> reader_;
    std::unique_ptr<WorkerPool<ExtractTask>> fileWorkerPool_;
    
    std::atomic<size_t> processedFiles_{0};
    std::atomic<size_t> totalFiles_{0};
    std::atomic<bool> cancelled_{false};
};

} // namespace pzip

