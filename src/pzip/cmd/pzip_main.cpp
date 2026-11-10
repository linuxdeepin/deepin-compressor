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
#include <iomanip>
#include <string>
#include <vector>
#include <chrono>
#include <getopt.h>
#include <fstream>
#include <iterator>
#include <condition_variable>
#include <optional>
#include <thread>
#include <mutex>
#include <atomic>

void printUsage(const char* progName) {
    std::cout << "pzip - Parallel ZIP Archiver v" << pzip::version() << "\n\n"
              << "用法: " << progName << " [选项] <archive.zip> <文件或目录...>\n\n"
              << "选项:\n"
              << "  -c, --concurrency <n>  设置并发线程数（默认: 全部 CPU 核心）\n"
              << "  -l, --level <0-9>      设置压缩级别（默认: 1，最快；0=不压缩）\n"
              << "  -p, --password <密码>  设置加密密码（UTF-8/终端字节）\n"
              << "  -P, --password-file <路径>  从文件读取密码字节（与 libzip 插件编码对齐时由 GUI 写入）\n"
              << "  -e, --encryption <方法> 加密方法: aes128, aes192, aes256（默认: aes256）\n"
              << "      --ui-events        输出给 GUI 解析的事件行（例如当前文件名），格式稳定，适合配合 -q 使用\n"
              << "  -v, --verbose          显示详细信息\n"
              << "  -q, --quiet            静默模式\n"
              << "  -h, --help             显示帮助信息\n"
              << "\n"
              << "示例:\n"
              << "  " << progName << " archive.zip file1.txt file2.txt\n"
              << "  " << progName << " archive.zip directory/\n"
              << "  " << progName << " -c 4 -l 9 archive.zip files/\n"
              << "  " << progName << " -p mypassword archive.zip files/\n"
              << "  " << progName << " -p mypassword -e aes256 archive.zip files/\n";
}

int main(int argc, char* argv[]) {
    // 默认选项
    pzip::ArchiverOptions options;
    bool verbose = false;
    bool quiet = false;
    bool uiEvents = false;
    std::string encryptionMethodStr;

    // 命令行选项定义
    static struct option longOptions[] = {
        {"concurrency", required_argument, nullptr, 'c'},
        {"level",       required_argument, nullptr, 'l'},
        {"password",    required_argument, nullptr, 'p'},
        {"password-file", required_argument, nullptr, 'P'},
        {"encryption",  required_argument, nullptr, 'e'},
        {"ui-events",   no_argument,       nullptr,  1 },
        {"verbose",     no_argument,       nullptr, 'v'},
        {"quiet",       no_argument,       nullptr, 'q'},
        {"help",        no_argument,       nullptr, 'h'},
        {nullptr,       0,                 nullptr, 0}
    };

    // 解析命令行选项
    int opt;
    while ((opt = getopt_long(argc, argv, "c:l:p:P:e:vqh", longOptions, nullptr)) != -1) {
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
            case 'p':
                options.password = optarg;
                break;
            case 'P': {
                std::ifstream pf(optarg, std::ios::binary);
                if (!pf) {
                    std::cerr << "错误: 无法读取密码文件: " << optarg << "\n";
                    return 1;
                }
                options.password.assign(std::istreambuf_iterator<char>(pf), std::istreambuf_iterator<char>());
                break;
            }
            case 'e':
                encryptionMethodStr = optarg;
                break;
            case 1:
                uiEvents = true;
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

    // 解析加密方法
    if (!options.password.empty()) {
        if (encryptionMethodStr.empty() || encryptionMethodStr == "aes256") {
            options.encryptionMethod = pzip::EncryptionMethod::AES256;
        } else if (encryptionMethodStr == "aes192") {
            options.encryptionMethod = pzip::EncryptionMethod::AES192;
        } else if (encryptionMethodStr == "aes128") {
            options.encryptionMethod = pzip::EncryptionMethod::AES128;
        } else {
            std::cerr << "错误: 不支持的加密方法: " << encryptionMethodStr << "\n";
            std::cerr << "支持的加密方法: aes128, aes192, aes256\n";
            return 1;
        }
    }

    // --ui-events 可能被 GUI 打开用于“当前文件名 + 进度/速率/剩余时间”显示；
    // 如果直接在压缩线程里打印（尤其是大量小文件），会被 stdout/pty 吞吐拖慢。
    // 因此这里做“异步合并输出”：
    // - 压缩线程只上报 name / bytes 到内存（极轻量）
    // - 单独线程限频输出：[PZIP_UI] entry / [PZIP_UI] inbytes
    struct UiEventEmitter {
        std::mutex mu;
        std::condition_variable cv;
        std::optional<std::string> pendingEntry;
        bool stop = false;
        std::atomic<uint64_t> inBytes{0};

        void pushEntry(std::string s) {
            {
                std::lock_guard<std::mutex> lk(mu);
                pendingEntry = std::move(s); // 合并：只保留最新
            }
            cv.notify_one();
        }

        void addBytes(uint64_t delta) {
            inBytes.fetch_add(delta, std::memory_order_relaxed);
            cv.notify_one();
        }

        void requestStop() {
            {
                std::lock_guard<std::mutex> lk(mu);
                stop = true;
            }
            cv.notify_one();
        }
    };
    std::unique_ptr<UiEventEmitter> ui;
    std::thread uiThread;
    if (uiEvents) {
        ui = std::make_unique<UiEventEmitter>();

        // 输出线程：打印最新 entry；同时周期性输出累计输入字节（用于 GUI 计算速度/剩余时间）
        uiThread = std::thread([em = ui.get()] {
            std::string lastPrinted;
            uint64_t lastInBytes = 0;
            while (true) {
                std::optional<std::string> curEntry;
                {
                    std::unique_lock<std::mutex> lk(em->mu);
                    em->cv.wait_for(lk, std::chrono::milliseconds(100), [&] {
                        return em->stop || em->pendingEntry.has_value();
                    });
                    if (em->stop && !em->pendingEntry.has_value()) {
                        break;
                    }
                    curEntry = std::move(em->pendingEntry);
                    em->pendingEntry.reset();
                }

                if (curEntry && *curEntry != lastPrinted) {
                    std::cout << "[PZIP_UI] entry " << *curEntry << "\n";
                    lastPrinted = *curEntry;
                }

                const uint64_t curInBytes = em->inBytes.load(std::memory_order_relaxed);
                if (curInBytes != lastInBytes) {
                    std::cout << "[PZIP_UI] inbytes " << curInBytes << "\n";
                    lastInBytes = curInBytes;
                }
            }
        });

        options.onEntryStart = [em = ui.get()](const std::string& nameInArchive) {
            em->pushEntry(nameInArchive);
        };
        options.onBytesRead = [em = ui.get()](uint64_t delta) {
            if (delta > 0) {
                em->addBytes(delta);
            }
        };
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
            std::cout << "压缩级别: " << options.compressionLevel << "\n";
            if (options.isEncrypted()) {
                std::cout << "加密: 是\n";
                std::cout << "加密方法: ";
                switch (options.encryptionMethod) {
                    case pzip::EncryptionMethod::AES128:
                        std::cout << "AES-128\n";
                        break;
                    case pzip::EncryptionMethod::AES192:
                        std::cout << "AES-192\n";
                        break;
                    case pzip::EncryptionMethod::AES256:
                        std::cout << "AES-256\n";
                        break;
                    default:
                        std::cout << "未知\n";
                }
            }
        }
    }
    
    // 执行压缩
    pzip::Error err = pzip::compress(archivePath, inputPaths, options);

    if (ui) {
        ui->requestStop();
        if (uiThread.joinable()) {
            uiThread.join();
        }
    }
    
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


