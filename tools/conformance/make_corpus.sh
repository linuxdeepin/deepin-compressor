#!/bin/bash
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later
#
# 生成一致性测试语料。同一脚本重复执行必须得到逐字节相同的产物，
# 否则基线（goldens）会因时间戳变化而失效。
#
# 用法：./make_corpus.sh [输出目录]

set -e

OUT="${1:-$(cd "$(dirname "$0")" && pwd)/corpus}"
case "$OUT" in /*) ;; *) OUT="$(pwd)/$OUT" ;; esac
rm -rf "$OUT"
mkdir -p "$OUT"

WORK="$(mktemp -d)"
trap 'rm -rf "$WORK"' EXIT

STAMP='2026-01-01 00:00:00 UTC'

# ---- 一棵含中文名、空文件、子目录、软链、可执行位的小树 ----
mkdir -p "$WORK/tree/sub/deep"
printf 'hello conformance\n' > "$WORK/tree/a.txt"
: > "$WORK/tree/empty.txt"
printf 'nested\n'  > "$WORK/tree/sub/b.txt"
printf 'deep\n'    > "$WORK/tree/sub/deep/c.txt"
printf '中文文件名内容\n' > "$WORK/tree/中文名.txt"
printf '#!/bin/sh\necho hi\n' > "$WORK/tree/run.sh"
chmod 0755 "$WORK/tree/run.sh"
ln -sf a.txt "$WORK/tree/link.txt"

# 固定时间戳，消除不可复现的元数据
find "$WORK/tree" -exec touch -h -d "$STAMP" {} +
(cd "$WORK/tree" && find . -mindepth 1 | LC_ALL=C sort > "$WORK/list.txt")

cd "$WORK/tree"

# ---- 归档样本：显式指定顺序与时间，关闭多余的元数据 ----
zip -X -q "$OUT/sample.zip" -@ < "$WORK/list.txt"
tar --no-recursion --sort=name --mtime="$STAMP" --owner=0 --group=0 --numeric-owner \
    -cf "$OUT/sample.tar" -T "$WORK/list.txt"
tar --no-recursion --sort=name --mtime="$STAMP" --owner=0 --group=0 --numeric-owner \
    -czf "$OUT/sample.tar.gz" -T "$WORK/list.txt"
7z a -t7z -bso0 -bsp0 -mtc=off -mta=off "$OUT/sample.7z" @"$WORK/list.txt" >/dev/null

# 分卷：用一段固定伪随机数据保证压缩后超过单卷大小，入口是 .001
rm -f "$OUT"/multi.7z.*
python3 - "$WORK/payload.bin" <<'PY'
import random, sys
r = random.Random(42)
with open(sys.argv[1], "wb") as fh:
    fh.write(bytes(r.randrange(256) for _ in range(20000)))
PY
touch -d "$STAMP" "$WORK/payload.bin"
(cd "$WORK" && 7z a -t7z -bso0 -bsp0 -mtc=off -mta=off -v4k "$OUT/multi.7z" payload.bin >/dev/null)

# 缺卷样本：只留首卷
rm -f "$OUT"/multi_missing.7z.*
cp "$OUT/multi.7z.001" "$OUT/multi_missing.7z.001"

# ---- tar 家族的其它 filter ----
tar --no-recursion --sort=name --mtime="$STAMP" --owner=0 --group=0 --numeric-owner \
    -cjf "$OUT/sample.tar.bz2" -T "$WORK/list.txt"
tar --no-recursion --sort=name --mtime="$STAMP" --owner=0 --group=0 --numeric-owner \
    -cJf "$OUT/sample.tar.xz" -T "$WORK/list.txt"

# ---- deb（ar 容器 + tar 成员；用 SOURCE_DATE_EPOCH 固定时间）----
DEBROOT="$WORK/debroot"
mkdir -p "$DEBROOT/DEBIAN" "$DEBROOT/usr/share/conformance"
printf 'Package: conformance\nVersion: 1.0\nArchitecture: all\nMaintainer: test <t@example.com>\nDescription: fixture\n' \
    > "$DEBROOT/DEBIAN/control"
printf 'payload\n' > "$DEBROOT/usr/share/conformance/data.txt"
find "$DEBROOT" -exec touch -h -d "$STAMP" {} +
SOURCE_DATE_EPOCH=$(date -u -d "$STAMP" +%s) dpkg-deb --root-owner-group --build "$DEBROOT" "$OUT/sample.deb" >/dev/null

# ---- iso ----
mkdir -p "$WORK/isoroot"
printf 'iso payload\n' > "$WORK/isoroot/hello.txt"
touch -d "$STAMP" "$WORK/isoroot/hello.txt"
# genisoimage（Debian 版）不支持 -creation-date 之类的开关，
# ISO 的卷时间戳因此随生成时刻变化，比对时把条目的 mtime 声明为不稳定字段
SOURCE_DATE_EPOCH=$(date -u -d "$STAMP" +%s) genisoimage -quiet -R -J -V CONFORMANCE \
    -o "$OUT/sample.iso" "$WORK/isoroot" 2>/dev/null


# ---- 非 UTF-8 条目名的 zip：名字按 GBK / Big5 / SJIS 原始字节写入，且不带 UTF-8 标志位 ----
# 构造方式：先用等长 ASCII 占位名建包，再把字节替换成目标编码的名字
python3 - "$OUT" <<'PY'
import sys, os, zipfile
out = sys.argv[1]
cases = {
    "names_gbk.zip":  ("中文文件.txt", "GBK"),
    "names_big5.zip": ("繁體測試.txt", "BIG5"),
    "names_sjis.zip": ("日本語資料.txt", "SHIFT_JIS"),
}
for filename, (text, enc) in cases.items():
    raw = text.encode(enc)
    path = os.path.join(out, filename)
    with zipfile.ZipFile(path, "w", zipfile.ZIP_STORED) as zf:
        info = zipfile.ZipInfo("Z" * len(raw))
        info.compress_type = zipfile.ZIP_STORED
        zf.writestr(info, b"payload\n")
    with open(path, "rb") as fh:
        data = fh.read()
    assert b"Z" * len(raw) in data
    data = data.replace(b"Z" * len(raw), raw)
    with open(path, "wb") as fh:
        fh.write(data)
PY


# ---- 端到端编码用例：每种编码一个 zip，内嵌 6 个该编码的条目名 ----
python3 - "$OUT" <<'PY'
import sys, os, zipfile
out = sys.argv[1]
CASES = {
    "names_gbk.zip": ("gb18030", ["中文文件.txt", "报告2026.pdf", "项目资料.zip", "会议纪要.doc",
                                  "统计图表.xls", "备份数据.tar"]),
    "names_big5.zip": ("big5", ["繁體檔案.txt", "報告資料.pdf", "專案清單.zip", "會議紀錄.doc",
                                "統計圖表.xls", "備份資料.tar"]),
    "names_sjis.zip": ("shift_jis", ["日本語.txt", "レポート.pdf", "資料一覧.zip", "議事録.doc",
                                     "統計表.xls", "バックアップ.tar"]),
    "names_euckr.zip": ("euc_kr", ["한국어.txt", "보고서.pdf", "자료목록.zip", "회의록.doc",
                                   "통계표.xls", "백업자료.tar"]),
    "names_cp1251.zip": ("cp1251", ["русский.txt", "отчёт.pdf", "архив.zip", "документ.doc",
                                    "таблица.xls", "резерв.tar"]),
    "names_tis620.zip": ("tis_620", ["ภาษาไทย.txt", "รายงาน.pdf", "เอกสาร.zip", "บันทึก.doc",
                                     "ตาราง.xls", "สำรอง.tar"]),
}
for filename, (enc, names) in CASES.items():
    entries = []
    for name in names:
        try:
            entries.append(name.encode(enc))
        except Exception:
            pass
    if not entries:
        continue
    path = os.path.join(out, filename)
    # 先用等长 ASCII 占位名建包，再把字节替换成目标编码的名字（不带 UTF-8 标志位）
    placeholders = []
    with zipfile.ZipFile(path, "w", zipfile.ZIP_STORED) as zf:
        for raw in entries:
            placeholder = ("P%03d" % len(placeholders)).encode() + b"x" * max(0, len(raw) - 4)
            placeholders.append((placeholder, raw))
            info = zipfile.ZipInfo(placeholder.decode())
            info.compress_type = zipfile.ZIP_STORED
            zf.writestr(info, b"payload\n")
    with open(path, "rb") as fh:
        data = fh.read()
    for placeholder, raw in placeholders:
        assert placeholder in data
        data = data.replace(placeholder, raw)
    with open(path, "wb") as fh:
        fh.write(data)
PY

# ---- 超长文件名（7z）：文件系统上建不出来，先在归档内用短名，再由 7z rn 改名 ----
rm -f "$OUT/longname.7z"
LONGNAME="$(python3 -c "print('b'*300 + '.txt')")"
(cd "$WORK/tree" && 7z a -t7z -bso0 -bsp0 -mtc=off -mta=off "$OUT/longname.7z" a.txt >/dev/null)
7z rn "$OUT/longname.7z" a.txt "$LONGNAME" >/dev/null
7z l -slt "$OUT/longname.7z" >/dev/null 2>&1 && \
    python3 - "$OUT/longname.7z" <<'PY'
import sys
import subprocess
out = subprocess.run(["7z", "l", sys.argv[1]], capture_output=True, text=True).stdout
assert "b" * 300 in out, "长名未写入归档"
PY

# ---- 损坏的 crx（前 4 字节是 CRX 魔数，后面是垃圾）----
printf 'Cr24\x03\x00\x00\x00' > "$OUT/corrupt.crx"
head -c 200 /dev/zero | tr '\0' 'x' >> "$OUT/corrupt.crx"

# ---- 加密样本 ----
7z a -tzip -bso0 -bsp0 -psecret123 -mem=AES256 -mtc=off -mta=off "$OUT/enc.zip" a.txt >/dev/null
7z a -t7z  -bso0 -bsp0 -psecret123 -mhe=on -mtc=off -mta=off "$OUT/enc.7z" a.txt >/dev/null

# ---- 损坏样本 ----
head -c 60 "$OUT/sample.zip" > "$OUT/corrupt.zip"

# 超长文件名（超过 NAME_MAX=255）：文件系统放不下，直接写进 zip
python3 - "$OUT/longname.zip" <<'PY'
import sys, zipfile
name = "a" * 300 + ".txt"
with zipfile.ZipFile(sys.argv[1], "w", zipfile.ZIP_DEFLATED) as zf:
    info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
    info.external_attr = 0o644 << 16
    zf.writestr(info, b"long name payload\n")
PY

printf 'zipcrypto payload\n' > "$WORK/secret.txt"
touch -d "$STAMP" "$WORK/secret.txt"
(cd "$WORK" && zip -q -X -P zpw123 "$OUT/enc_zipcrypto.zip" secret.txt)

# ---- 单文件样本（gz 用 -n 去掉名字与时间） ----
printf 'single file payload\n' > "$WORK/plain.txt"
: > "$WORK/empty.txt"
touch -d "$STAMP" "$WORK/plain.txt" "$WORK/empty.txt"
gzip -n -c "$WORK/plain.txt"  > "$OUT/plain.txt.gz"
bzip2 -c "$WORK/plain.txt"    > "$OUT/plain.txt.bz2"
xz -c "$WORK/plain.txt"       > "$OUT/plain.txt.xz"
gzip -n -c "$WORK/empty.txt"  > "$OUT/empty.txt.gz"
head -c 20 "$OUT/plain.txt.gz" > "$OUT/truncated.txt.gz"

# ---- 覆盖冲突用：与解压产物同名，内容不同 ----
mkdir -p "$OUT/prefill"
printf 'pre-existing\n' > "$OUT/prefill/plain.txt"
touch -d "$STAMP" "$OUT/prefill/plain.txt"

ls -l "$OUT"

# 输出包本身的时间戳也固定，否则单文件后端 list 出来的 mtime 每次都变
touch -d "$STAMP" "$OUT"/* 2>/dev/null || true
