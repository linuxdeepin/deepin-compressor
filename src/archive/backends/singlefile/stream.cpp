// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stream.h"

#include <QFile>

#include <bzlib.h>
#include <lzma.h>
#include <zlib.h>

namespace {

constexpr int kInputBufferSize = 64 * 1024;

} // namespace

struct Decoder::Private {
    QFile input;
    Codec codec = Codec::Gzip;
    bool finished = false;

    QByteArray inBuf;
    int inLen = 0;
    int inPos = 0;

    z_stream z {};
    bool zReady = false;

    bz_stream bz {};
    bool bzReady = false;

    lzma_stream lz = LZMA_STREAM_INIT;
    bool lzReady = false;

    // 从文件补充输入；返回 false 表示已无更多输入
    bool fill()
    {
        inLen = static_cast<int>(input.read(inBuf.data(), inBuf.size()));
        inPos = 0;
        return inLen > 0;
    }
};

Decoder::Decoder()
    : d(new Private)
{
}

Decoder::~Decoder()
{
    close();
}

bool Decoder::open(const QString &path, Codec codec)
{
    d->codec = codec;
    d->input.setFileName(path);
    if (!d->input.open(QIODevice::ReadOnly))
        return false;

    d->inBuf.resize(kInputBufferSize);

    switch (codec) {
    case Codec::Gzip:
        d->z = z_stream {};
        // 15 + 16：gzip 头
        if (inflateInit2(&d->z, 15 + 16) != Z_OK)
            return false;
        d->zReady = true;
        break;
    case Codec::Bzip2:
        d->bz = bz_stream {};
        if (BZ2_bzDecompressInit(&d->bz, 0, 0) != BZ_OK)
            return false;
        d->bzReady = true;
        break;
    case Codec::Xz:
        if (lzma_stream_decoder(&d->lz, UINT64_MAX, 0) != LZMA_OK)
            return false;
        d->lzReady = true;
        break;
    case Codec::Lzma:
        if (lzma_alone_decoder(&d->lz, UINT64_MAX) != LZMA_OK)
            return false;
        d->lzReady = true;
        break;
    }
    return true;
}

qint64 Decoder::read(char *out, qint64 capacity)
{
    if (!d || d->finished || capacity <= 0)
        return 0;

    switch (d->codec) {
    case Codec::Gzip: {
        d->z.next_out = reinterpret_cast<Bytef *>(out);
        d->z.avail_out = static_cast<uInt>(capacity);

        while (d->z.avail_out > 0) {
            if (d->z.avail_in == 0) {
                if (!d->fill()) {
                    // 输入耗尽但未见到流结束：按结束处理
                    d->finished = true;
                    break;
                }
                d->z.next_in = reinterpret_cast<Bytef *>(d->inBuf.data());
                d->z.avail_in = static_cast<uInt>(d->inLen);
            }

            const int rc = inflate(&d->z, Z_NO_FLUSH);
            if (rc == Z_STREAM_END) {
                d->finished = true;
                break;
            }
            if (rc != Z_OK && rc != Z_BUF_ERROR)
                return -1;
            if (d->z.avail_in == 0 && d->z.avail_out == static_cast<uInt>(capacity))
                continue;
        }
        return static_cast<qint64>(capacity) - d->z.avail_out;
    }

    case Codec::Bzip2: {
        d->bz.next_out = out;
        d->bz.avail_out = static_cast<unsigned>(capacity);

        while (d->bz.avail_out > 0) {
            if (d->bz.avail_in == 0) {
                if (!d->fill()) {
                    d->finished = true;
                    break;
                }
                d->bz.next_in = d->inBuf.data();
                d->bz.avail_in = static_cast<unsigned>(d->inLen);
            }

            const int rc = BZ2_bzDecompress(&d->bz);
            if (rc == BZ_STREAM_END) {
                d->finished = true;
                break;
            }
            if (rc != BZ_OK)
                return -1;
        }
        return static_cast<qint64>(capacity) - d->bz.avail_out;
    }

    case Codec::Xz:
    case Codec::Lzma: {
        d->lz.next_out = reinterpret_cast<uint8_t *>(out);
        d->lz.avail_out = static_cast<size_t>(capacity);

        while (d->lz.avail_out > 0) {
            if (d->lz.avail_in == 0) {
                if (!d->fill()) {
                    d->finished = true;
                    break;
                }
                d->lz.next_in = reinterpret_cast<const uint8_t *>(d->inBuf.constData());
                d->lz.avail_in = static_cast<size_t>(d->inLen);
            }

            const lzma_ret rc = lzma_code(&d->lz, LZMA_RUN);
            if (rc == LZMA_STREAM_END) {
                d->finished = true;
                break;
            }
            if (rc != LZMA_OK && rc != LZMA_BUF_ERROR)
                return -1;
            if (rc == LZMA_BUF_ERROR && d->lz.avail_in == 0)
                continue;
        }
        return static_cast<qint64>(capacity) - static_cast<qint64>(d->lz.avail_out);
    }
    }

    return 0;
}

void Decoder::close()
{
    if (!d)
        return;

    if (d->zReady) {
        inflateEnd(&d->z);
        d->zReady = false;
    }
    if (d->bzReady) {
        BZ2_bzDecompressEnd(&d->bz);
        d->bzReady = false;
    }
    if (d->lzReady) {
        lzma_end(&d->lz);
        d->lzReady = false;
    }
    if (d->input.isOpen())
        d->input.close();
}
