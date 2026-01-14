// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "pzip/utils.h"
#include <zlib.h>
#include <sys/stat.h>
#include <utime.h>
#include <sstream>
#include <iomanip>
#include <cstring>
#include <random>

namespace pzip {
namespace utils {

std::pair<bool, bool> detectUTF8(const std::string& s) {
    bool valid = true;
    bool require = false;
    
    size_t i = 0;
    while (i < s.size()) {
        unsigned char c = s[i];
        
        if (c < 0x20 || c > 0x7d || c == 0x5c) {
            // 需要检查是否是有效的多字节 UTF-8
            if (c < 0x80) {
                // ASCII 控制字符或反斜杠
                require = true;
            } else if ((c & 0xE0) == 0xC0) {
                // 2 字节序列
                if (i + 1 >= s.size() || (s[i+1] & 0xC0) != 0x80) {
                    valid = false;
                    break;
                }
                i += 1;
                require = true;
            } else if ((c & 0xF0) == 0xE0) {
                // 3 字节序列
                if (i + 2 >= s.size() || 
                    (s[i+1] & 0xC0) != 0x80 || 
                    (s[i+2] & 0xC0) != 0x80) {
                    valid = false;
                    break;
                }
                i += 2;
                require = true;
            } else if ((c & 0xF8) == 0xF0) {
                // 4 字节序列
                if (i + 3 >= s.size() || 
                    (s[i+1] & 0xC0) != 0x80 || 
                    (s[i+2] & 0xC0) != 0x80 ||
                    (s[i+3] & 0xC0) != 0x80) {
                    valid = false;
                    break;
                }
                i += 3;
                require = true;
            } else {
                valid = false;
                break;
            }
        }
        i++;
    }
    
    return {valid, require};
}

std::string toZipPath(const fs::path& path) {
    std::string result = path.generic_string();
    
    // 确保使用正斜杠
    for (char& c : result) {
        if (c == '\\') c = '/';
    }
    
    // 移除开头的斜杠
    while (!result.empty() && result[0] == '/') {
        result = result.substr(1);
    }
    
    return result;
}

fs::path fromZipPath(const std::string& zipPath) {
    std::string result = zipPath;
    
    // 在 Windows 上转换斜杠
#ifdef _WIN32
    for (char& c : result) {
        if (c == '/') c = '\\';
    }
#endif
    
    return fs::path(result);
}

uint32_t crc32(const uint8_t* data, size_t size) {
    return ::crc32(0L, data, size);
}

uint32_t crc32Update(uint32_t crc, const uint8_t* data, size_t size) {
    return ::crc32(crc, data, size);
}

time_t getModTime(const fs::path& path) {
    struct stat st;
    if (stat(path.c_str(), &st) == 0) {
        return st.st_mtime;
    }
    return time(nullptr);
}

bool setModTime(const fs::path& path, time_t modTime) {
    struct utimbuf times;
    times.actime = modTime;
    times.modtime = modTime;
    return utime(path.c_str(), &times) == 0;
}

uint32_t modeToZipAttr(mode_t mode) {
    // ZIP 外部属性：高 16 位是 Unix 模式，低 16 位是 DOS 属性
    return static_cast<uint32_t>(mode) << 16;
}

mode_t zipAttrToMode(uint32_t attr) {
    // 提取高 16 位作为 Unix 模式
    return static_cast<mode_t>(attr >> 16);
}

fs::path createTempFile(const std::string& prefix) {
    // 获取临时目录
    fs::path tempDir = fs::temp_directory_path();
    
    // 生成随机文件名
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 999999);
    
    std::ostringstream oss;
    oss << prefix << "-" << std::setfill('0') << std::setw(6) << dis(gen);
    
    return tempDir / oss.str();
}

std::string formatSize(uint64_t size) {
    const char* units[] = {"B", "KB", "MB", "GB", "TB"};
    int unit = 0;
    double dsize = static_cast<double>(size);
    
    while (dsize >= 1024.0 && unit < 4) {
        dsize /= 1024.0;
        unit++;
    }
    
    std::ostringstream oss;
    if (unit == 0) {
        oss << size << " " << units[unit];
    } else {
        oss << std::fixed << std::setprecision(1) << dsize << " " << units[unit];
    }
    
    return oss.str();
}

std::string formatTime(double seconds) {
    std::ostringstream oss;
    
    if (seconds < 60) {
        oss << std::fixed << std::setprecision(1) << seconds << "s";
    } else if (seconds < 3600) {
        int mins = static_cast<int>(seconds) / 60;
        int secs = static_cast<int>(seconds) % 60;
        oss << mins << "m " << secs << "s";
    } else {
        int hours = static_cast<int>(seconds) / 3600;
        int mins = (static_cast<int>(seconds) % 3600) / 60;
        oss << hours << "h " << mins << "m";
    }
    
    return oss.str();
}

} // namespace utils
} // namespace pzip


