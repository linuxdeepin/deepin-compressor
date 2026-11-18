#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later
"""
归档后端一致性矩阵：录制 / 校验 / 跨后端比对

用法：
  ./run_matrix.py record   --build <build-dir>   # 从"当前实现"录制 golden
  ./run_matrix.py verify   --build <build-dir>   # 与 golden 比对（CI 闸门）
  ./run_matrix.py diff A.json B.json             # 任意两份观测的差异报告

设计要点：
- golden 由替换前的实现录制，之后任何实现都必须与之等价；
- 比对规则显式分级：语义差异 = FAIL，容忍差异 = WARN；
- 同一份样本跑不同后端时，diff 可给出"行为不一致"的具体字段（用于证明这套网能抓到问题）。
"""

import argparse
import glob
import json
import os
import shutil
import subprocess
import sys
import tempfile
from typing import Any, Dict, List, Tuple

HERE = os.path.dirname(os.path.abspath(__file__))
REPO = os.path.dirname(os.path.dirname(HERE))
CORPUS = os.path.join(HERE, "corpus")
TEST_SOURCES = os.path.join(REPO, "tests", "UnitTest", "test_sources")
GOLDENS = os.path.join(HERE, "goldens")


def golden_path(name: str) -> str:
    return os.path.join(GOLDENS, name + ".json")

# 插件在构建树中的相对路径
SO = {
    "libzip":        "3rdparty/libzipplugin/liblibzipplugin.so",
    "libarchive":    "3rdparty/libarchive/readwritelibarchiveplugin/libreadwritelibarchiveplugin.so",
    "libarchive-ro": "3rdparty/libarchive/readonlylibarchiveplugin/libreadonlylibarchiveplugin.so",
    "gz":            "3rdparty/libsinglefileplugin/gzplugin/libgzplugin.so",
    "bz2":           "3rdparty/libsinglefileplugin/bz2plugin/libbz2plugin.so",
    "xz":            "3rdparty/libsinglefileplugin/xzplugin/libxzplugin.so",
    "minizip":       "3rdparty/libminizipplugin/libminizipplugin.so",
    "cli7z":         "3rdparty/cli7zplugin/libcli7zplugin.so",
    "clirar":        "3rdparty/clirarplugin/libclirarplugin.so",
    "pigz":          "3rdparty/libpigzplugin/liblibpigzplugin.so",
}

# 用例矩阵：一个用例 = 后端 × 操作 × 样本 × 交互策略
# note 字段说明这个用例在守什么语义
CASES = [
    # 库后端：ZIP
    dict(name="libzip.list.zip",        backend="libzip",  op="list",    sample=f"{CORPUS}/sample.zip", note="条目/大小为隐式返回值(DataManager)"),
    dict(name="libzip.test.zip",        backend="libzip",  op="test",    sample=f"{CORPUS}/sample.zip", note="完整性校验"),
    dict(name="libzip.extract.zip",     backend="libzip",  op="extract", sample=f"{CORPUS}/sample.zip", note="内容/权限/软链落盘"),
    dict(name="libzip.list.corrupt",    backend="libzip",  op="list",    sample=f"{CORPUS}/corrupt.zip", note="损坏包错误类型"),
    dict(name="libzip.extract.enc.cancel", backend="libzip", op="extract", sample=f"{CORPUS}/enc.zip", respond="cancel", note="密码询问 -> 取消路径"),
    # 非 UTF-8 条目名：走编码探测（GBK/Big5/SJIS），守"名字解出来的结果"
    dict(name="libzip.list.names_gbk",   backend="libzip", op="list", sample=f"{CORPUS}/names_gbk.zip",  note="GBK 条目名 -> 探测后解码"),
    dict(name="libzip.list.names_big5",  backend="libzip", op="list", sample=f"{CORPUS}/names_big5.zip", note="Big5 条目名 -> 探测后解码"),
    dict(name="libzip.list.names_sjis",  backend="libzip", op="list", sample=f"{CORPUS}/names_sjis.zip", note="SJIS 条目名 -> 探测后解码"),
    dict(name="libzip.list.names_euckr",  backend="libzip", op="list", sample=f"{CORPUS}/names_euckr.zip",  note="EUC-KR 条目名"),
    dict(name="libzip.list.names_cp1251", backend="libzip", op="list", sample=f"{CORPUS}/names_cp1251.zip", note="CP1251 条目名"),
    dict(name="libzip.list.names_tis620", backend="libzip", op="list", sample=f"{CORPUS}/names_tis620.zip", note="TIS-620 条目名"),
    # 库后端：libarchive（tar 家族）
    dict(name="libarchive.list.tar",    backend="libarchive", op="list",    sample=f"{CORPUS}/sample.tar", note="tar 条目/权限"),
    dict(name="libarchive.extract.tar", backend="libarchive", op="extract", sample=f"{CORPUS}/sample.tar", note="tar 落盘/软链/权限"),
    dict(name="libarchive.list.tgz",    backend="libarchive", op="list",    sample=f"{CORPUS}/sample.tar.gz", note="filter 链"),
    # 单文件插件
    dict(name="gz.list",                backend="gz",     op="list",    sample=f"{CORPUS}/sample.tar.gz", note="单文件 list 语义"),
    # gz 单文件插件落盘时不恢复 mtime（实测），该字段由现存实现决定为"当前时间"，
    # 因此只在本用例内声明为不稳定字段，而不是全局放宽规则。
    dict(name="gz.extract",             backend="gz",     op="extract", sample=f"{CORPUS}/sample.tar.gz", note="流式解压", unstable=["fs.mtime"]),
    # 单文件后端：gz / bz2 / xz —— 覆盖 6 类能力（格式、操作矩阵、错误、列表、交互、落盘）
    dict(name="gz.list.plain",      backend="gz",  op="list",    sample=f"{CORPUS}/plain.txt.gz",  note="单条目：名字=包名去后缀，size=包大小"),
    dict(name="gz.extract.plain",   backend="gz",  op="extract", sample=f"{CORPUS}/plain.txt.gz",  note="流式解压，不恢复 mtime", unstable=["fs.mtime"]),
    dict(name="gz.extract.overwrite", backend="gz", op="extract", sample=f"{CORPUS}/plain.txt.gz", respond="overwrite",
         extra=["--prefill", f"{CORPUS}/prefill/plain.txt"], note="目标已存在 → 覆盖", unstable=["fs.mtime"]),
    dict(name="gz.extract.skip",    backend="gz",  op="extract", sample=f"{CORPUS}/plain.txt.gz", respond="skip",
         extra=["--prefill", f"{CORPUS}/prefill/plain.txt"], note="目标已存在 → 跳过，原文件内容保留", unstable=["fs.mtime"]),
    dict(name="gz.list.truncated",  backend="gz",  op="list",    sample=f"{CORPUS}/truncated.txt.gz", note="截断包的错误类型"),
    dict(name="gz.extract.truncated", backend="gz", op="extract", sample=f"{CORPUS}/truncated.txt.gz", note="截断包解压失败", unstable=["fs.mtime"]),
    dict(name="gz.list.empty",      backend="gz",  op="list",    sample=f"{CORPUS}/empty.txt.gz",  note="空内容包"),
    dict(name="bz2.list.plain",     backend="bz2", op="list",    sample=f"{CORPUS}/plain.txt.bz2", note="bz2 列表"),
    dict(name="bz2.extract.plain",  backend="bz2", op="extract", sample=f"{CORPUS}/plain.txt.bz2", note="bz2 解压", unstable=["fs.mtime"]),
    dict(name="xz.list.plain",      backend="xz",  op="list",    sample=f"{CORPUS}/plain.txt.xz",  note="xz 列表"),
    dict(name="xz.extract.plain",   backend="xz",  op="extract", sample=f"{CORPUS}/plain.txt.xz",  note="xz 解压", unstable=["fs.mtime"]),
    # CLI 后端
    dict(name="cli7z.list.7z",          backend="cli7z",  op="list",    sample=f"{CORPUS}/sample.7z", note="CLI 输出解析 -> 7z 条目"),
    dict(name="cli7z.extract.7z",       backend="cli7z",  op="extract", sample=f"{CORPUS}/sample.7z", note="CLI 异步 + 进度/当前文件"),
    dict(name="cli7z.extract.enc.cancel", backend="cli7z", op="extract", sample=f"{CORPUS}/enc.7z", respond="cancel", note="层级加密头 -> 密码询问"),
    dict(name="cli7z.list.multi",     backend="cli7z", op="list",    sample=f"{CORPUS}/multi.7z.001", note="分卷首卷"),
    dict(name="cli7z.extract.multi",  backend="cli7z", op="extract", sample=f"{CORPUS}/multi.7z.001", note="分卷解压"),
    dict(name="cli7z.list.missing_volume", backend="cli7z", op="list", sample=f"{CORPUS}/multi_missing.7z.001", note="缺卷 -> 损坏确认"),
    dict(name="cli7z.list.longname",    backend="cli7z", op="list",    sample=f"{CORPUS}/longname.7z", note="300 字符文件名"),
    dict(name="cli7z.extract.longname", backend="cli7z", op="extract", sample=f"{CORPUS}/longname.7z", note="超长名解压"),
    dict(name="clirar.list.rar",       backend="clirar", op="list",    sample=f"{REPO}/tests/FuzzyTest/libfuzzer/libfuzzer_in/archives/common/rar/small_archive.rar", note="unrar 输出解析"),
    dict(name="clirar.extract.rar",    backend="clirar", op="extract", sample=f"{REPO}/tests/FuzzyTest/libfuzzer/libfuzzer_in/archives/common/rar/small_archive.rar", note="unrar 解压"),
    dict(name="clirar.list.corrupt",   backend="clirar", op="list",    sample=f"{CORPUS}/corrupt.crx", note="非 rar 输入的报错"),
    # tar 家族：其它 filter
    dict(name="libarchive.list.tbz2",    backend="libarchive", op="list",    sample=f"{CORPUS}/sample.tar.bz2", note="bzip2 filter"),
    dict(name="libarchive.extract.tbz2", backend="libarchive", op="extract", sample=f"{CORPUS}/sample.tar.bz2", note="bzip2 filter 解压", unstable=["signals.progressCount", "signals.progressFirst", "signals.progressLast", "signals.progressMonotonic"]),
    dict(name="libarchive.list.txz",     backend="libarchive", op="list",    sample=f"{CORPUS}/sample.tar.xz",  note="xz filter"),
    dict(name="libarchive.extract.txz",  backend="libarchive", op="extract", sample=f"{CORPUS}/sample.tar.xz",  note="xz filter 解压", unstable=["signals.progressCount", "signals.progressFirst", "signals.progressLast", "signals.progressMonotonic"]),
    # 只读格式
    dict(name="libarchive-ro.list.deb",    backend="libarchive-ro", op="list",    sample=f"{CORPUS}/sample.deb", note="ar 容器"),
    dict(name="libarchive-ro.extract.deb", backend="libarchive-ro", op="extract", sample=f"{CORPUS}/sample.deb", note="deb 解包", unstable=["signals.progressCount", "signals.progressFirst", "signals.progressLast", "signals.progressMonotonic"]),
    dict(name="libarchive-ro.list.iso",    backend="libarchive-ro", op="list",    sample=f"{CORPUS}/sample.iso", note="iso9660", unstable=["archiveData.entries[].mtime"],),
    dict(name="libarchive-ro.extract.iso", backend="libarchive-ro", op="extract", sample=f"{CORPUS}/sample.iso", note="iso 解出", unstable=["signals.progressCount", "signals.progressFirst", "signals.progressLast", "signals.progressMonotonic"]),
    # 仓库里已有的小样本（保证历史资产也被守住）
    dict(name="libzip.list.repo_zip",   backend="libzip", op="list",    sample=f"{TEST_SOURCES}/zip/extract/test.zip"),
    dict(name="libzip.extract.repo_zip", backend="libzip", op="extract", sample=f"{TEST_SOURCES}/zip/extract/test.zip", note="仓库样本整包解出"),
    dict(name="libzip.list.enc_zipcrypto", backend="libzip", op="list",  sample=f"{CORPUS}/enc_zipcrypto.zip", note="ZipCrypto 加密包列表（不需要密码）"),
    dict(name="libzip.extract.enc_zipcrypto.cancel", backend="libzip", op="extract", sample=f"{CORPUS}/enc_zipcrypto.zip", respond="cancel", note="ZipCrypto 解压 -> 取消密码询问"),
    dict(name="libzip.list.longname",   backend="libzip", op="list",    sample=f"{CORPUS}/longname.zip", note="300 字符文件名"),
    dict(name="libzip.extract.longname", backend="libzip", op="extract", sample=f"{CORPUS}/longname.zip", note="超长名落盘（截断规则）"),
    dict(name="libarchive.list.repo_tar", backend="libarchive", op="list", sample=f"{TEST_SOURCES}/tar/extract/test.tar"),
    dict(name="minizip.list.crx",       backend="minizip", op="list",   sample=f"{TEST_SOURCES}/crx/test.crx"),
    dict(name="minizip.extract.crx",    backend="minizip", op="extract", sample=f"{TEST_SOURCES}/crx/test.crx", note="crx 整包解出；旧实现不恢复 mtime", unstable=["fs.mtime"]),
    dict(name="minizip.list.corrupt",   backend="minizip", op="list",   sample=f"{CORPUS}/corrupt.crx", note="损坏的 crx"),
    dict(name="minizip.extract.corrupt", backend="minizip", op="extract", sample=f"{CORPUS}/corrupt.crx", note="损坏的 crx 解压"),
]

# 视为"环境噪声"的字段（不参与比较）
IGNORE_KEYS = {"target", "tempTarget", "case"}


def run_case(build: str, case: Dict[str, Any], timeout: int = 180) -> Any:
    binary = os.path.join(build, "bin", "tests", "conformance")
    if not os.path.exists(binary):
        sys.exit(f"找不到观测器: {binary}\n请先 cmake -DBUILD_CONFORMANCE=ON 并 make conformance")
    # 同一后端在不同构建里可能落在不同目录（3rdparty/... 或 own-*/），
    # 构建树里还可能同时留着历史产物：按同名文件取最新的那份
    so = os.path.join(build, SO[case["backend"]])
    hits = glob.glob(os.path.join(build, "**", os.path.basename(SO[case["backend"]])), recursive=True)
    if hits:
        hits.sort(key=os.path.getmtime, reverse=True)
        so = hits[0]
    if not os.path.exists(so):
        return {"_skipped": f"插件未构建: {so}"}

    sample = case["sample"]
    if case.get("copy_sample"):
        # 写操作会改归档本身：每次用一份全新副本，保证可重复
        work = tempfile.mkdtemp(prefix="conformance-copy-")
        sample = os.path.join(work, os.path.basename(sample))
        shutil.copy2(case["sample"], sample)

    cmd = [binary, "--backend", case["backend"], "--so", so,
           "--op", case["op"], "--sample", sample]
    if case.get("out_archive"):
        work = tempfile.mkdtemp(prefix="conformance-out-")
        cmd += ["--archive", os.path.join(work, case["out_archive"])]
    if case.get("respond"):
        cmd += ["--respond", case["respond"]]
    if case.get("extra"):
        cmd += case["extra"]

    env = dict(os.environ)
    # compressor-interface 可能在 3rdparty/interface，也可能在 src/archive/api 的构建目录
    lib_dirs = [os.path.dirname(p) for p in
                glob.glob(os.path.join(build, "**", "libcompressor-interface.so"), recursive=True)]
    env["LD_LIBRARY_PATH"] = ":".join(lib_dirs + [env.get("LD_LIBRARY_PATH", "")])
    env["QT_QPA_PLATFORM"] = "offscreen"
    p = subprocess.run(cmd, capture_output=True, text=True, timeout=timeout, env=env)
    if not p.stdout.strip():
        return {"_error": f"观测失败 rc={p.returncode}: {p.stderr[-800:]}"}
    try:
        return json.loads(p.stdout)
    except json.JSONDecodeError as e:
        return {"_error": f"JSON 解析失败: {e}; stdout={p.stdout[:400]}"}


def normalize(obs: Any, unstable: List[str] = None) -> Any:
    """把观测结果规范化：去掉环境噪声，路径换成相对名，便于跨机器比对。"""
    if not isinstance(obs, dict):
        return obs
    out = {k: v for k, v in obs.items() if k not in IGNORE_KEYS}
    if "case" in obs:
        c = dict(obs["case"])
        c.pop("sample", None)
        out["case"] = c
    # 条目排序键统一（DataManager 是 QMap，本身有序；这里再兜一层）
    ad = out.get("archiveData")
    if isinstance(ad, dict) and isinstance(ad.get("entries"), list):
        ad["entries"] = sorted(ad["entries"], key=lambda x: x.get("name", ""))
    fs = out.get("fs")
    for rule in (unstable or []):
        if rule == "fs.mtime" and isinstance(fs, list):
            for e in fs:
                if isinstance(e, dict):
                    e.pop("mtime", None)
        elif rule == "archiveData.entries[].mtime" and isinstance(ad, dict):
            # 例如 ISO：条目时间戳来自镜像生成时刻，跨次生成会变
            for e in ad.get("entries", []):
                if isinstance(e, dict):
                    e.pop("mtime", None)
    if isinstance(fs, list):
        # 目录的 mtime 是解压动作的时间戳，天然不稳定，不参与比对
        for e in fs:
            if isinstance(e, dict) and e.get("type") == "dir":
                e.pop("mtime", None)
        out["fs"] = sorted(fs, key=lambda x: x.get("path", ""))
    return out


# ---------------- 比对 ----------------

SEV_FAIL = "FAIL"
SEV_WARN = "WARN"


def walk_diff(a: Any, b: Any, path: str = "") -> List[Tuple[str, str, Any, Any]]:
    """结构化比较，返回 [(severity, path, a, b), ...]"""
    diffs = []
    if type(a) is not type(b) and not (isinstance(a, (int, float)) and isinstance(b, (int, float))):
        return [(SEV_FAIL, path or ".", a, b)]
    if isinstance(a, dict):
        for k in sorted(set(a) | set(b)):
            if k not in a:
                diffs.append((SEV_FAIL, f"{path}.{k}", "<缺失>", b[k]))
            elif k not in b:
                diffs.append((SEV_FAIL, f"{path}.{k}", a[k], "<缺失>"))
            else:
                diffs += walk_diff(a[k], b[k], f"{path}.{k}")
        return diffs
    if isinstance(a, list):
        if len(a) != len(b):
            diffs.append((SEV_FAIL, f"{path}.length", len(a), len(b)))
        for i in range(min(len(a), len(b))):
            diffs += walk_diff(a[i], b[i], f"{path}[{i}]")
        return diffs
    if a != b:
        # 进度/信号类字段允许抖动
        tolerant = any(t in path for t in (".signals.progress", ".signals.curFiles", ".signals.finishedSignals"))
        diffs.append((SEV_WARN if tolerant else SEV_FAIL, path, a, b))
    return diffs


def compare(expected: Any, actual: Any, label: str = ""):
    diffs = walk_diff(expected, actual)
    fails = [d for d in diffs if d[0] == SEV_FAIL]
    warns = [d for d in diffs if d[0] == SEV_WARN]
    return fails, warns


def fmt(d):
    if isinstance(d, str):
        return f"  ERROR {d}"
    sev, path, a, b = d
    return f"  {sev} {path}\n       期望: {json.dumps(a, ensure_ascii=False)[:160]}\n       实际: {json.dumps(b, ensure_ascii=False)[:160]}"


# ---------------- 三个子命令 ----------------

def cmd_record(build, only=None):
    os.makedirs(GOLDENS, exist_ok=True)
    rc = 0
    for case in CASES:
        if only and only not in case["name"]:
            continue
        obs = run_case(build, case)
        if "_skipped" in obs or "_error" in obs:
            print(f"[SKIP] {case['name']}: {obs.get('_skipped') or obs.get('_error')}")
            continue
        path = os.path.join(GOLDENS, case["name"] + ".json")
        with open(path, "w", encoding="utf-8") as f:
            json.dump(normalize(obs, case.get("unstable")), f, ensure_ascii=False, indent=2, sort_keys=True)
        sev = ""
        r = obs.get("result", {})
        if r.get("errorType", 0) != 0 or r.get("finish", 0) != 0:
            sev = f"  (finish={r.get('finish')} err={r.get('errorType')})"
        print(f"[REC ] {case['name']:<32} -> {os.path.relpath(path, HERE)}{sev}")
    return rc


def cmd_verify(build, only=None):
    total = passed = 0
    failures = []
    for case in CASES:
        if only and only not in case["name"]:
            continue
        gpath = golden_path(case["name"])
        if not os.path.exists(gpath):
            continue
        total += 1
        with open(gpath, encoding="utf-8") as f:
            expected = json.load(f)
        obs = run_case(build, case)
        if "_skipped" in obs or "_error" in obs:
            failures.append((case["name"], [obs.get("_skipped") or obs.get("_error")], []))
            continue
        # 基线与实测都要按同一套不稳定字段规则规范化，否则基线里残留的字段永远比不过
        fails, warns = compare(normalize(expected, case.get("unstable")),
                               normalize(obs, case.get("unstable")))
        if fails:
            failures.append((case["name"], fails, warns))
        else:
            passed += 1
            print(f"[ OK ] {case['name']}" + (f"  ({len(warns)} warn)" if warns else ""))

    for name, fails, warns in failures:
        print(f"\n[FAIL] {name}  ({len(fails)} 处语义差异)")
        for d in fails[:12]:
            print(fmt(d))
        if len(fails) > 12:
            print(f"  ... 还有 {len(fails) - 12} 处")
        for d in warns[:3]:
            print(fmt(d))

    print(f"\n结果: {passed}/{total} 通过")
    return 0 if not failures else 1


def cmd_diff(a_path, b_path):
    with open(a_path, encoding="utf-8") as f:
        a = normalize(json.load(f))
    with open(b_path, encoding="utf-8") as f:
        b = normalize(json.load(f))
    fails, warns = compare(a, b)
    print(f"A = {os.path.basename(a_path)}")
    print(f"B = {os.path.basename(b_path)}")
    print(f"语义差异 {len(fails)} 处，容忍差异 {len(warns)} 处\n")
    for d in fails[:40]:
        print(fmt(d))
    if len(fails) > 40:
        print(f"... 还有 {len(fails)-40} 处")
    for d in warns[:5]:
        print(fmt(d))
    return 0


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("mode", choices=["record", "verify", "diff"])
    ap.add_argument("paths", nargs="*")
    ap.add_argument("--build", default=os.environ.get("CONFORMANCE_BUILD", ""))
    ap.add_argument("--only", default=None, help="只跑名字包含该子串的用例")
    args = ap.parse_args()

    if args.mode == "diff":
        if len(args.paths) != 2:
            sys.exit("diff 需要两个 JSON 路径")
        return cmd_diff(args.paths[0], args.paths[1])

    build = args.build
    if not build:
        sys.exit("需要 --build <cmake 构建目录> 或环境变量 CONFORMANCE_BUILD")
    if args.mode == "record":
        return cmd_record(build, args.only)
    return cmd_verify(build, args.only)


if __name__ == "__main__":
    sys.exit(main())
