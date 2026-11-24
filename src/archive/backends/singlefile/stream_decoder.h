// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef ARCHIVE_SINGLEFILE_STREAM_DECODER_H
#define ARCHIVE_SINGLEFILE_STREAM_DECODER_H

#include <QFile>
#include <QString>

namespace dcarchive {

enum class SingleFileCodec {
    Gzip,
    Bzip2,
    Xz,
    Lzma,
};

// 单文件压缩流的顺序解码器。
// read() 返回 >0 为字节数，0 为结束，-1 为错误。
class StreamDecoder
{
public:
    StreamDecoder();
    ~StreamDecoder();

    StreamDecoder(const StreamDecoder &) = delete;
    StreamDecoder &operator=(const StreamDecoder &) = delete;

    bool open(const QString &path, SingleFileCodec codec);
    qint64 read(char *out, qint64 capacity);
    void close();

private:
    struct State;
    State *m_state = nullptr;
    QFile m_input;
};

} // namespace dcarchive

#endif // ARCHIVE_SINGLEFILE_STREAM_DECODER_H
