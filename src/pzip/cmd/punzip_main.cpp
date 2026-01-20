// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file punzip_main.cpp
 * @brief punzip 命令行工具 - 并行 ZIP 解压
 * 
 * 用法: punzip [选项] <archive.zip> [-d <输出目录>]
 */

#include "pzip/pzip.h"
#include <iostream>
#include <string>
#include <chrono>
#include <getopt.h>

void printUsage(const char* progName) {
    std::cout << "punzip - Parallel ZIP Extractor v" << pzip::version() << "\n\n"
              << "用法: " << progName << " [选项] <archive.zip>\n\n"
              << "选项:\n"
              << "  -d, --directory <dir>  解压到指定目录（默认: 当前目录）\n"
              << "  -c, --concurrency <n>  设置并发线程数（默认: CPU 核心数）\n"
              << "  -o, --overwrite        覆盖已存在的文件\n"
              << "  -n, --no-overwrite     不覆盖已存在的文件\n"
              << "  -v, --verbose          显示详细信息\n"
              << "  -q, --quiet            静默模式\n"
              << "  -l, --list             仅列出文件，不解压\n"
              << "  -h, --help             显示帮助信息\n"
              << "\n"
              << "示例:\n"
              << "  " << progName << " archive.zip\n"
              << "  " << progName << " -d /tmp/output archive.zip\n"
              << "  " << progName << " -c 4 -v archive.zip\n";
}

void listArchive(const std::string& archivePath) {
    pzip::ZipReader reader(archivePath);
    pzip::Error err = reader.open();
    
    if (err) {
        std::cerr << "错误: " << err.message << "\n";
        return;
    }
    
    const auto& entries = reader.entries();
    
    std::cout << "  长度      日期     时间    名称\n";
    std::cout << "--------  ---------- -----   ----\n";
    
    uint64_t totalSize = 0;
    for (const auto& entry : entries) {
        // 格式化时间
        time_t modTime = 0;  // TODO: 从 header 获取
        struct tm* tm = localtime(&modTime);
        
        char dateStr[32] = "0000-00-00 00:00";
        if (tm) {
            snprintf(dateStr, sizeof(dateStr), "%04d-%02d-%02d %02d:%02d",
                     tm->tm_year + 1900, tm->tm_mon + 1, tm->tm_mday,
                     tm->tm_hour, tm->tm_min);
        }
        
        std::cout << std::setw(8) << entry.header.uncompressedSize << "  "
                  << dateStr << "   "
                  << entry.header.name << "\n";
        
        totalSize += entry.header.uncompressedSize;
    }
    
    std::cout << "--------                     -------\n";
    std::cout << std::setw(8) << totalSize << "                     " 
              << entries.size() << " 个文件\n";
}

int main(int argc, char* argv[]) {
    // 默认选项
    pzip::ExtractorOptions options;
    std::string outputDir = ".";
    bool verbose = false;
    bool quiet = false;
    bool listOnly = false;
    
    // 命令行选项定义
    static struct option longOptions[] = {
        {"directory",    required_argument, nullptr, 'd'},
        {"concurrency",  required_argument, nullptr, 'c'},
        {"overwrite",    no_argument,       nullptr, 'o'},
        {"no-overwrite", no_argument,       nullptr, 'n'},
        {"verbose",      no_argument,       nullptr, 'v'},
        {"quiet",        no_argument,       nullptr, 'q'},
        {"list",         no_argument,       nullptr, 'l'},
        {"help",         no_argument,       nullptr, 'h'},
        {nullptr,        0,                 nullptr, 0}
    };
    
    // 解析命令行选项
    int opt;
    while ((opt = getopt_long(argc, argv, "d:c:onvqlh", longOptions, nullptr)) != -1) {
        switch (opt) {
            case 'd':
                outputDir = optarg;
                break;
            case 'c':
                options.concurrency = std::stoul(optarg);
                break;
            case 'o':
                options.overwrite = true;
                break;
            case 'n':
                options.overwrite = false;
                break;
            case 'v':
                verbose = true;
                break;
            case 'q':
                quiet = true;
                break;
            case 'l':
                listOnly = true;
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
    if (argc - optind < 1) {
        std::cerr << "错误: 需要指定 ZIP 文件\n";
        printUsage(argv[0]);
        return 1;
    }
    
    std::string archivePath = argv[optind];
    
    // 验证文件存在
    if (!pzip::fs::exists(archivePath)) {
        std::cerr << "错误: 文件不存在: " << archivePath << "\n";
        return 1;
    }
    
    // 仅列出模式
    if (listOnly) {
        listArchive(archivePath);
        return 0;
    }
    
    // 设置进度回调
    if (!quiet) {
        options.progress = [verbose](size_t current, size_t total) {
            if (verbose) {
                std::cout << "\r解压进度: " << current << "/" << total 
                          << " (" << (total > 0 ? current * 100 / total : 0) << "%)" 
                          << std::flush;
            }
        };
    }
    
    // 开始计时
    auto startTime = std::chrono::high_resolution_clock::now();
    
    if (!quiet) {
        std::cout << "解压文件: " << archivePath << "\n";
        std::cout << "输出目录: " << outputDir << "\n";
        if (verbose) {
            std::cout << "并发线程数: " << (options.concurrency > 0 ? options.concurrency : std::thread::hardware_concurrency()) << "\n";
        }
    }
    
    // 执行解压
    pzip::Error err = pzip::decompress(archivePath, outputDir, options);
    
    // 结束计时
    auto endTime = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(endTime - startTime);
    double seconds = duration.count() / 1000.0;
    
    if (err) {
        std::cerr << "\n错误: " << err.message << "\n";
        return 1;
    }
    
    if (!quiet) {
        std::cout << "\n完成!\n";
        std::cout << "耗时: " << pzip::utils::formatTime(seconds) << "\n";
    }
    
    return 0;
}


