#!/usr/bin/env bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
#
# 编码探测对比：把同一份字节分别喂给 旧 chardet / uchardet / ICU，打印各自判定。
#
# 用法: ./run_probe.sh [构建目录]
#   构建目录用于找 libcompressor-ChardetDetector.a（对照构建，默认 dc-off）。

set -u

HERE="$(cd "$(dirname "$0")" && pwd)"
REPO="$(cd "$HERE/../.." && pwd)"
# 旧 chardet 只在对照构建里编（dc-off）；自有实现直接编 src 源码
BUILD="${1:-$HOME/.hermes/cache/scratch/dc-off}"
LIB="$BUILD/3rdparty/ChardetDetector/libcompressor-ChardetDetector.a"

if [ ! -f "$LIB" ]; then
    echo "找不到 $LIB（先构建一次，或用参数指定构建目录）" >&2
    exit 1
fi

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

g++ -std=c++17 -O1 -o "$WORK/probe" "$HERE/probe.cpp" \
    "$REPO/src/archive/api/encoding/encoding_detector.cpp" \
    -I"$REPO/src/archive/api/encoding" \
    -I"$REPO/3rdparty/ChardetDetector" \
    $(pkg-config --cflags uchardet icu-i18n) \
    "$LIB" \
    $(pkg-config --libs uchardet icu-i18n) || exit 1

# 构造样本：同一段文字用不同编码落盘
gen() { printf '%s' "$2" | iconv -f UTF-8 -t "$1" > "$WORK/$3" 2>/dev/null; }

gen GBK       '中文测试文件名示例内容
项目报告2026年第一季度'                     gbk.bin
gen BIG5      '繁體中文測試檔名內容
專案報告'                                   big5.bin
gen SHIFT_JIS '日本語のファイル名テスト
レポート資料'                               sjis.bin
gen EUC-KR    '한국어 파일 이름 테스트
보고서 자료'                                euckr.bin

# 短样本：压缩包里的条目名通常就这么几个字节
gen GBK       '中文.txt'                    gbk_short.bin
gen BIG5      '報告.txt'                    big5_short.bin
gen SHIFT_JIS '資料/ファイル.txt'           sjis_short.bin

# 短样本 + 填充（验证"补足上下文"能否救回短样本）
cp "$WORK/gbk_short.bin" "$WORK/gbk_short_pad.bin"
printf '为增加探测率保留的中文' | iconv -f UTF-8 -t GBK >> "$WORK/gbk_short_pad.bin"
cp "$WORK/big5_short.bin" "$WORK/big5_short_pad.bin"
printf '為增加探測率保留的中文' | iconv -f UTF-8 -t BIG5 >> "$WORK/big5_short_pad.bin"

printf 'plain ascii file name test\n' > "$WORK/ascii.bin"
printf '中文测试文件名示例内容\n' > "$WORK/utf8.txt"

(cd "$WORK" && ./probe *.bin utf8.txt)
