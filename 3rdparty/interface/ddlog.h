// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef COMPRESSOR_INTERFACE_DDLOG_H
#define COMPRESSOR_INTERFACE_DDLOG_H

#include <QDebug>
#include <QLoggingCategory>

// compressor-interface 库统一日志分类（libarchive 插件亦复用）。
// 分类名挂在 org.deepin.compressor 前缀下，随宿主进程的日志规则生效，
// 便于通过 org.deepin.compressor* 通配与主程序日志一起开关。
inline Q_LOGGING_CATEGORY(ifaceLog, "org.deepin.compressor.interface")

#endif   // COMPRESSOR_INTERFACE_DDLOG_H
