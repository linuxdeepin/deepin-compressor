// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 编码探测对比探针：同一份字节分别喂给 旧 chardet / uchardet / ICU
// 用法: ./probe <file>...
#include <cstdio>
#include <cstring>
#include <string>
#include <vector>

#include "chardet.h"          // 旧实现：3rdparty/ChardetDetector
#include "encoding_detector.h"   // 自有实现：src/archive/api/encoding
#include <uchardet/uchardet.h>
#include <unicode/ucsdet.h>

static std::string readFile(const char *path)
{
    FILE *fp = fopen(path, "rb");
    if (!fp)
        return std::string();
    std::string data;
    char buf[4096];
    size_t n;
    while ((n = fread(buf, 1, sizeof(buf), fp)) > 0)
        data.append(buf, n);
    fclose(fp);
    return data;
}

static std::string oldDetect(const std::string &data, float *confidence)
{
    DetectObj *obj = detect_obj_init();
    if (!obj)
        return "?";
    std::string result = "?";
    // 本仓库这份 chardet 未开 CHARDET_BINARY_SAFE，只有 NUL 结尾版本
    if (detect(data.c_str(), &obj) == CHARDET_SUCCESS) {
        result = obj->encoding ? obj->encoding : "?";
        if (confidence)
            *confidence = obj->confidence;
    }
    detect_obj_free(&obj);
    return result;
}

static std::string uchardetDetect(const std::string &data, float *confidence)
{
    uchardet_t ud = uchardet_new();
    std::string result = "?";
    if (uchardet_handle_data(ud, data.data(), data.size()) == 0) {
        uchardet_data_end(ud);
        const char *cs = uchardet_get_charset(ud);
        result = cs ? cs : "?";
        if (confidence)
            *confidence = -1.0f;   // uchardet 不提供置信度
    }
    uchardet_delete(ud);
    return result;
}

static std::string icuDetect(const std::string &data, float *confidence)
{
    UErrorCode status = U_ZERO_ERROR;
    UCharsetDetector *det = ucsdet_open(&status);
    if (U_FAILURE(status))
        return "?";
    ucsdet_setText(det, data.data(), static_cast<int32_t>(data.size()), &status);
    const UCharsetMatch *match = ucsdet_detect(det, &status);
    std::string result = "?";
    if (U_SUCCESS(status) && match) {
        const char *name = ucsdet_getName(match, &status);
        result = name ? name : "?";
        if (confidence)
            *confidence = static_cast<float>(ucsdet_getConfidence(match, &status));
    }
    ucsdet_close(det);
    return result;
}

int main(int argc, char **argv)
{
    if (argc < 2) {
        fprintf(stderr, "用法: %s <file>...\n", argv[0]);
        return 2;
    }

    printf("%-28s %-20s %-20s %-20s %-20s\n", "样本", "旧 chardet", "自研(新)", "uchardet", "ICU");
    for (int i = 1; i < argc; ++i) {
        const std::string data = readFile(argv[i]);
        if (data.empty()) {
            printf("%-28s (读不到)\n", argv[i]);
            continue;
        }

        float c1 = 0, c2 = 0, c3 = 0;
        const std::string r1 = oldDetect(data, &c1);
        const std::string r2 = uchardetDetect(data, &c2);
        const std::string r3 = icuDetect(data, &c3);
        const encdet::Result own = encdet::detect(data.data(), data.size());

        char col1[64], colOwn[64], col2[64], col3[64];
        snprintf(col1, sizeof(col1), "%s(%.2f)", r1.c_str(), c1);
        // 判不出时与旧实现一样显示 "?"
        snprintf(colOwn, sizeof(colOwn), "%s(%.2f)",
                 own.encoding.empty() ? "?" : own.encoding.c_str(), own.confidence);
        snprintf(col2, sizeof(col2), "%s(%.2f)", r2.c_str(), c2);
        snprintf(col3, sizeof(col3), "%s(%.2f)", r3.c_str(), c3);
        printf("%-28s %-20s %-20s %-20s %-20s\n", argv[i], col1, colOwn, col2, col3);
    }
    return 0;
}
