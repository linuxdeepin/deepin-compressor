// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file pzip_main.cpp
 * @brief pzip 命令行工具 - 并行 ZIP 压缩
 * 
 * 用法: pzip [选项] <archive.zip> <文件或目录...>
 */

#include "pzip/pzip.h"
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <getopt.h>

void printUsage(const char* progName) {
    std::cout << "pzip - Parallel ZIP Archiver v" << pzip::version() << "\n\n"
              << "用法: " << progName << " [选项] <archive.zip> <文件或目录...>\n\n"
              << "选项:\n"
              << "  -c, --concurrency <n>  设置并发线程数（默认: CPU 核心数）\n"
              << "  -l, --level <0-9>      设置压缩级别（默认: 6）\n"
              << "  -v, --verbose          显示详细信息\n"
              << "  -q, --quiet            静默模式\n"
              << "  -h, --help             显示帮助信息\n"
              << "\n"
              << "示例:\n"
              << "  " << progName << " archive.zip file1.txt file2.txt\n"
              << "  " << progName << " archive.zip directory/\n"
              << "  " << progName << " -c 4 -l 9 archive.zip files/\n";
}

int main(int argc, char* argv[]) {
    // 默认选项
    pzip::ArchiverOptions options;
    bool verbose = false;
    bool quiet = false;
    
    // 命令行选项定义
    static struct option longOptions[] = {
        {"concurrency", required_argument, nullptr, 'c'},
        {"level",       required_argument, nullptr, 'l'},
        {"verbose",     no_argument,       nullptr, 'v'},
        {"quiet",       no_argument,       nullptr, 'q'},
        {"help",        no_argument,       nullptr, 'h'},
        {nullptr,       0,                 nullptr, 0}
    };
    
    // 解析命令行选项
    int opt;
    while ((opt = getopt_long(argc, argv, "c:l:vqh", longOptions, nullptr)) != -1) {
        switch (opt) {
            case 'c':
                options.concurrency = std::stoul(optarg);
                break;
            case 'l':
                options.compressionLevel = std::stoi(optarg);
                if (options.compressionLevel < 0 || options.compressionLevel > 9) {
                    std::cerr << "错误: 压缩级别必须在 0-9 之间\n";
                    return 1;
                }
                break;
            case 'v':
                verbose = true;
                break;
            case 'q':
                quiet = true;
                break;
            case 'h':
                printUsage(argv[0]);
                return 0;
            default:
                printUsage(argv[0]);
                return 1;
        }
    }
    
    // 检查参数数量
    if (argc - optind < 2) {
        std::cerr << "错误: 需要指定输出文件和输入文件/目录\n";
        printUsage(argv[0]);
        return 1;
    }
    
    // 获取输出文件和输入文件列表
    std::string archivePath = argv[optind];
    std::vector<pzip::fs::path> inputPaths;
    for (int i = optind + 1; i < argc; ++i) {
        inputPaths.emplace_back(argv[i]);
    }
    
    // 验证输入文件存在
    for (const auto& path : inputPaths) {
        if (!pzip::fs::exists(path)) {
            std::cerr << "错误: 文件不存在: " << path << "\n";
            return 1;
        }
    }
    
    // 设置进度回调
    if (!quiet) {
        options.progress = [verbose](size_t current, size_t total) {
            if (verbose) {
                std::cout << "\r压缩进度: " << current << "/" << total 
                          << " (" << (total > 0 ? current * 100 / total : 0) << "%)" 
                          << std::flush;
            }
        };
    }
    
    // 开始计时
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (!quiet) {
        std::cout << "创建压缩文件: " << archivePath << "\n";
        if (verbose) {
            std::cout << "并发线程数: " << (options.concurrency > 0 ? options.concurrency : std::thread::hardware_concurrency()) << "\n";
            std::cout << "压缩级别: " << (options.compressionLevel < 0 ? 6 : options.compressionLevel) << "\n";
        }
    }
    
    // 执行压缩
    pzip::Error err = pzip::compress(archivePath, inputPaths, options);
    
    // 结束计时
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    double seconds = duration.count() / 1000.0;
    
    if (err) {
        std::cerr << "\n错误: " << err.message << "\n";
        return 1;
    }
    
    if (!quiet) {
        std::cout << "\n";
        
        // 显示统计信息
        std::error_code ec;
        auto archiveSize = pzip::fs::file_size(archivePath, ec);
        
        uint64_t totalInputSize = 0;
        for (const auto& path : inputPaths) {
            if (pzip::fs::is_directory(path, ec)) {
                for (const auto& entry : pzip::fs::recursive_directory_iterator(path, ec)) {
                    if (entry.is_regular_file()) {
                        totalInputSize += entry.file_size();
                    }
                }
            } else {
                totalInputSize += pzip::fs::file_size(path, ec);
            }
        }
        
        std::cout << "完成!\n";
        std::cout << "原始大小: " << pzip::utils::formatSize(totalInputSize) << "\n";
        std::cout << "压缩大小: " << pzip::utils::formatSize(archiveSize) << "\n";
        
        if (totalInputSize > 0) {
            double ratio = 100.0 * (1.0 - static_cast<double>(archiveSize) / totalInputSize);
            std::cout << "压缩率: " << std::fixed << std::setprecision(1) << ratio << "%\n";
        }
        
        std::cout << "耗时: " << pzip::utils::formatTime(seconds) << "\n";
        
        if (seconds > 0 && totalInputSize > 0) {
            double speed = totalInputSize / seconds / (1024 * 1024);
            std::cout << "速度: " << std::fixed << std::setprecision(1) << speed << " MB/s\n";
        }
    }
    
    return 0;
}


