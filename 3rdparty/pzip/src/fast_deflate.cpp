// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

/**
 * @file fast_deflate.cpp
 * @brief 高性能 DEFLATE 压缩器实现
 * 
 * 直接参考 klauspost/compress (Go) 官方代码翻译
 * 优化版本：添加 Level1 快速编码器
 */

#include "pzip/fast_deflate.h"
#include <cstring>
#include <algorithm>
#include <limits>

namespace pzip {

// C++17 兼容性
#if defined(__GNUC__) || defined(__clang__)
#define CTZ64(x) __builtin_ctzll(x)
#define CLZ32(x) __builtin_clz(x)
#else
inline int CTZ64(uint64_t x) {
    if (x == 0) return 64;
    int n = 0;
    if ((x & 0xFFFFFFFF) == 0) { n += 32; x >>= 32; }
    if ((x & 0xFFFF) == 0) { n += 16; x >>= 16; }
    if ((x & 0xFF) == 0) { n += 8; x >>= 8; }
    if ((x & 0xF) == 0) { n += 4; x >>= 4; }
    if ((x & 0x3) == 0) { n += 2; x >>= 2; }
    if ((x & 0x1) == 0) { n += 1; }
    return n;
}
inline int CLZ32(uint32_t x) {
    if (x == 0) return 32;
    int n = 0;
    if ((x & 0xFFFF0000) == 0) { n += 16; x <<= 16; }
    if ((x & 0xFF000000) == 0) { n += 8; x <<= 8; }
    if ((x & 0xF0000000) == 0) { n += 4; x <<= 4; }
    if ((x & 0xC0000000) == 0) { n += 2; x <<= 2; }
    if ((x & 0x80000000) == 0) { n += 1; }
    return n;
}
#endif

// ============================================================================
// 静态表 - 仅用于 HuffmanBitWriter
// ============================================================================

static const uint8_t lengthExtraBits[32] = {
    0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2, 2,
    3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0, 0, 0, 0,
};

static const uint8_t lengthBase[32] = {
    0, 1, 2, 3, 4, 5, 6, 7, 8, 10, 12, 14, 16, 20, 24, 28,
    32, 40, 48, 56, 64, 80, 96, 112, 128, 160, 192, 224, 255, 0, 0, 0,
};

static const int8_t offsetExtraBits[32] = {
    0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6, 6,
    7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13, 14, 14,
};

static const uint32_t offsetBase[32] = {
    0x000000, 0x000001, 0x000002, 0x000003, 0x000004,
    0x000006, 0x000008, 0x00000c, 0x000010, 0x000018,
    0x000020, 0x000030, 0x000040, 0x000060, 0x000080,
    0x0000c0, 0x000100, 0x000180, 0x000200, 0x000300,
    0x000400, 0x000600, 0x000800, 0x000c00, 0x001000,
    0x001800, 0x002000, 0x003000, 0x004000, 0x006000,
    0x008000, 0x00c000,
};

static uint32_t offsetCombined[32] = {};

static struct StaticInit {
    StaticInit() {
        for (int i = 0; i < 32; ++i) {
            if (offsetExtraBits[i] == 0 || offsetBase[i] > 0x006000) continue;
            offsetCombined[i] = static_cast<uint32_t>(offsetExtraBits[i]) | (offsetBase[i] << 8);
        }
    }
} staticInit;

// ============================================================================
// 内联函数
// ============================================================================

PZIP_FORCE_INLINE uint16_t reverseBits(uint16_t number, uint8_t bitLength) {
    uint16_t result = 0;
    for (int i = 0; i < bitLength; ++i) {
        result = (result << 1) | (number & 1);
        number >>= 1;
    }
    return result;
}

// 快速字符串匹配 - 使用 64 位比较
PZIP_FORCE_INLINE PZIP_HOT int matchLen(const uint8_t* a, const uint8_t* b, size_t maxLen) {
    int n = 0;
    size_t left = maxLen;
    
    // 8 字节对齐快速比较
    while (left >= 8) {
        uint64_t va = load64(a, n);
        uint64_t vb = load64(b, n);
        uint64_t diff = va ^ vb;
        if (PZIP_UNLIKELY(diff != 0)) {
            return n + CTZ64(diff) / 8;
        }
        n += 8;
        left -= 8;
    }
    
    // 处理剩余字节
    while (n < static_cast<int>(maxLen) && a[n] == b[n]) {
        n++;
    }
    return n;
}

// ============================================================================
// FastGen 实现
// ============================================================================

int32_t FastGen::addBlock(const uint8_t* src, size_t len) {
    if (hist_.size() + len > hist_.capacity()) {
        if (hist_.capacity() == 0) {
            hist_.reserve(ALLOC_HISTORY);
        } else {
            int32_t offset = static_cast<int32_t>(hist_.size()) - MAX_MATCH_OFFSET;
            if (offset > 0) {
                std::memmove(hist_.data(), hist_.data() + offset, MAX_MATCH_OFFSET);
                cur_ += offset;
                hist_.resize(MAX_MATCH_OFFSET);
            }
        }
    }
    int32_t s = static_cast<int32_t>(hist_.size());
    hist_.insert(hist_.end(), src, src + len);
    return s;
}

void FastGen::reset() {
    if (hist_.capacity() < ALLOC_HISTORY) {
        hist_.reserve(ALLOC_HISTORY);
    }
    cur_ += MAX_MATCH_OFFSET + static_cast<int32_t>(hist_.size());
    hist_.clear();
}

// ============================================================================
// FastEncL1 实现 - Level 1 快速编码器
// ============================================================================

void FastEncL1::reset() {
    FastGen::reset();
    table_.fill({});
}

void PZIP_HOT FastEncL1::encode(Tokens* dst, const uint8_t* src, size_t len) {
    constexpr int inputMargin = 11;
    constexpr int minNonLiteralBlockSize = 13;
    constexpr int skipLog = 5;
    constexpr int doEvery = 2;
    
    // 防止溢出
    constexpr int32_t bufferReset = 0x7FFFFFFF - static_cast<int32_t>(ALLOC_HISTORY) - static_cast<int32_t>(MAX_STORE_BLOCK_SIZE) - 1;
    if (cur_ >= bufferReset) {
        if (hist_.empty()) {
            table_.fill({});
            cur_ = MAX_MATCH_OFFSET;
        } else {
            int32_t minOff = cur_ + static_cast<int32_t>(hist_.size()) - MAX_MATCH_OFFSET;
            for (auto& e : table_) {
                e.offset = (e.offset <= minOff) ? 0 : (e.offset - cur_ + MAX_MATCH_OFFSET);
            }
            cur_ = MAX_MATCH_OFFSET;
        }
    }
    
    int32_t s = addBlock(src, len);
    
    if (static_cast<int>(len) < minNonLiteralBlockSize) {
        dst->n = static_cast<uint16_t>(len);
        return;
    }
    
    const uint8_t* data = hist_.data();
    int32_t nextEmit = s;
    int32_t sLimit = static_cast<int32_t>(hist_.size()) - inputMargin;
    
    uint64_t cv = load64(data, s);
    
    while (true) {
        int32_t nextS = s;
        int32_t t;
        
        // 查找匹配
        while (true) {
            uint32_t nextHash = hash5(cv);
            TableEntry candidate = table_[nextHash];
            nextS = s + doEvery + (s - nextEmit) / (1 << skipLog);
            
            if (PZIP_UNLIKELY(nextS > sLimit)) {
                goto emitRemainder;
            }
            
            uint64_t now = load64(data, nextS);
            table_[nextHash] = {s + cur_};
            uint32_t nextHash2 = hash5(now);
            t = candidate.offset - cur_;
            
            if (s - t < MAX_MATCH_OFFSET && static_cast<uint32_t>(cv) == load32(data, t)) {
                table_[nextHash2] = {nextS + cur_};
                break;
            }
            
            // 再试一次
            cv = now;
            s = nextS;
            nextS++;
            candidate = table_[nextHash2];
            now >>= 8;
            table_[nextHash2] = {s + cur_};
            
            t = candidate.offset - cur_;
            if (s - t < MAX_MATCH_OFFSET && static_cast<uint32_t>(cv) == load32(data, t)) {
                table_[hash5(now)] = {nextS + cur_};
                break;
            }
            cv = now;
            s = nextS;
        }
        
        // 内层匹配循环 - 持续处理连续的匹配
        for (;;) {
            size_t maxLen = std::min(hist_.size() - s - 4, MAX_MATCH_LENGTH - 4);
            int32_t l = matchLen(data + s + 4, data + t + 4, maxLen) + 4;
            
            // 向后扩展
            while (t > 0 && s > nextEmit && data[t - 1] == data[s - 1]) {
                s--; t--; l++;
            }
            
            // 输出字面量 - 内联版本
            for (int32_t i = nextEmit; i < s; ++i) {
                dst->tokens[dst->n] = makeLiteralToken(data[i]);
                dst->litHist[data[i]]++;
                dst->n++;
            }
            
            // 输出匹配 - 内联版本
            dst->addMatchLong(l, static_cast<uint32_t>(s - t - 1));
            s += l;
            nextEmit = s;
            
            if (nextS >= s) s = nextS + 1;
            
            if (PZIP_UNLIKELY(s >= sLimit)) {
                if (s + 8 < static_cast<int32_t>(hist_.size())) {
                    cv = load64(data, s);
                    table_[hash5(cv)] = {s + cur_};
                }
                goto emitRemainder;
            }
            
            // 更新哈希表并查找下一个匹配
            uint64_t x = load64(data, s - 2);
            int32_t o = cur_ + s - 2;
            table_[hash5(x)] = {o};
            x >>= 16;
            TableEntry candidate = table_[hash5(x)];
            table_[hash5(x)] = {o + 2};
            
            t = candidate.offset - cur_;
            if (s - t > MAX_MATCH_OFFSET || static_cast<uint32_t>(x) != load32(data, t)) {
                // 没有找到下一个有效匹配，退出内层循环
                cv = x >> 8;
                s++;
                break;
            }
            // 找到了有效匹配，继续内层循环处理
        }
    }
    
emitRemainder:
    if (nextEmit < static_cast<int32_t>(hist_.size())) {
        if (dst->n == 0) return;
        for (int32_t i = nextEmit; i < static_cast<int32_t>(hist_.size()); ++i) {
            dst->tokens[dst->n] = makeLiteralToken(data[i]);
            dst->litHist[data[i]]++;
            dst->n++;
        }
    }
}

// ============================================================================
// FastEncL4 实现 - Level 4 编码器
// ============================================================================

void FastEncL4::reset() {
    FastGen::reset();
    table_.fill({});
    bTable_.fill({});
}

void PZIP_HOT FastEncL4::encode(Tokens* dst, const uint8_t* src, size_t len) {
    constexpr int inputMargin = 11;
    constexpr int minNonLiteralBlockSize = 13;
    constexpr int skipLog = 6;
    constexpr int doEvery = 1;
    
    constexpr int32_t bufferReset = 0x7FFFFFFF - static_cast<int32_t>(ALLOC_HISTORY) - static_cast<int32_t>(MAX_STORE_BLOCK_SIZE) - 1;
    if (cur_ >= bufferReset) {
        if (hist_.empty()) {
            table_.fill({});
            bTable_.fill({});
            cur_ = MAX_MATCH_OFFSET;
        } else {
            int32_t minOff = cur_ + static_cast<int32_t>(hist_.size()) - MAX_MATCH_OFFSET;
            for (auto& e : table_) {
                e.offset = (e.offset <= minOff) ? 0 : (e.offset - cur_ + MAX_MATCH_OFFSET);
            }
            for (auto& e : bTable_) {
                e.offset = (e.offset <= minOff) ? 0 : (e.offset - cur_ + MAX_MATCH_OFFSET);
            }
            cur_ = MAX_MATCH_OFFSET;
        }
    }
    
    int32_t s = addBlock(src, len);
    
    if (static_cast<int>(len) < minNonLiteralBlockSize) {
        dst->n = static_cast<uint16_t>(len);
        return;
    }
    
    const uint8_t* data = hist_.data();
    int32_t nextEmit = s;
    int32_t sLimit = static_cast<int32_t>(hist_.size()) - inputMargin;
    
    uint64_t cv = load64(data, s);
    
    while (true) {
        int32_t nextS = s;
        int32_t t;
        
        while (true) {
            uint32_t nextHashS = hash4(static_cast<uint32_t>(cv));
            uint32_t nextHashL = hash7(cv);
            
            s = nextS;
            nextS = s + doEvery + (s - nextEmit) / (1 << skipLog);
            if (PZIP_UNLIKELY(nextS > sLimit)) goto emitRemainder;
            
            TableEntry sCandidate = table_[nextHashS];
            TableEntry lCandidate = bTable_[nextHashL];
            uint64_t next = load64(data, nextS);
            TableEntry entry = {s + cur_};
            table_[nextHashS] = entry;
            bTable_[nextHashL] = entry;
            
            t = lCandidate.offset - cur_;
            if (s - t < MAX_MATCH_OFFSET && static_cast<uint32_t>(cv) == load32(data, t)) {
                break;
            }
            
            t = sCandidate.offset - cur_;
            if (s - t < MAX_MATCH_OFFSET && static_cast<uint32_t>(cv) == load32(data, t)) {
                TableEntry lCand2 = bTable_[hash7(next)];
                int32_t lOff = lCand2.offset - cur_;
                if (nextS - lOff < MAX_MATCH_OFFSET && load32(data, lOff) == static_cast<uint32_t>(next)) {
                    size_t max1 = std::min(hist_.size() - s - 4, MAX_MATCH_LENGTH - 4);
                    size_t max2 = std::min(hist_.size() - nextS - 4, MAX_MATCH_LENGTH - 4);
                    int l1 = matchLen(data + s + 4, data + t + 4, max1);
                    int l2 = matchLen(data + nextS + 4, data + nextS - lOff + 4, max2);
                    if (l2 > l1) {
                        s = nextS;
                        t = lCand2.offset - cur_;
                    }
                }
                break;
            }
            cv = next;
        }
        
        {
            size_t maxLen = std::min(hist_.size() - s - 4, MAX_MATCH_LENGTH - 4);
            int32_t l = matchLen(data + s + 4, data + t + 4, maxLen) + 4;
            
            while (t > 0 && s > nextEmit && data[t - 1] == data[s - 1]) {
                s--; t--; l++;
            }
            
            // 输出字面量 - 内联版本
            for (int32_t i = nextEmit; i < s; ++i) {
                dst->tokens[dst->n] = makeLiteralToken(data[i]);
                dst->litHist[data[i]]++;
                dst->n++;
            }
            
            // 输出匹配
            dst->addMatchLong(l, static_cast<uint32_t>(s - t - 1));
            s += l;
            nextEmit = s;
            
            if (nextS >= s) s = nextS + 1;
            
            if (PZIP_UNLIKELY(s >= sLimit)) {
                if (s + 8 < static_cast<int32_t>(hist_.size())) {
                    cv = load64(data, s);
                    table_[hash4(static_cast<uint32_t>(cv))] = {s + cur_};
                    bTable_[hash7(cv)] = {s + cur_};
                }
                goto emitRemainder;
            }
            
            // 更新哈希表
            int32_t i = nextS;
            if (i < s - 1) {
                cv = load64(data, i);
                TableEntry te = {i + cur_};
                TableEntry te2 = {te.offset + 1};
                bTable_[hash7(cv)] = te;
                bTable_[hash7(cv >> 8)] = te2;
                table_[hash4(static_cast<uint32_t>(cv >> 8))] = te2;
                
                for (i += 3; i < s - 1; i += 3) {
                    cv = load64(data, i);
                    te = {i + cur_};
                    te2 = {te.offset + 1};
                    bTable_[hash7(cv)] = te;
                    bTable_[hash7(cv >> 8)] = te2;
                    table_[hash4(static_cast<uint32_t>(cv >> 8))] = te2;
                }
            }
            
            uint64_t x = load64(data, s - 1);
            int32_t o = cur_ + s - 1;
            table_[hash4(static_cast<uint32_t>(x))] = {o};
            bTable_[hash7(x)] = {o};
            cv = x >> 8;
        }
    }
    
emitRemainder:
    if (nextEmit < static_cast<int32_t>(hist_.size())) {
        if (dst->n == 0) return;
        for (int32_t i = nextEmit; i < static_cast<int32_t>(hist_.size()); ++i) {
            dst->tokens[dst->n] = makeLiteralToken(data[i]);
            dst->litHist[data[i]]++;
            dst->n++;
        }
    }
}

// ============================================================================
// HuffmanEncoder 实现
// ============================================================================

HuffmanEncoder::HuffmanEncoder(int size) {
    size_t cap = 1ULL << (32 - CLZ32(static_cast<uint32_t>(size - 1)));
    codes.resize(cap);
    freqCache.resize(size + 1);
}

void HuffmanEncoder::generate(const uint16_t* freq, int numSymbols, int maxBits) {
    std::vector<LiteralNode> list;
    list.reserve(numSymbols + 1);
    
    int count = 0;
    for (int i = 0; i < numSymbols; ++i) {
        if (freq[i] != 0) {
            list.push_back({static_cast<uint16_t>(i), freq[i]});
            count++;
        } else {
            codes[i] = HCode();
        }
    }
    
    if (count <= 2) {
        for (int i = 0; i < count; ++i) {
            codes[list[i].literal].set(static_cast<uint16_t>(i), 1);
        }
        return;
    }
    
    std::sort(list.begin(), list.end(), [](const LiteralNode& a, const LiteralNode& b) {
        return a.freq < b.freq;
    });
    
    bitCounts(list, maxBits);
    assignEncodingAndSize(bitCount.data(), list);
}

void HuffmanEncoder::bitCounts(std::vector<LiteralNode>& list, int maxBits) {
    int n = static_cast<int>(list.size());
    if (maxBits > n - 1) maxBits = n - 1;
    
    bitCount.fill(0);
    int bitsRemaining = 1 << maxBits;
    
    for (int i = n - 1; i >= 0; --i) {
        int bits = 1;
        int needed = 1;
        while (bits < maxBits && needed <= bitsRemaining / 2) {
            bits++;
            needed <<= 1;
        }
        bitCount[bits]++;
        bitsRemaining -= (1 << (maxBits - bits));
    }
}

void HuffmanEncoder::assignEncodingAndSize(const int32_t* bc, std::vector<LiteralNode>& list) {
    uint16_t code = 0;
    int listIdx = static_cast<int>(list.size()) - 1;
    
    for (int bits = 1; bits <= 15 && listIdx >= 0; ++bits) {
        code <<= 1;
        int cnt = bc[bits];
        
        std::vector<uint16_t> symbols;
        for (int i = 0; i < cnt && listIdx >= 0; ++i) {
            symbols.push_back(list[listIdx--].literal);
        }
        std::sort(symbols.begin(), symbols.end());
        
        for (uint16_t sym : symbols) {
            codes[sym].set(reverseBits(code++, bits), bits);
        }
    }
}

int HuffmanEncoder::bitLength(const uint16_t* freq, int numSymbols) const {
    int total = 0;
    for (int i = 0; i < numSymbols && i < static_cast<int>(codes.size()); ++i) {
        if (freq[i] != 0) {
            total += static_cast<int>(freq[i]) * codes[i].len();
        }
    }
    return total;
}

// ============================================================================
// 固定霍夫曼编码表
// ============================================================================

static std::unique_ptr<HuffmanEncoder> createFixedLiteralEncoding() {
    auto h = std::make_unique<HuffmanEncoder>(LITERAL_COUNT);
    for (int ch = 0; ch < LITERAL_COUNT; ++ch) {
        uint16_t bits;
        uint8_t size;
        if (ch < 144) { bits = ch + 48; size = 8; }
        else if (ch < 256) { bits = ch + 400 - 144; size = 9; }
        else if (ch < 280) { bits = ch - 256; size = 7; }
        else { bits = ch + 192 - 280; size = 8; }
        h->codes[ch].set(reverseBits(bits, size), size);
    }
    return h;
}

static std::unique_ptr<HuffmanEncoder> createFixedOffsetEncoding() {
    auto h = std::make_unique<HuffmanEncoder>(30);
    for (int ch = 0; ch < 30; ++ch) {
        h->codes[ch].set(reverseBits(static_cast<uint16_t>(ch), 5), 5);
    }
    return h;
}

static std::unique_ptr<HuffmanEncoder> fixedLiteralEncoding = createFixedLiteralEncoding();
static std::unique_ptr<HuffmanEncoder> fixedOffsetEncoding = createFixedOffsetEncoding();

// ============================================================================
// HuffmanBitWriter 实现
// ============================================================================

HuffmanBitWriter::HuffmanBitWriter() {
    output_.reserve(256 * 1024);
    literalEncoding_ = std::make_unique<HuffmanEncoder>(LITERAL_COUNT);
    offsetEncoding_ = std::make_unique<HuffmanEncoder>(OFFSET_CODE_COUNT);
    reset();
}

void HuffmanBitWriter::reset() {
    output_.clear();
    bits_ = 0;
    nbits_ = 0;
    nbytes_ = 0;
    lastHeader_ = 0;
}

void HuffmanBitWriter::writeOutBits() {
    store64(bytes_.data(), nbytes_, bits_);
    bits_ >>= 48;
    nbits_ -= 48;
    nbytes_ += 6;
    
    if (PZIP_UNLIKELY(nbytes_ >= BUFFER_FLUSH_SIZE)) {
        output_.insert(output_.end(), bytes_.begin(), bytes_.begin() + nbytes_);
        nbytes_ = 0;
    }
}

void HuffmanBitWriter::flush() {
    if (lastHeader_ > 0) {
        writeCode(literalEncoding_->codes[END_BLOCK_MARKER]);
        lastHeader_ = 0;
    }
    
    uint8_t n = nbytes_;
    while (nbits_ != 0) {
        bytes_[n++] = static_cast<uint8_t>(bits_);
        bits_ >>= 8;
        nbits_ = (nbits_ > 8) ? (nbits_ - 8) : 0;
    }
    bits_ = 0;
    
    if (n > 0) {
        output_.insert(output_.end(), bytes_.begin(), bytes_.begin() + n);
    }
    nbytes_ = 0;
}

void HuffmanBitWriter::writeBytes(const uint8_t* bytes, size_t len) {
    uint8_t n = nbytes_;
    while (nbits_ != 0) {
        bytes_[n++] = static_cast<uint8_t>(bits_);
        bits_ >>= 8;
        nbits_ -= 8;
    }
    if (n != 0) {
        output_.insert(output_.end(), bytes_.begin(), bytes_.begin() + n);
    }
    nbytes_ = 0;
    output_.insert(output_.end(), bytes, bytes + len);
}

void HuffmanBitWriter::writeStoredHeader(int length, bool isEof) {
    if (lastHeader_ > 0) {
        writeCode(literalEncoding_->codes[END_BLOCK_MARKER]);
        lastHeader_ = 0;
    }
    
    if (length == 0 && isEof) {
        writeFixedHeader(isEof);
        writeBits(0, 7);
        flush();
        return;
    }
    
    writeBits(isEof ? 1 : 0, 3);
    flush();
    writeBits(length, 16);
    writeBits(~length & 0xFFFF, 16);
}

void HuffmanBitWriter::writeFixedHeader(bool isEof) {
    if (lastHeader_ > 0) {
        writeCode(literalEncoding_->codes[END_BLOCK_MARKER]);
        lastHeader_ = 0;
    }
    writeBits(isEof ? 3 : 2, 3);
}

void HuffmanBitWriter::indexTokens(Tokens* t, bool alwaysEOB) {
    std::memcpy(literalFreq_.data(), t->litHist.data(), 256 * sizeof(uint16_t));
    std::memcpy(literalFreq_.data() + 256, t->extraHist.data(), 32 * sizeof(uint16_t));
    offsetFreq_ = t->offHist;
    if (t->n != 0 && alwaysEOB) {
        literalFreq_[END_BLOCK_MARKER] = 1;
    }
}

void HuffmanBitWriter::generate() {
    literalEncoding_->generate(literalFreq_.data(), LITERAL_COUNT, 15);
    offsetEncoding_->generate(offsetFreq_.data(), OFFSET_CODE_COUNT, 15);
}

int HuffmanBitWriter::extraBitSize() {
    int total = 0;
    for (int i = 0; i < LITERAL_COUNT - 257; ++i) {
        total += static_cast<int>(literalFreq_[257 + i]) * lengthExtraBits[i & 31];
    }
    for (int i = 0; i < OFFSET_CODE_COUNT; ++i) {
        total += static_cast<int>(offsetFreq_[i]) * offsetExtraBits[i & 31];
    }
    return total;
}

int HuffmanBitWriter::fixedSize(int extraBits) {
    return 3 + fixedLiteralEncoding->bitLength(literalFreq_.data(), LITERAL_COUNT) +
           fixedOffsetEncoding->bitLength(offsetFreq_.data(), OFFSET_CODE_COUNT) + extraBits;
}

int HuffmanBitWriter::storedSize(const uint8_t* input, size_t len, bool* storable) {
    *storable = (input != nullptr && len <= MAX_STORE_BLOCK_SIZE);
    return *storable ? (static_cast<int>(len) + 5) * 8 : 0;
}

void PZIP_HOT HuffmanBitWriter::writeTokens(const Token* tokens, size_t n, 
                                             const HCode* leCodes, const HCode* oeCodes) {
    if (n == 0) return;
    
    const HCode* lits = leCodes;
    const HCode* offs = oeCodes;
    const HCode* lengths = leCodes + LENGTH_CODES_START;
    
    uint64_t bits = bits_;
    uint8_t nbits = nbits_;
    uint8_t nbytes = nbytes_;
    
    bool deferEOB = (tokens[n - 1] == static_cast<Token>(END_BLOCK_MARKER));
    if (deferEOB) n--;
    
    for (size_t i = 0; i < n; ++i) {
        Token t = tokens[i];
        
        if (PZIP_LIKELY(t < 256)) {
            HCode c = lits[t];
            bits |= c.code64() << (nbits & 63);
            nbits += c.len();
            if (PZIP_UNLIKELY(nbits >= 48)) {
                store64(bytes_.data(), nbytes, bits);
                bits >>= 48;
                nbits -= 48;
                nbytes += 6;
                if (nbytes >= BUFFER_FLUSH_SIZE) {
                    output_.insert(output_.end(), bytes_.begin(), bytes_.begin() + nbytes);
                    nbytes = 0;
                }
            }
            continue;
        }
        
        uint32_t length = (t >> LENGTH_SHIFT) & 0xFF;
        uint8_t lengthCode = lengthCodes1[length] - 1;
        
        HCode c = lengths[lengthCode];
        bits |= c.code64() << (nbits & 63);
        nbits += c.len();
        if (nbits >= 48) {
            store64(bytes_.data(), nbytes, bits);
            bits >>= 48;
            nbits -= 48;
            nbytes += 6;
            if (nbytes >= BUFFER_FLUSH_SIZE) {
                output_.insert(output_.end(), bytes_.begin(), bytes_.begin() + nbytes);
                nbytes = 0;
            }
        }
        
        if (lengthCode >= 8) {
            uint8_t extraLengthBits = lengthExtraBits[lengthCode];
            int32_t extraLength = static_cast<int32_t>(length) - lengthBase[lengthCode];
            bits |= static_cast<uint64_t>(extraLength) << (nbits & 63);
            nbits += extraLengthBits;
            if (nbits >= 48) {
                store64(bytes_.data(), nbytes, bits);
                bits >>= 48;
                nbits -= 48;
                nbytes += 6;
                if (nbytes >= BUFFER_FLUSH_SIZE) {
                    output_.insert(output_.end(), bytes_.begin(), bytes_.begin() + nbytes);
                    nbytes = 0;
                }
            }
        }
        
        uint32_t offset = t & OFFSET_MASK;
        uint32_t offCode = (offset >> 16) & 31;
        offset &= 0xFFFF; 
        
        c = offs[offCode];
        bits |= c.code64() << (nbits & 63);
        nbits += c.len();
        if (nbits >= 48) {
            store64(bytes_.data(), nbytes, bits);
            bits >>= 48;
            nbits -= 48;
            nbytes += 6;
            if (nbytes >= BUFFER_FLUSH_SIZE) {
                output_.insert(output_.end(), bytes_.begin(), bytes_.begin() + nbytes);
                nbytes = 0;
            }
        }
        
        if (offCode >= 4) {
            uint32_t offsetComb = offsetCombined[offCode];
            bits |= static_cast<uint64_t>((offset - (offsetComb >> 8)) & 0xFFFF) << (nbits & 63);
            nbits += static_cast<uint8_t>(offsetComb);
            if (nbits >= 48) {
                store64(bytes_.data(), nbytes, bits);
                bits >>= 48;
                nbits -= 48;
                nbytes += 6;
                if (nbytes >= BUFFER_FLUSH_SIZE) {
                    output_.insert(output_.end(), bytes_.begin(), bytes_.begin() + nbytes);
                    nbytes = 0;
                }
            }
        }
    }
    
    bits_ = bits;
    nbits_ = nbits;
    nbytes_ = nbytes;
    
    if (deferEOB) {
        writeCode(leCodes[END_BLOCK_MARKER]);
    }
}

void HuffmanBitWriter::writeBlock(Tokens* tokens, bool eof, 
                                   const uint8_t* input, size_t inputLen) {
    tokens->addEOB();
    
    if (lastHeader_ > 0) {
        writeCode(literalEncoding_->codes[END_BLOCK_MARKER]);
        lastHeader_ = 0;
    }
    
    indexTokens(tokens, false);
    generate();
    
    bool storable;
    int ssize = storedSize(input, inputLen, &storable);
    int extraBits = storable ? extraBitSize() : 0;
    int size = fixedSize(extraBits);
    
    if (storable && ssize <= size) {
        writeStoredHeader(static_cast<int>(inputLen), eof);
        writeBytes(input, inputLen);
        return;
    }
    
    writeFixedHeader(eof);
    writeTokens(tokens->tokens.data(), tokens->n, 
                fixedLiteralEncoding->codes.data(), fixedOffsetEncoding->codes.data());
}

void HuffmanBitWriter::writeBlockDynamic(Tokens* tokens, bool eof,
                                          const uint8_t* input, size_t inputLen, bool sync) {
    // 每个块都需要 EOB 标记
    tokens->addEOB();
    
    if (lastHeader_ > 0) {
        writeCode(literalEncoding_->codes[END_BLOCK_MARKER]);
        lastHeader_ = 0;
    }
    
    indexTokens(tokens, true);
    
    bool storable;
    int ssize = storedSize(input, inputLen, &storable);
    int extraBits = storable ? extraBitSize() : 0;
    
    generate();
    int size = fixedSize(extraBits);
    
    if (storable && ssize <= size) {
        writeStoredHeader(static_cast<int>(inputLen), eof);
        writeBytes(input, inputLen);
        return;
    }
    
    writeFixedHeader(eof);
    writeTokens(tokens->tokens.data(), tokens->n, 
                fixedLiteralEncoding->codes.data(), fixedOffsetEncoding->codes.data());
}

// ============================================================================
// FastDeflate 实现
// ============================================================================

FastDeflate::FastDeflate(CompressionLevel level)
    : level_(level)
    , encoderL1_(std::make_unique<FastEncL1>())
    , encoderL4_(std::make_unique<FastEncL4>())
    , writer_(std::make_unique<HuffmanBitWriter>()) {
    // Level 1-3 使用 L1 编码器（最快）
    // Level 4+ 使用 L4 编码器（更好压缩率）
    useL1_ = (static_cast<int>(level) <= 3);
}

void FastDeflate::reset() {
    encoderL1_->reset();
    encoderL4_->reset();
    writer_->reset();
    tokens_.reset();
}

size_t FastDeflate::compress(const uint8_t* input, size_t inputSize, 
                             std::vector<uint8_t>& output) {
    reset();
    
    if (inputSize == 0) {
        writer_->writeStoredHeader(0, true);
        writer_->flush();
        output = std::move(writer_->data());
        return output.size();
    }
    
    size_t pos = 0;
    while (pos < inputSize) {
        size_t blockSize = std::min(inputSize - pos, MAX_STORE_BLOCK_SIZE);
        bool isLast = (pos + blockSize >= inputSize);
        
        tokens_.reset();
        
        if (useL1_) {
            encoderL1_->encode(&tokens_, input + pos, blockSize);
        } else {
            encoderL4_->encode(&tokens_, input + pos, blockSize);
        }
        
        if (tokens_.n == 0 || tokens_.n >= static_cast<uint16_t>(blockSize)) {
            writer_->writeStoredHeader(static_cast<int>(blockSize), isLast);
            writer_->writeBytes(input + pos, blockSize);
        } else {
            writer_->writeBlockDynamic(&tokens_, isLast, input + pos, blockSize, isLast);
        }
        
        pos += blockSize;
    }
    
    writer_->flush();
    output = std::move(writer_->data());
    return output.size();
}

// ============================================================================
// 便捷函数
// ============================================================================

size_t deflateCompress(const uint8_t* input, size_t inputSize,
                       std::vector<uint8_t>& output,
                       CompressionLevel level) {
    FastDeflate deflate(level);
    return deflate.compress(input, inputSize, output);
}

// ============================================================================
// DeflateStream 实现
// ============================================================================

DeflateStream::DeflateStream(CompressionLevel level)
    : deflate_(std::make_unique<FastDeflate>(level)) {
    buffer_.reserve(BUFFER_SIZE);
}

DeflateStream::~DeflateStream() = default;

size_t DeflateStream::write(const uint8_t* data, size_t size) {
    buffer_.insert(buffer_.end(), data, data + size);
    return size;
}

size_t DeflateStream::finish(std::vector<uint8_t>& output) {
    return deflate_->compress(buffer_.data(), buffer_.size(), output);
}

void DeflateStream::reset() {
    buffer_.clear();
    deflate_->reset();
}

} // namespace pzip
