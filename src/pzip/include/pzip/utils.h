// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "common.h"
#include <filesystem>
#include <string>

namespace pzip {

namespace fs = std::filesystem;

/**
 * @brief 工具函数集合
 */
namespace utils {

/**
 * @brief 检测字符串是否为有效 UTF-8
 * @param s 输入字符串
 * @return {是否有效, 是否需要 UTF-8 标志}
 */
std::pair<bool, bool> detectUTF8(const std::string& s);

/**
 * @brief 将路径转换为 ZIP 格式（使用 /）
 * @param path 文件路径
 * @return ZIP 格式路径
 */
std::string toZipPath(const fs::path& path);

/**
 * @brief 从 ZIP 路径转换为本地路径
 * @param zipPath ZIP 格式路径
 * @return 本地路径
 */
fs::path fromZipPath(const std::string& zipPath);

/**
 * @brief 计算 CRC32
 * @param data 数据指针
 * @param size 数据大小
 * @return CRC32 值
 */
uint32_t crc32(const uint8_t* data, size_t size);

/**
 * @brief 更新 CRC32
 * @param crc 当前 CRC32 值
 * @param data 数据指针
 * @param size 数据大小
 * @return 更新后的 CRC32 值
 */
uint32_t crc32Update(uint32_t crc, const uint8_t* data, size_t size);

/**
 * @brief 获取文件修改时间
 * @param path 文件路径
 * @return Unix 时间戳
 */
time_t getModTime(const fs::path& path);

/**
 * @brief 设置文件修改时间
 * @param path 文件路径
 * @param modTime Unix 时间戳
 * @return 是否成功
 */
bool setModTime(const fs::path& path, time_t modTime);

/**
 * @brief 从 Unix 权限获取 ZIP 外部属性
 * @param mode Unix 文件模式
 * @return ZIP 外部属性
 */
uint32_t modeToZipAttr(mode_t mode);

/**
 * @brief 从 ZIP 外部属性获取 Unix 权限
 * @param attr ZIP 外部属性
 * @return Unix 文件模式
 */
mode_t zipAttrToMode(uint32_t attr);

/**
 * @brief 创建临时文件
 * @param prefix 文件名前缀
 * @return 临时文件路径
 */
fs::path createTempFile(const std::string& prefix = "pzip");

/**
 * @brief 格式化文件大小
 * @param size 字节数
 * @return 格式化的字符串（如 "1.5 MB"）
 */
std::string formatSize(uint64_t size);

/**
 * @brief 格式化时间
 * @param seconds 秒数
 * @return 格式化的字符串（如 "1m 30s"）
 */
std::string formatTime(double seconds);

} // namespace utils

} // namespace pzip


