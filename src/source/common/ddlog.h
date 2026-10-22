// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DDLOG_H
#define DDLOG_H

#include <QDebug>
#include <QLoggingCategory>

// 项目统一日志分类：归档管理器主程序。
// 分类名以 org.deepin.compressor 为前缀，供动态日志规则按
// org.deepin.compressor* 通配过滤（调试模式分级开关依赖分类）。
inline Q_LOGGING_CATEGORY(appLog, "org.deepin.compressor")

#endif   // DDLOG_H
