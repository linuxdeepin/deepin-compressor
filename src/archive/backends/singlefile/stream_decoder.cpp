// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "stream_decoder.h"

#include <bzlib.h>
#include <lzma.h>
#include <zlib.h>

#include <QByteArray>

namespace dcarchive {
namespace {

constexpr int kInputChunk = 16 * 1024;

} // namespace

struct StreamDecoder::State {
    SingleFileCodec codec = SingleFileCodec::Gzip;
    z_stream z = {};
    bz_stream bz = {};
    lzma_stream lz = LZMA_STREAM_INIT;
    bool initialized = false;
    bool inputEof = false;
    bool finished = false;
    QByteArray input;
    int inputPos = 0;
    bool zOwned = false;
    bool bzOwned = false;
    bool lzOwned = false;
};

StreamDecoder::StreamDecoder()
    : m_state(new State)
{
}

StreamDecoder::~StreamDecoder()
{
    close();
    delete m_state;
}

bool StreamDecoder::open(const QString &path, SingleFileCodec codec)
{
    close();

    m_input.setFileName(path);
    if (!m_input.open(QIODevice::ReadOnly))
        return false;

    m_state->codec = codec;
    m_state->finished = false;
    m_state->inputEof = false;
    m_state->input.clear();
    m_state->inputPos = 0;

    if (codec == SingleFileCodec::Gzip) {
        m_state->z = {};
        if (inflateInit2(&m_state->z, 15 + 16) != Z_OK)
            return false;
        m_state->zOwned = true;
    } else if (codec == SingleFileCodec::Bzip2) {
        m_state->bz = {};
        if (BZ2_bzDecompressInit(&m_state->bz, 0, 0) != BZ_OK)
            return false;
        m_state->bzOwned = true;
    } else {
        m_state->lz = LZMA_STREAM_INIT;
        const lzma_ret ret = codec == SingleFileCodec::Xz
                ? lzma_stream_decoder(&m_state->lz, UINT64_MAX, 0)
                : lzma_alone_decoder(&m_state->lz, UINT64_MAX);
        if (ret != LZMA_OK)
            return false;
        m_state->lzOwned = true;
    }

    m_state->initialized = true;
    return true;
}

qint64 StreamDecoder::read(char *out, qint64 capacity)
{
    if (!m_state->initialized || m_state->finished || capacity <= 0)
        return m_state->finished ? 0 : -1;

    while (true) {
        if (m_state->inputPos >= m_state->input.size() && !m_state->inputEof) {
            m_state->input = m_input.read(kInputChunk);
            m_state->inputPos = 0;
            if (m_state->input.isEmpty())
                m_state->inputEof = true;
        }

        if (m_state->codec == SingleFileCodec::Gzip) {
            m_state->z.next_in = reinterpret_cast<Bytef *>(
                    const_cast<char *>(m_state->input.constData() + m_state->inputPos));
            m_state->z.avail_in = static_cast<uInt>(m_state->input.size() - m_state->inputPos);
            m_state->z.next_out = reinterpret_cast<Bytef *>(out);
            m_state->z.avail_out = static_cast<uInt>(capacity);

            const int ret = inflate(&m_state->z, Z_NO_FLUSH);
            m_state->inputPos = m_state->input.size() - int(m_state->z.avail_in);
            const qint64 produced = capacity - qint64(m_state->z.avail_out);

            if (ret == Z_STREAM_END) {
                m_state->finished = true;
                return produced;
            }
            if (ret != Z_OK && ret != Z_BUF_ERROR)
                return -1;
            if (produced > 0)
                return produced;
            if (m_state->inputEof && m_state->z.avail_in == 0) {
                // 与旧实现一致：输入耗尽但未见结束标志时按流结束处理。
                m_state->finished = true;
                return 0;
            }
        } else if (m_state->codec == SingleFileCodec::Bzip2) {
            m_state->bz.next_in = const_cast<char *>(m_state->input.constData() + m_state->inputPos);
            m_state->bz.avail_in = static_cast<unsigned int>(m_state->input.size() - m_state->inputPos);
            m_state->bz.next_out = out;
            m_state->bz.avail_out = static_cast<unsigned int>(capacity);

            const int ret = BZ2_bzDecompress(&m_state->bz);
            m_state->inputPos = m_state->input.size() - int(m_state->bz.avail_in);
            const qint64 produced = capacity - qint64(m_state->bz.avail_out);

            if (ret == BZ_STREAM_END) {
                m_state->finished = true;
                return produced;
            }
            if (ret != BZ_OK && ret != BZ_OUTBUFF_FULL)
                return -1;
            if (produced > 0)
                return produced;
            if (m_state->inputEof && m_state->bz.avail_in == 0) {
                m_state->finished = true;
                return 0;
            }
        } else {
            m_state->lz.next_in = reinterpret_cast<const uint8_t *>(
                    m_state->input.constData() + m_state->inputPos);
            m_state->lz.avail_in = static_cast<size_t>(m_state->input.size() - m_state->inputPos);
            m_state->lz.next_out = reinterpret_cast<uint8_t *>(out);
            m_state->lz.avail_out = static_cast<size_t>(capacity);

            const lzma_ret ret = lzma_code(&m_state->lz, LZMA_RUN);
            m_state->inputPos = m_state->input.size() - int(m_state->lz.avail_in);
            const qint64 produced = capacity - qint64(m_state->lz.avail_out);

            if (ret == LZMA_STREAM_END) {
                m_state->finished = true;
                return produced;
            }
            if (ret != LZMA_OK)
                return -1;
            if (produced > 0)
                return produced;
            if (m_state->inputEof && m_state->lz.avail_in == 0) {
                m_state->finished = true;
                return 0;
            }
        }
    }
}

void StreamDecoder::close()
{
    if (!m_state)
        return;

    if (m_state->zOwned) {
        inflateEnd(&m_state->z);
        m_state->zOwned = false;
    }
    if (m_state->bzOwned) {
        BZ2_bzDecompressEnd(&m_state->bz);
        m_state->bzOwned = false;
    }
    if (m_state->lzOwned) {
        lzma_end(&m_state->lz);
        m_state->lzOwned = false;
    }
    if (m_input.isOpen())
        m_input.close();
    m_state->initialized = false;
    m_state->finished = false;
    m_state->input.clear();
    m_state->inputPos = 0;
    m_state->inputEof = false;
}

} // namespace dcarchive
