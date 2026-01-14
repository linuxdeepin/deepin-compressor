// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file pzip.hpp
 * @brief pzip 主头文件
 * 
 * 包含所有公共 API
 */

#include "common.h"
#include "archiver.h"
#include "extractor.h"
#include "utils.h"

namespace pzip {

/**
 * @brief 获取版本号
 */
inline const char* version() {
    return "1.0.0";
}

/**
 * @brief 快速压缩文件/目录
 * @param archivePath 输出 ZIP 文件路径
 * @param paths 要压缩的文件/目录列表
 * @param options 压缩选项
 * @return 错误信息
 */
inline Error compress(const fs::path& archivePath, 
                      const std::vector<fs::path>& paths,
                      const ArchiverOptions& options = {}) {
    Archiver archiver(archivePath, options);
    Error err = archiver.archive(paths);
    if (err) return err;
    return archiver.close();
}

/**
 * @brief 快速解压 ZIP 文件
 * @param archivePath ZIP 文件路径
 * @param outputDir 输出目录
 * @param options 解压选项
 * @return 错误信息
 */
inline Error decompress(const fs::path& archivePath,
                        const fs::path& outputDir,
                        const ExtractorOptions& options = {}) {
    Extractor extractor(outputDir, options);
    Error err = extractor.extract(archivePath);
    if (err) return err;
    return extractor.close();
}

} // namespace pzip


