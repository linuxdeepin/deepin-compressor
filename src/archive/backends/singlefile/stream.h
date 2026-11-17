// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef SINGLEFILE_STREAM_H
#define SINGLEFILE_STREAM_H

#include <QString>
#include <QtGlobal>

#include <memory>

enum class Codec {
    Gzip,
    Bzip2,
    Xz,
    Lzma
};

// 单文件压缩流的顺序解码器。
// read() 返回 >0 为实际字节数，0 为流结束，-1 为错误。
// 输入被截断时按流结束处理（与原有实现一致，不视为错误）。
class Decoder
{
public:
    Decoder();
    ~Decoder();

    Decoder(const Decoder &) = delete;
    Decoder &operator=(const Decoder &) = delete;

    bool open(const QString &path, Codec codec);
    qint64 read(char *out, qint64 capacity);
    void close();

private:
    struct Private;
    std::unique_ptr<Private> d;
};

#endif // SINGLEFILE_STREAM_H
