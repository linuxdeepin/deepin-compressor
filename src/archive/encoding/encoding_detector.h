// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DC_ENCODING_DETECTOR_H
#define DC_ENCODING_DETECTOR_H

#include <cstddef>
#include <string>
#include <vector>

namespace encdet {

struct Result
{
    std::string encoding;   // 判定结果，空表示"判不出"
    float confidence = 0.0f;
};

// 依次尝试 GB18030/GBK、Big5、Shift_JIS、EUC-KR，取置信度最高且超过阈值者
Result detect(const char *data, size_t size);

} // namespace encdet

#endif // DC_ENCODING_DETECTOR_H
