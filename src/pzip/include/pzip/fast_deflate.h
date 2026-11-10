// Copyright (C) 2025 ~ 2026 Uniontech Software Technology Co.,Ltd.
// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

/**
 * @file fast_deflate.hpp
 * @brief 高性能 DEFLATE 压缩器
 * 
 * 参考 klauspost/compress (Go) 官方实现
 * - level1.go: 快速单哈希表匹配算法  
 * - level4.go: 双哈希表匹配算法
 * - huffman_bit_writer.go: 高效霍夫曼编码输出
 */

#include <cstdint>
#include <cstring>
#include <functional>
#include <vector>
#include <array>
#include <algorithm>
#include <memory>
#include <utility>

#if defined(__GNUC__) || defined(__clang__)
#define PZIP_FORCE_INLINE __attribute__((always_inline)) inline
#define PZIP_HOT __attribute__((hot))
#define PZIP_LIKELY(x) __builtin_expect(!!(x), 1)
#define PZIP_UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define PZIP_FORCE_INLINE inline
#define PZIP_HOT
#define PZIP_LIKELY(x) (x)
#define PZIP_UNLIKELY(x) (x)
#endif

namespace pzip {

// 压缩级别
enum class CompressionLevel {
    NoCompression = 0,
    BestSpeed = 1,
    Level2 = 2,
    Level3 = 3,
    Level4 = 4,
    Level5 = 5,
    DefaultCompression = 6,
    Level7 = 7,
    Level8 = 8,
    BestCompression = 9
};

// ============================================================================
// 常量定义 - 来自 klauspost/compress/flate
// ============================================================================

constexpr int TABLE_BITS = 15;
constexpr size_t TABLE_SIZE = 1 << TABLE_BITS;
constexpr int TABLE_SHIFT = 32 - TABLE_BITS;

constexpr size_t BASE_MATCH_LENGTH = 3;
constexpr size_t MAX_MATCH_LENGTH = 258;
constexpr int32_t MAX_MATCH_OFFSET = 1 << 15;

constexpr size_t MAX_STORE_BLOCK_SIZE = 65535;
constexpr size_t ALLOC_HISTORY = MAX_STORE_BLOCK_SIZE * 5;

constexpr uint32_t PRIME_4_BYTES = 2654435761U;
constexpr uint64_t PRIME_5_BYTES = 889523592379ULL;
constexpr uint64_t PRIME_7_BYTES = 58295818150454627ULL;

constexpr int OFFSET_CODE_COUNT = 30;
constexpr int END_BLOCK_MARKER = 256;
constexpr int LENGTH_CODES_START = 257;
constexpr int LITERAL_COUNT = 286;
constexpr int BUFFER_FLUSH_SIZE = 246;

constexpr int LENGTH_SHIFT = 22;
constexpr uint32_t OFFSET_MASK = (1 << LENGTH_SHIFT) - 1;
constexpr uint32_t MATCH_TYPE = 1U << 30;

// ============================================================================
// 静态查找表 - 内联以提高性能
// ============================================================================

// lengthCodes1 表 - 从 klauspost/compress
alignas(64) constexpr uint8_t lengthCodes1[256] = {
    1, 2, 3, 4, 5, 6, 7, 8, 9, 9, 10, 10, 11, 11, 12, 12,
    13, 13, 13, 13, 14, 14, 14, 14, 15, 15, 15, 15, 16, 16, 16, 16,
    17, 17, 17, 17, 17, 17, 17, 17, 18, 18, 18, 18, 18, 18, 18, 18,
    19, 19, 19, 19, 19, 19, 19, 19, 20, 20, 20, 20, 20, 20, 20, 20,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21,
    22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22, 22,
    23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23, 23,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 29,
};

// offsetCodes 表
alignas(64) constexpr uint32_t offsetCodes[256] = {
    0, 1, 2, 3, 4, 4, 5, 5, 6, 6, 6, 6, 7, 7, 7, 7,
    8, 8, 8, 8, 8, 8, 8, 8, 9, 9, 9, 9, 9, 9, 9, 9,
    10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10, 10,
    11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11, 11,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13, 13,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15, 15,
};

// offsetCodes14 表 (用于 offset >= 256)
alignas(64) constexpr uint32_t offsetCodes14[256] = {
    14, 15, 16, 17, 18, 18, 19, 19, 20, 20, 20, 20, 21, 21, 21, 21,
    22, 22, 22, 22, 22, 22, 22, 22, 23, 23, 23, 23, 23, 23, 23, 23,
    24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24, 24,
    25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25, 25,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26, 26,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27, 27,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28, 28,
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
    29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29, 29,
};

// ============================================================================
// 内联辅助函数
// ============================================================================

PZIP_FORCE_INLINE uint32_t load32(const uint8_t* b, size_t i) {
    uint32_t v;
    std::memcpy(&v, b + i, 4);
    return v;
}

PZIP_FORCE_INLINE uint64_t load64(const uint8_t* b, size_t i) {
    uint64_t v;
    std::memcpy(&v, b + i, 8);
    return v;
}

PZIP_FORCE_INLINE void store64(uint8_t* b, size_t i, uint64_t v) {
    std::memcpy(b + i, &v, 8);
}

// 4字节哈希
PZIP_FORCE_INLINE uint32_t hash4(uint32_t u) {
    return (u * PRIME_4_BYTES) >> TABLE_SHIFT;
}

// 5字节哈希 (level1 使用)
PZIP_FORCE_INLINE uint32_t hash5(uint64_t u) {
    return static_cast<uint32_t>(((u << (64 - 40)) * PRIME_5_BYTES) >> (64 - TABLE_BITS));
}

// 7字节哈希 (level4 使用)
PZIP_FORCE_INLINE uint32_t hash7(uint64_t u) {
    return static_cast<uint32_t>(((u << (64 - 56)) * PRIME_7_BYTES) >> (64 - TABLE_BITS));
}

// offsetCode - 内联以避免函数调用
PZIP_FORCE_INLINE uint32_t offsetCode(uint32_t off) {
    if (off < 256) return offsetCodes[off];
    return offsetCodes14[(off >> 7) & 0xFF];
}

// ============================================================================
// Token 类型
// ============================================================================

using Token = uint32_t;

PZIP_FORCE_INLINE Token makeLiteralToken(uint8_t lit) {
    return static_cast<Token>(lit);
}

PZIP_FORCE_INLINE Token makeMatchToken(uint32_t xlength, uint32_t xoffset) {
    return MATCH_TYPE | (xlength << LENGTH_SHIFT) | xoffset;
}

// ============================================================================
// Tokens 容器 - 使用固定大小数组，所有方法内联
// ============================================================================

struct Tokens {
    std::array<uint16_t, 32> extraHist;
    std::array<uint16_t, 32> offHist;
    std::array<uint16_t, 256> litHist;
    uint16_t n = 0;
    std::array<Token, MAX_STORE_BLOCK_SIZE + 1> tokens;
    
    PZIP_FORCE_INLINE void reset() {
        n = 0;
        litHist.fill(0);
        extraHist.fill(0);
        offHist.fill(0);
    }
    
    PZIP_FORCE_INLINE void addLiteral(uint8_t lit) {
        tokens[n] = makeLiteralToken(lit);
        litHist[lit]++;
        n++;
    }
    
    // addMatch - 短匹配 (<=258)
    PZIP_FORCE_INLINE void addMatch(uint32_t xlength, uint32_t xoffset) {
        uint32_t oCode = offsetCode(xoffset);
        xoffset |= oCode << 16;
        extraHist[lengthCodes1[static_cast<uint8_t>(xlength)]]++;
        offHist[oCode & 31]++;
        tokens[n++] = MATCH_TYPE | (xlength << LENGTH_SHIFT) | xoffset;
    }
    
    // addMatchLong - 长匹配，完全内联
    PZIP_FORCE_INLINE void addMatchLong(int32_t xlength, uint32_t xoffset) {
        uint32_t oc = offsetCode(xoffset);
        xoffset |= oc << 16;
        
        while (xlength > 0) {
            int32_t xl = xlength;
            if (xl > 258) {
                xl = (xl > 258 + 3) ? 258 : (258 - 3);
            }
            xlength -= xl;
            xl -= 3; // BASE_MATCH_LENGTH
            
            extraHist[lengthCodes1[static_cast<uint8_t>(xl)]]++;
            offHist[oc & 31]++;
            tokens[n++] = MATCH_TYPE | (static_cast<uint32_t>(xl) << LENGTH_SHIFT) | xoffset;
        }
    }
    
    PZIP_FORCE_INLINE void addEOB() {
        tokens[n++] = END_BLOCK_MARKER;
    }
};

// ============================================================================
// hcode - 霍夫曼编码
// ============================================================================

struct HCode {
    uint32_t value = 0;
    
    HCode() = default;
    HCode(uint16_t code, uint8_t len) : value(len | (static_cast<uint32_t>(code) << 8)) {}
    
    PZIP_FORCE_INLINE uint8_t len() const { return static_cast<uint8_t>(value); }
    PZIP_FORCE_INLINE uint64_t code64() const { return static_cast<uint64_t>(value >> 8); }
    PZIP_FORCE_INLINE bool zero() const { return value == 0; }
    
    void set(uint16_t code, uint8_t len) {
        value = len | (static_cast<uint32_t>(code) << 8);
    }
};

// ============================================================================
// HuffmanEncoder
// ============================================================================

class HuffmanEncoder {
public:
    std::vector<HCode> codes;
    std::array<int32_t, 17> bitCount;
    
    explicit HuffmanEncoder(int size);
    void generate(const uint16_t* freq, int numSymbols, int maxBits);
    int bitLength(const uint16_t* freq, int numSymbols) const;
    
private:
    struct LiteralNode {
        uint16_t literal;
        uint16_t freq;
    };
    std::vector<LiteralNode> freqCache;
    void bitCounts(std::vector<LiteralNode>& list, int maxBits);
    void assignEncodingAndSize(const int32_t* bitCount, std::vector<LiteralNode>& list);
};

// ============================================================================
// HuffmanBitWriter - 高效位输出
// ============================================================================

class HuffmanBitWriter {
public:
    explicit HuffmanBitWriter();
    ~HuffmanBitWriter() = default;
    
    HuffmanBitWriter(const HuffmanBitWriter&) = delete;
    HuffmanBitWriter& operator=(const HuffmanBitWriter&) = delete;
    
    HuffmanBitWriter(HuffmanBitWriter&&) = default;
    HuffmanBitWriter& operator=(HuffmanBitWriter&&) = default;
    
    void reset();
    void flush();
    
    PZIP_FORCE_INLINE void writeBits(int32_t b, uint8_t nb) {
        bits_ |= static_cast<uint64_t>(b) << (nbits_ & 63);
        nbits_ += nb;
        if (PZIP_UNLIKELY(nbits_ >= 48)) {
            writeOutBits();
        }
    }
    
    PZIP_FORCE_INLINE void writeCode(HCode c) {
        bits_ |= c.code64() << (nbits_ & 63);
        nbits_ += c.len();
        if (PZIP_UNLIKELY(nbits_ >= 48)) {
            writeOutBits();
        }
    }
    
    void writeBytes(const uint8_t* bytes, size_t len);
    void writeStoredHeader(int length, bool isEof);
    void writeFixedHeader(bool isEof);
    
    void writeBlock(Tokens* tokens, bool eof, const uint8_t* input, size_t inputLen);
    void writeBlockDynamic(Tokens* tokens, bool eof, const uint8_t* input, size_t inputLen, bool sync);
    void writeBlockHuff(bool eof, const uint8_t* input, size_t inputLen, bool sync);
    
    void writeTokens(const Token* tokens, size_t n, const HCode* leCodes, const HCode* oeCodes);
    
    const std::vector<uint8_t>& data() const { return output_; }
    std::vector<uint8_t>& data() { return output_; }
    
    void setLogNewTablePenalty(int penalty) { logNewTablePenalty_ = penalty; }
    
private:
    void writeOutBits();
    void indexTokens(Tokens* t, bool alwaysEOB);
    void generate();
    int extraBitSize();
    int fixedSize(int extraBits);
    int storedSize(const uint8_t* input, size_t len, bool* storable);
    void histogram(const uint8_t* input, size_t len);
    std::pair<int, int> headerSize();
    void generateCodegen(int numLiterals, int numOffsets, HuffmanEncoder* litEnc, HuffmanEncoder* offEnc);
    int codegens();
    void writeDynamicHeader(int numLiterals, int numOffsets, int numCodegens, bool isEof);
    
    std::vector<uint8_t> output_;
    uint64_t bits_ = 0;
    uint8_t nbits_ = 0;
    uint8_t nbytes_ = 0;
    int lastHeader_ = 0;
    bool lastHuffMan_ = false;
    int logNewTablePenalty_ = 7;
    
    std::array<uint8_t, 256 + 8> bytes_;
    std::array<uint16_t, LENGTH_CODES_START + 32> literalFreq_;
    std::array<uint16_t, 32> offsetFreq_;
    std::array<uint16_t, 19> codegenFreq_;
    std::array<uint8_t, LITERAL_COUNT + OFFSET_CODE_COUNT + 1> codegen_;
    
    std::unique_ptr<HuffmanEncoder> literalEncoding_;
    std::unique_ptr<HuffmanEncoder> offsetEncoding_;
    std::unique_ptr<HuffmanEncoder> tmpLitEncoding_;
    std::unique_ptr<HuffmanEncoder> codegenEncoding_;
};

// ============================================================================
// TableEntry
// ============================================================================

struct TableEntry {
    int32_t offset = 0;
};

// ============================================================================
// FastGen - 快速编码器基类
// ============================================================================

class FastGen {
public:
    FastGen() : cur_(MAX_STORE_BLOCK_SIZE) {
        hist_.reserve(ALLOC_HISTORY);
    }
    virtual ~FastGen() = default;
    
    int32_t addBlock(const uint8_t* src, size_t len);
    virtual void reset();
    
    // 纯虚函数，由子类实现
    virtual void encode(Tokens* dst, const uint8_t* src, size_t len) = 0;
    
protected:
    std::vector<uint8_t> hist_;
    int32_t cur_;
};

// ============================================================================
// FastEncL1 - Level 1 编码器 (最快)
// ============================================================================

class FastEncL1 : public FastGen {
public:
    FastEncL1() { table_.fill({}); }
    
    void encode(Tokens* dst, const uint8_t* src, size_t len) override;
    void reset() override;
    
private:
    std::array<TableEntry, TABLE_SIZE> table_;
};

// ============================================================================
// FastEncL4 - Level 4 编码器 (平衡)
// ============================================================================

class FastEncL4 : public FastGen {
public:
    FastEncL4() {
        table_.fill({});
        bTable_.fill({});
    }
    
    void encode(Tokens* dst, const uint8_t* src, size_t len) override;
    void reset() override;
    
private:
    std::array<TableEntry, TABLE_SIZE> table_;
    std::array<TableEntry, TABLE_SIZE> bTable_;
};

// ============================================================================
// FastDeflate - 高性能压缩器
// ============================================================================

class FastDeflate {
public:
    explicit FastDeflate(CompressionLevel level = CompressionLevel::DefaultCompression);
    ~FastDeflate() = default;
    
    size_t compress(const uint8_t* input, size_t inputSize, std::vector<uint8_t>& output);
    void reset();

private:
    CompressionLevel level_;
    std::unique_ptr<FastEncL1> encoderL1_;
    std::unique_ptr<FastEncL4> encoderL4_;
    std::unique_ptr<HuffmanBitWriter> writer_;
    Tokens tokens_;
    bool useL1_;
};

// ============================================================================
// 便捷函数
// ============================================================================

size_t deflateCompress(const uint8_t* input, size_t inputSize, 
                       std::vector<uint8_t>& output,
                       CompressionLevel level = CompressionLevel::DefaultCompression);

// ============================================================================
// FlateWriter - 流式压缩器（参照 Go klauspost/compress flate.Writer）
// ============================================================================

// 输出回调类型（参照 Go io.Writer）
using WriteFunc = std::function<void(const uint8_t*, size_t)>;

class FlateWriter {
public:
    // 接收输出目标（参照 Go flate.NewWriter(w io.Writer, level int)）
    explicit FlateWriter(WriteFunc output, CompressionLevel level = CompressionLevel::BestSpeed);
    ~FlateWriter() = default;
    
    // 流式写入数据（参照 Go compressor.write）
    size_t write(const uint8_t* data, size_t size);
    
    // 完成压缩（参照 Go compressor.Close）
    void close();
    
    // 重置并设置新的输出目标（参照 Go compressor.Reset）
    void reset(WriteFunc output);

private:
    void storeFast();
    size_t fillBlock(const uint8_t* data, size_t size);
    void flushOutput();
    void forceFlush();
    
    // 获取当前使用的编码器
    FastGen* encoder() const { return useL1_ ? static_cast<FastGen*>(encoderL1_.get()) 
                                             : static_cast<FastGen*>(encoderL4_.get()); }
    
    WriteFunc output_;
    std::vector<uint8_t> window_;
    size_t windowEnd_ = 0;
    
    CompressionLevel level_;
    bool useL1_ = true;  // Level 1-3 使用 L1，Level 4+ 使用 L4
    std::unique_ptr<FastEncL1> encoderL1_;
    std::unique_ptr<FastEncL4> encoderL4_;
    std::unique_ptr<HuffmanBitWriter> writer_;
    Tokens tokens_;
};

} // namespace pzip
