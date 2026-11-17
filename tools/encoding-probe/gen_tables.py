#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later

"""从 3rdparty/ChardetDetector 抽取"参数"（状态机表、语言模型表、拉丁表），
生成自研探测器用的数据头文件 own_tables.h。

只做机械改名：顶层数据符号统一加 own_ 前缀并加 static，避免与库里的同名符号冲突。
算法一行不抄——生成出来的文件里只有数组常量。
"""

import re
import sys
from pathlib import Path

REPO = Path(__file__).resolve().parents[2]
SRC = REPO / "3rdparty" / "ChardetDetector"
OUT = REPO / "src" / "archive" / "api" / "encoding" / "encoding_tables.h"

# (相对路径, 抽取起始标记, 说明)
FREQ_TABLES = [
    "GB2312Freq.tab",
    "Big5Freq.tab",
    "JISFreq.tab",
    "EUCKRFreq.tab",
    "EUCTWFreq.tab",
]

SOURCES = [
    ("nsMBCSSM.cpp", None, "CJK 多字节状态机（SMModel）"),
    ("nsEscSM.cpp", None, "ISO-2022 系状态机（SMModel）"),
    ("tables/LangBulgarianModel.cpp", None, "单字节语言模型"),
    ("tables/LangCyrillicModel.cpp", None, "单字节语言模型"),
    ("tables/LangGreekModel.cpp", None, "单字节语言模型"),
    ("tables/LangHebrewModel.cpp", None, "单字节语言模型"),
    ("tables/LangHungarianModel.cpp", None, "单字节语言模型"),
    ("tables/LangThaiModel.cpp", None, "单字节语言模型"),
    ("nsLatin1Prober.cpp", "latin1", "Latin1 类表"),
    ("JpCntx.cpp", None, "日文上下文表"),
]

TOP_DECL = re.compile(
    r"^(static\s+)?(const\s+)?(PRUint32|PRUint16|PRInt16|PRUint8|PRInt32|unsigned\s+char|signed\s+char|"
    r"char|float|double|SMModel|SequenceModel)\b",
)

# 这些是函数 / 结构体定义，不能当数据抽
SKIP = re.compile(r"^(static\s+)?(void|int|float|float\s+\w+::|PRInt32\s+\w+::|PRBool)\b.*\(")

def source_commit() -> str:
    import subprocess
    try:
        out = subprocess.run(["git", "-C", str(REPO), "rev-parse", "--short", "HEAD"],
                             capture_output=True, text=True, timeout=10)
        return out.stdout.strip() or "unknown"
    except Exception:
        return "unknown"


HEADER = """// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

// 自动生成，请勿手改。生成脚本：tools/encoding-probe/gen_tables.py
// 数据来源：Mozilla UniversalDetector（MPL 1.1 / GPL 2.0 / LGPL 2.1）
// 来源提交：{commit}

#ifndef DC_ENCODING_TABLES_H
#define DC_ENCODING_TABLES_H

#include <cstdint>

// ---- 基础类型（与原实现一致的位宽） ----
typedef short PRInt16;
typedef unsigned short PRUint16;
typedef int PRInt32;
typedef unsigned PRUint32;
typedef int PRBool;

#ifndef PR_FALSE
#define PR_FALSE 0
#endif
#ifndef PR_TRUE
#define PR_TRUE 1
#endif

// ---- 打包查表：4/8/16 位单位 ----
typedef enum { eIdxSft4bits = 3, eIdxSft8bits = 2, eIdxSft16bits = 1 } nsIdxSft;
typedef enum { eSftMsk4bits = 7, eSftMsk8bits = 3, eSftMsk16bits = 1 } nsSftMsk;
typedef enum { eBitSft4bits = 2, eBitSft8bits = 3, eBitSft16bits = 4 } nsBitSft;
typedef enum { eUnitMsk4bits = 0x0000000FL, eUnitMsk8bits = 0x000000FFL,
               eUnitMsk16bits = 0x0000FFFFL } nsUnitMsk;

typedef struct nsPkgInt {
    nsIdxSft idxsft;
    nsSftMsk sftmsk;
    nsBitSft bitsft;
    nsUnitMsk unitmsk;
    PRUint32 *data;
} nsPkgInt;

#define PCK16BITS(a,b) ((PRUint32)(((b) << 16) | (a)))
#define PCK8BITS(a,b,c,d) PCK16BITS(((PRUint32)(((b) << 8) | (a))), ((PRUint32)(((d) << 8) | (c))))
#define PCK4BITS(a,b,c,d,e,f,g,h) PCK8BITS(((PRUint32)(((b) << 4) | (a))), \
                                          ((PRUint32)(((d) << 4) | (c))), \
                                          ((PRUint32)(((f) << 4) | (e))), \
                                          ((PRUint32)(((h) << 4) | (g))))
#define GETFROMPCK(i, c) \
 (((((c).data)[(i)>>(c).idxsft])>>(((i)&(c).sftmsk)<<(c).bitsft))&(c).unitmsk)

// ---- 状态机状态枚举（与原实现一致） ----
typedef enum { eStart = 0, eError, eItsMe } nsSMState;

// ---- 探测器用的两组模型结构 ----
typedef struct {
    nsPkgInt classTable;
    PRUint32 classFactor;
    nsPkgInt stateTable;
    const PRUint32 *charLenTable;
    const char *name;
} SMModel;

typedef struct {
    unsigned char *charToOrderMap;
    char *precedenceMatrix;
    float mTypicalPositiveRatio;
    PRBool keepEnglishLetter;
    const char *charsetName;
} SequenceModel;

"""


LATIN1_DEFINES = """
#define UDF    0
#define OTH    1
#define ASC    2
#define ASS    3
#define ACV    4
#define ACO    5
#define ASV    6
#define ASO    7
#define CLASS_NUM   8
"""


def extract(path: Path) -> str:
    """只抽取"顶层数据块"：从形如 `<类型> <名字>[] = {` 的行开始，到配对的 `};` 结束。
    函数体、宏、结构体定义一律丢弃。"""
    text = path.read_text(encoding="utf-8", errors="replace")
    lines = text.splitlines()

    result = []
    i = 0
    n = len(lines)
    while i < n:
        line = lines[i]
        m = TOP_DECL.match(line)
        if m and ("=" in line):
            block = [line]
            depth = line.count("{") - line.count("}")
            seen_brace = "{" in line
            i += 1
            # 数据块有两种写法：`X[] = {…}` 一气呵成，或 `X[] =` 换行再 `{…}`。
            # 一直读到"见过左花括号且配平"为止；遇到下一个顶层声明则视为上块结束。
            while i < n and (not seen_brace or depth > 0):
                nxt = lines[i]
                if seen_brace and depth == 0 and TOP_DECL.match(nxt):
                    break
                block.append(nxt)
                seen_brace = seen_brace or ("{" in nxt)
                depth += nxt.count("{") - nxt.count("}")
                i += 1
                if seen_brace and depth <= 0 and block[-1].lstrip().startswith("};"):
                    break

            head = block[0]
            rest = head[len(m.group(0)):].lstrip()
            name_match = re.match(r"([A-Za-z_]\w*)", rest)
            if name_match:
                name = name_match.group(1)
                body = rest[len(name):]
                result.append(f"static {m.group(2) or ''}{m.group(3)} own_{name}{body}")
                result.extend(block[1:])
                continue
        i += 1
    body = "\n".join(result)
    if path.name == "nsLatin1Prober.cpp":
        body = LATIN1_DEFINES + body
    return body


def main() -> int:
    check_only = "--check" in sys.argv
    if not SRC.exists():
        print(f"源目录不存在（{SRC}）。encoding_tables.h 已是冻结数据，无需重新生成；\n"
              f"若要重新生成，请先 checkout 生成它的那个提交。", file=sys.stderr)
        return 0 if check_only else 1

    chunks = [HEADER.replace("{commit}", source_commit())]
    for rel, _marker, note in SOURCES:
        path = SRC / rel
        if not path.exists():
            print(f"跳过（不存在）: {rel}", file=sys.stderr)
            continue
        chunks.append(f"\n// ---- {rel}：{note} ----\n")
        chunks.append(extract(path))
        chunks.append("\n")

    for rel in FREQ_TABLES:
        path = SRC / rel
        if not path.exists():
            print(f"跳过（不存在）: {rel}", file=sys.stderr)
            continue
        chunks.append(f"\n// ---- {rel}：频率表 ----\n")
        chunks.append(path.read_text(encoding="utf-8", errors="replace"))

    text = "\n".join(chunks)
    names = set(re.findall(r"own_([A-Za-z_]\w*)", text))
    for name in sorted(names, key=len, reverse=True):
        text = re.sub(rf"(?<!own_)(?<![A-Za-z0-9_]){name}(?![A-Za-z0-9_])", f"own_{name}", text)
    text = text.replace("own_own_", "own_")
    text += "\n#endif // DC_ENCODING_TABLES_H\n"
    if check_only:
        current = OUT.read_text(encoding="utf-8") if OUT.exists() else ""
        if current == text:
            print(f"{OUT} 与源一致")
            return 0
        print(f"{OUT} 与源不一致，需要重新生成", file=sys.stderr)
        return 1

    OUT.write_text(text, encoding="utf-8")
    print(f"生成 {OUT}（{len(text.splitlines())} 行）")
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
