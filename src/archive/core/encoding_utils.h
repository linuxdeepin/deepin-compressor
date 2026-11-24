// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_ENCODING_UTILS_H
#define ARCHIVE_ENCODING_UTILS_H

#include <QString>

namespace dcarchive {

// 把归档中的原始条目名字节转换为 Unicode：UTF-8/ASCII 直接解码，
// 其他编码使用自研探测器结果选择 codec。
QString decodeArchiveName(const char *data, int size);

} // namespace dcarchive

#endif // ARCHIVE_ENCODING_UTILS_H
