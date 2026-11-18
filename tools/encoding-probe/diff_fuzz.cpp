// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 进程内差分：同一份字节同时喂给 旧 chardet 与 自有探测器，逐字节比对
// 用法: ./diff_fuzz [随机轮数]
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <random>
#include <string>
#include <vector>

#include "chardet.h"                    // 旧实现
#include "encoding_detector.h"          // 自有实现

namespace {

struct OldResult
{
    std::string encoding;
    float confidence = 0.0f;
    bool decided = false;
};

OldResult oldDetect(const char *data)
{
    OldResult r;
    DetectObj *obj = detect_obj_init();
    if (!obj)
        return r;
    if (detect(data, &obj) == CHARDET_SUCCESS && obj->encoding != nullptr) {
        r.encoding = obj->encoding;
        r.confidence = obj->confidence;
        r.decided = true;
    }
    detect_obj_free(&obj);
    return r;
}

std::string newEncoding(const char *data, size_t len)
{
    const encdet::Result r = encdet::detect(data, len);
    return r.encoding;
}

float newConfidence(const char *data, size_t len)
{
    return encdet::detect(data, len).confidence;
}

} // namespace

int main(int argc, char **argv)
{
    const int rounds = argc > 1 ? atoi(argv[1]) : 20000;
    std::mt19937 rng(20260117);
    std::uniform_int_distribution<int> byteDist(0, 255);

    int mismatchName = 0;
    int mismatchConf = 0;
    int checked = 0;

    auto hexdump = [](const std::string &d) {
        std::string out;
        for (unsigned char c : d) {
            char b[3];
            snprintf(b, sizeof(b), "%02x", c);
            out += b;
        }
        return out;
    };

    auto compare = [&](const std::string &data) {
      try {
        std::string buf = data;
        buf.push_back('\0');            // 旧接口按 C 字符串语义
        const OldResult o = oldDetect(buf.data());
        const std::string n = newEncoding(buf.data(), buf.size() - 1);
        const float cf = newConfidence(buf.data(), buf.size() - 1);
        ++checked;

        const std::string oldName = o.decided ? o.encoding : std::string();
        if (oldName != n) {
            if (mismatchName < 10) {
                printf("[名字差异] old=%s(%.2f) new=%s(%.2f) bytes=", oldName.empty() ? "?" : oldName.c_str(),
                       o.confidence, n.empty() ? "?" : n.c_str(), cf);
                for (unsigned char c : data)
                    printf("%02x", c);
                printf("\n");
            }
            ++mismatchName;
        } else if (o.decided && o.confidence != cf) {
            if (mismatchConf < 10)
                printf("[置信度差异] %s old=%.4f new=%.4f\n", n.c_str(), o.confidence, cf);
            ++mismatchConf;
        }
      } catch (const std::exception &e) {
        printf("[异常] %s bytes=%s\n", e.what(), hexdump(data).c_str());
        std::exit(2);
      }
    };

    // 1) 随机短字节串（覆盖 ASCII/高位/任意组合）
    for (int i = 0; i < rounds; ++i) {
        const int len = 1 + (rng() % 64);
        std::string s;
        s.reserve(static_cast<size_t>(len));
        for (int k = 0; k < len; ++k)
            s.push_back(static_cast<char>(byteDist(rng)));
        compare(s);
    }

    // 2) 随机长字节串（越过 1024 的门槛，压阈值/短路路径）
    for (int i = 0; i < rounds / 10; ++i) {
        const int len = 1024 + (rng() % 2048);
        std::string s;
        s.reserve(static_cast<size_t>(len));
        for (int k = 0; k < len; ++k)
            s.push_back(static_cast<char>(byteDist(rng)));
        compare(s);
    }

    printf("差分完成：%d 条，名字不一致 %d，置信度不一致 %d\n", checked, mismatchName, mismatchConf);
    return (mismatchName == 0 && mismatchConf == 0) ? 0 : 1;
}
