// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "common.h"
#include "encoding_detector.h"

#include <gtest/gtest.h>

#include <cstddef>

namespace {

encdet::Result detectBytes(const char *data, std::size_t size)
{
    return encdet::detect(data, size);
}

} // namespace

TEST(EncodingDetectorTest, DetectsUtf8Chinese)
{
    const char data[] = {char(0xe4), char(0xb8), char(0xad), char(0xe6), char(0x96),
                         char(0x87), char(0xe6), char(0xb5), char(0x8b), char(0xe8),
                         char(0xaf), char(0x95)};
    const encdet::Result result = detectBytes(data, sizeof(data));
    EXPECT_EQ(result.encoding, "UTF-8");
    EXPECT_GT(result.confidence, 0.5f);
}

TEST(EncodingDetectorTest, DetectsGbkChinese)
{
    const char data[] = {char(0xd6), char(0xd0), char(0xce), char(0xc4),
                         char(0xb2), char(0xe2), char(0xca), char(0xd4)};
    const encdet::Result result = detectBytes(data, sizeof(data));
    EXPECT_EQ(result.encoding, "gb18030");
    EXPECT_GT(result.confidence, 0.5f);
}

TEST(EncodingDetectorTest, MatchesLegacyBig5ShortSampleQuirk)
{
    // 旧实现对短 Big5 样本也判为 gb18030；这里验证行为一致。
    const char data[] = {char(0xa4), char(0xa4), char(0xa4), char(0xe5),
                         char(0xb4), char(0xfa), char(0xb8), char(0xd5)};
    const encdet::Result result = detectBytes(data, sizeof(data));
    EXPECT_EQ(result.encoding, "gb18030");
    EXPECT_GT(result.confidence, 0.5f);
}

TEST(EncodingDetectorTest, DetectsLongBig5Chinese)
{
    const char data[] = {char(0xc1), char(0x63), char(0xc5), char(0xe9), char(0xa4), char(0xa4),
                         char(0xa4), char(0xe5), char(0xb4), char(0xfa), char(0xb8), char(0xd5),
                         char(0xbd), char(0x64), char(0xa8), char(0xd2), char(0xb3), char(0x6f),
                         char(0xac), char(0x4f), char(0xa4), char(0x40), char(0xac), char(0x71),
                         char(0xa4), char(0xf1), char(0xb8), char(0xfb), char(0xaa), char(0xf8),
                         char(0xaa), char(0xba), char(0xa4), char(0xe5), char(0xa6), char(0x72),
                         char(0xa4), char(0xba), char(0xae), char(0x65), char(0xa5), char(0xce),
                         char(0xa9), char(0xf3), char(0xbd), char(0x73), char(0xbd), char(0x58),
                         char(0xb0), char(0xbb), char(0xb4), char(0xfa)};
    const encdet::Result result = detectBytes(data, sizeof(data));
    EXPECT_EQ(result.encoding, "Big5");
    EXPECT_GT(result.confidence, 0.5f);
}

TEST(EncodingDetectorTest, DetectsShiftJis)
{
    const char data[] = {char(0x93), char(0xfa), char(0x96), char(0x7b),
                         char(0x8c), char(0xea), char(0x83), char(0x65),
                         char(0x83), char(0x58), char(0x83), char(0x67)};
    const encdet::Result result = detectBytes(data, sizeof(data));
    EXPECT_EQ(result.encoding, "Shift_JIS");
    EXPECT_GT(result.confidence, 0.5f);
}

TEST(EncodingDetectorTest, DetectsEucKr)
{
    const char data[] = {char(0xc7), char(0xd1), char(0xb1), char(0xb9), char(0xbe),
                         char(0xee), char(0x20), char(0xc5), char(0xd7), char(0xbd),
                         char(0xba), char(0xc6), char(0xae)};
    const encdet::Result result = detectBytes(data, sizeof(data));
    EXPECT_EQ(result.encoding, "EUC-KR");
    EXPECT_GT(result.confidence, 0.5f);
}

TEST(EncodingDetectorTest, DetectsAscii)
{
    const char data[] = "hello world";
    const encdet::Result result = detectBytes(data, sizeof(data) - 1);
    EXPECT_EQ(result.encoding, "ASCII");
}

TEST(CommonEncodingTest, ChartDetIntegrationUsesOwnDetector)
{
    Common common;
    const char data[] = {char(0xd6), char(0xd0), char(0xce), char(0xc4),
                         char(0xb2), char(0xe2), char(0xca), char(0xd4)};
    QString encoding;
    float confidence = 0.0f;
    EXPECT_EQ(common.ChartDet_DetectingTextCoding(data, encoding, confidence), 0);
    EXPECT_EQ(encoding, QStringLiteral("gb18030"));
    EXPECT_GT(confidence, 0.5f);
}
