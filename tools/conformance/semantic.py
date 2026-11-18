#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
#
# SPDX-License-Identifier: GPL-3.0-or-later
"""写操作的语义校验。

旧实现的 delete / rename 是"就地重写"：initializeWriter() 以 WriteOnly 打开
归档本身，而读句柄还开着，结果把包写坏（GNU tar / python tarfile 都读不了）。
所以这三个操作不能用"与旧实现逐字段相同"当判据，改为用独立的
python tarfile 判定结果是否正确。
"""

import argparse
import glob
import hashlib
import io
import json
import os
import shutil
import subprocess
import tarfile
import zipfile
import sys
import tempfile

REPO = os.path.dirname(os.path.dirname(os.path.dirname(os.path.abspath(__file__))))
CORPUS = os.path.join(REPO, "tools", "conformance", "corpus")

SO = {
    "cli7z": "3rdparty/cli7zplugin/libcli7zplugin.so",
    "clirar": "3rdparty/clirarplugin/libclirarplugin.so",
    "libzip": "3rdparty/libzipplugin/liblibzipplugin.so",
    "libarchive": "3rdparty/libarchive/readwritelibarchiveplugin/libreadwritelibarchiveplugin.so",
    "pigz": "3rdparty/libpigzplugin/liblibpigzplugin.so",
    "clipzip": "3rdparty/clipzipplugin/libclipzipplugin.so",
}

# 夹具必须小于 10KB：旧实现"就地重写"在小包上必然损坏（大包会被读缓冲掩盖）
FILES = {
    "a.txt": b"alpha\n",
    "b.txt": b"bravo\n",
    "sub/c.txt": b"charlie\n",
}


def sha(data):
    return hashlib.sha256(data).hexdigest()


def make_fixture(path, mode="w"):
    """用 python tarfile 生成夹具，不依赖外部 tar 二进制。"""
    with tarfile.open(path, mode, format=tarfile.PAX_FORMAT) as tf:  # type: ignore[arg-type]
        for name, data in FILES.items():
            if "/" in name:
                parent = name.split("/")[0]
                d = tarfile.TarInfo(parent)
                d.type = tarfile.DIRTYPE
                d.mode = 0o755
                d.mtime = 1767225600
                tf.addfile(d)
            info = tarfile.TarInfo(name)
            info.size = len(data)
            info.mtime = 1767225600
            info.mode = 0o644
            tf.addfile(info, io.BytesIO(data))
        link = tarfile.TarInfo("link.txt")
        link.type = tarfile.SYMTYPE
        link.linkname = "a.txt"
        link.mtime = 1767225600
        tf.addfile(link)


def make_zip_fixture(path):
    import zipfile
    with zipfile.ZipFile(path, "w", zipfile.ZIP_DEFLATED) as zf:
        for name, data in FILES.items():
            info = zipfile.ZipInfo(name, date_time=(2026, 1, 1, 0, 0, 0))
            info.external_attr = 0o644 << 16
            zf.writestr(info, data)


def entries_zip(path):
    import zipfile
    with zipfile.ZipFile(path) as zf:
        return {n: (len(zf.read(n)), sha(zf.read(n))) for n in zf.namelist()}


def seven_zip_entries(path):
    """用 7z 自身的列表输出核对结果（插件只负责拼参数和解析，7z 行为不在此列）。"""
    out = subprocess.run(["7z", "l", "-slt", path], capture_output=True, text=True)
    entries = {}
    current = None
    for line in out.stdout.splitlines():
        if line.startswith("Path = "):
            current = line[7:].strip()
        elif line.startswith("Size = ") and current:
            entries[current] = int(line[7:].strip())
            current = None
    entries.pop(path, None)
    return entries


def seven_zip_content(path, name):
    out = subprocess.run(["7z", "e", "-so", path, name], capture_output=True)
    return sha(out.stdout)


def make_7z_fixture(path):
    work = tempfile.mkdtemp(prefix="sem-7zfix-")
    for name, data in FILES.items():
        target = os.path.join(work, name)
        os.makedirs(os.path.dirname(target), exist_ok=True)
        with open(target, "wb") as fh:
            fh.write(data)
    for root, _dirs, files in os.walk(work):
        for f in files:
            os.utime(os.path.join(root, f), (1767225600, 1767225600))
    subprocess.run(["7z", "a", "-bso0", "-bsp0", "-mtc=off", "-mta=off", path] + sorted(FILES),
                   cwd=work, check=True)


def resolve_so(build, backend):
    # 构建树里可能同时存在历史残留和自有实现（同一插件名），取最新的那份
    so = os.path.join(build, SO[backend])
    hits = glob.glob(os.path.join(build, "**", os.path.basename(so)), recursive=True)
    if not hits:
        return so
    hits.sort(key=os.path.getmtime, reverse=True)
    return hits[0]


def run(build, backend, op, sample, extra):
    binary = os.path.join(build, "bin", "tests", "conformance")
    env = dict(os.environ)
    env["LD_LIBRARY_PATH"] = os.pathsep.join(
        [os.path.join(build, "archive-api"), os.path.join(build, "3rdparty", "interface")])
    env["QT_QPA_PLATFORM"] = "offscreen"
    cmd = [binary, "--backend", backend, "--so", resolve_so(build, backend),
           "--op", op, "--sample", sample] + extra
    out = subprocess.run(cmd, env=env, capture_output=True, text=True)
    try:
        return json.loads(out.stdout), None
    except json.JSONDecodeError:
        return None, (out.stdout + out.stderr)[-400:]


def entries(path, mode="r"):
    with tarfile.open(path, mode) as tf:  # type: ignore[arg-type]
        return {m.name: (m.size, sha(tf.extractfile(m).read()) if m.isfile() else None)
                for m in tf.getmembers()}


def expect(name, got, want, results):
    ok = got == want
    results.append((name, ok, None if ok else f"期望 {want}\n      实际 {got}"))


def check_add(build, results):
    work = tempfile.mkdtemp(prefix="sem-add-")
    archive = os.path.join(work, "out.tar.gz")
    inputs = [os.path.join(CORPUS, "prefill", "plain.txt")]
    doc, err = run(build, "libarchive", "add", inputs[0],
                   ["--archive", archive, "--emit", "application/x-compressed-tar",
                    "--input", inputs[0]])
    if err or doc is None:
        return expect("add: 执行", err, "可解析的输出", results)
    expect("add: 返回", doc["result"]["finish"], 0, results)
    try:
        got = entries(archive, "r:gz")
    except Exception as e:  # noqa: BLE001
        return expect("add: 产物可读", f"损坏 {e}", "可读", results)
    want_sha = sha(open(inputs[0], "rb").read())
    expect("add: 条目", set(got), {"plain.txt"}, results)
    expect("add: 内容", got["plain.txt"][1], want_sha, results)


def check_delete(build, results, write_mode="w", read_mode="r", suffix=""):
    work = tempfile.mkdtemp(prefix="sem-del-")
    archive = os.path.join(work, f"f.tar{suffix}")
    make_fixture(archive, write_mode)
    before = entries(archive, read_mode)
    doc, err = run(build, "libarchive", "delete", archive, ["--entry", "a.txt"])
    if err or doc is None:
        return expect(f"delete{suffix}: 执行", err, "可解析的输出", results)
    expect(f"delete{suffix}: 返回", doc["result"]["finish"], 0, results)
    try:
        after = entries(archive, read_mode)
    except Exception as e:  # noqa: BLE001
        return expect(f"delete{suffix}: 产物可读", f"损坏 {e}", "可读", results)
    expect(f"delete{suffix}: 删除生效", "a.txt" not in after, True, results)
    expect(f"delete{suffix}: 其余条目保留", after, {k: v for k, v in before.items() if k != "a.txt"}, results)


def check_rename(build, results):
    work = tempfile.mkdtemp(prefix="sem-ren-")
    archive = os.path.join(work, "f.tar")
    make_fixture(archive)
    before = entries(archive)
    doc, err = run(build, "libarchive", "rename", archive,
                   ["--entry", "a.txt", "--alias", "z.txt"])
    if err or doc is None:
        return expect("rename: 执行", err, "可解析的输出", results)
    expect("rename: 返回", doc["result"]["finish"], 0, results)
    try:
        after = entries(archive)
    except Exception as e:  # noqa: BLE001
        return expect("rename: 产物可读", f"损坏 {e}", "可读", results)
    expect("rename: 新名存在", "z.txt" in after, True, results)
    expect("rename: 旧名消失", "a.txt" not in after, True, results)
    expect("rename: 内容不变", after.get("z.txt"), before["a.txt"], results)


def check_pigz_add(build, results):
    """pigz 是异步写后端：tar | pigz 子进程，结束后发 signalFinished。"""
    work = tempfile.mkdtemp(prefix="sem-pigz-")
    archive = os.path.join(work, "out.tar.gz")
    src = os.path.join(CORPUS, "prefill", "plain.txt")
    doc, err = run(build, "pigz", "add", src,
                   ["--archive", archive, "--emit", "application/x-compressed-tar", "--input", src])
    if err or doc is None:
        return expect("pigz.add: 执行", err, "可解析的输出", results)
    expect("pigz.add: 返回", doc["result"]["finish"], 0, results)
    if not os.path.exists(archive) or os.path.getsize(archive) == 0:
        return expect("pigz.add: 产物存在", "缺失或为空", "非空 tar.gz", results)
    try:
        got = entries(archive, "r:gz")
    except Exception as e:  # noqa: BLE001
        return expect("pigz.add: 产物可读", f"损坏 {e}", "可读", results)
    expect("pigz.add: 条目", set(got), {"plain.txt"}, results)
    expect("pigz.add: 内容", got["plain.txt"][1], sha(open(src, "rb").read()), results)


def check_pigz_noop(build, results):
    """pigz 的 list/extract/delete/rename 是静默空实现：返回成功但不做事。"""
    src = os.path.join(CORPUS, "sample.tar.gz")
    for op in ("list", "extract", "delete", "rename"):
        doc, err = run(build, "pigz", op, src, ["--entry", "a.txt", "--alias", "z.txt"])
        if err or doc is None:
            expect(f"pigz.{op}: 执行", err, "可解析的输出", results)
            continue
        expect(f"pigz.{op}: 返回成功", doc["result"]["finish"], 0, results)
        expect(f"pigz.{op}: 不产生条目", doc["archiveData"]["entryCount"], 0, results)


def check_zip_delete(build, results):
    work = tempfile.mkdtemp(prefix="sem-zdel-")
    archive = os.path.join(work, "f.zip")
    make_zip_fixture(archive)
    before = entries_zip(archive)
    doc, err = run(build, "libzip", "delete", archive, ["--entry", "a.txt"])
    if err or doc is None:
        return expect("zip.delete: 执行", err, "可解析的输出", results)
    expect("zip.delete: 返回", doc["result"]["finish"], 0, results)
    try:
        after = entries_zip(archive)
    except Exception as e:  # noqa: BLE001
        return expect("zip.delete: 产物可读", f"损坏 {e}", "可读", results)
    expect("zip.delete: 删除生效", "a.txt" not in after, True, results)
    expect("zip.delete: 其余保留", after, {k: v for k, v in before.items() if k != "a.txt"}, results)


def check_zip_rename(build, results):
    work = tempfile.mkdtemp(prefix="sem-zren-")
    archive = os.path.join(work, "f.zip")
    make_zip_fixture(archive)
    before = entries_zip(archive)
    doc, err = run(build, "libzip", "rename", archive, ["--entry", "a.txt", "--alias", "z.txt"])
    if err or doc is None:
        return expect("zip.rename: 执行", err, "可解析的输出", results)
    expect("zip.rename: 返回", doc["result"]["finish"], 0, results)
    try:
        after = entries_zip(archive)
    except Exception as e:  # noqa: BLE001
        return expect("zip.rename: 产物可读", f"损坏 {e}", "可读", results)
    expect("zip.rename: 新名", "z.txt" in after, True, results)
    expect("zip.rename: 旧名消失", "a.txt" not in after, True, results)
    expect("zip.rename: 内容不变", after.get("z.txt"), before["a.txt"], results)


def check_zip_add(build, results):
    work = tempfile.mkdtemp(prefix="sem-zadd-")
    archive = os.path.join(work, "out.zip")
    inputs = [os.path.join(CORPUS, "prefill", "plain.txt")]
    doc, err = run(build, "libzip", "add", inputs[0],
                   ["--archive", archive, "--emit", "application/zip", "--input", inputs[0]])
    if err or doc is None:
        return expect("zip.add: 执行", err, "可解析的输出", results)
    expect("zip.add: 返回", doc["result"]["finish"], 0, results)
    try:
        got = entries_zip(archive)
    except Exception as e:  # noqa: BLE001
        return expect("zip.add: 产物可读", f"损坏 {e}", "可读", results)
    expect("zip.add: 条目", set(got), {"plain.txt"}, results)
    expect("zip.add: 内容", got["plain.txt"][1], sha(open(inputs[0], "rb").read()), results)


def check_7z_delete(build, results):
    work = tempfile.mkdtemp(prefix="sem-7zdel-")
    archive = os.path.join(work, "f.7z")
    make_7z_fixture(archive)
    before = seven_zip_entries(archive)
    doc, err = run(build, "cli7z", "delete", archive, ["--entry", "a.txt"])
    if err or doc is None:
        return expect("7z.delete: 执行", err, "可解析的输出", results)
    expect("7z.delete: 返回", doc["result"]["finish"], 0, results)
    after = seven_zip_entries(archive)
    expect("7z.delete: 删除生效", "a.txt" not in after, True, results)
    expect("7z.delete: 其余保留", set(after), set(before) - {"a.txt"}, results)


def check_7z_rename(build, results):
    """7z 的重命名是静默空实现：返回成功，包内条目和内容都不变。"""
    work = tempfile.mkdtemp(prefix="sem-7zren-")
    archive = os.path.join(work, "f.7z")
    make_7z_fixture(archive)
    before = seven_zip_entries(archive)
    doc, err = run(build, "cli7z", "rename", archive, ["--entry", "a.txt", "--alias", "z.txt"])
    if err or doc is None:
        return expect("7z.rename: 执行", err, "可解析的输出", results)
    expect("7z.rename: 返回成功", doc["result"]["finish"], 0, results)
    expect("7z.rename: 条目不变", seven_zip_entries(archive), before, results)


def check_7z_add(build, results):
    work = tempfile.mkdtemp(prefix="sem-7zadd-")
    archive = os.path.join(work, "out.7z")
    src = os.path.join(CORPUS, "prefill", "plain.txt")
    doc, err = run(build, "cli7z", "add", src,
                   ["--archive", archive, "--emit", "application/x-7z-compressed", "--input", src])
    if err or doc is None:
        return expect("7z.add: 执行", err, "可解析的输出", results)
    expect("7z.add: 返回", doc["result"]["finish"], 0, results)
    got = seven_zip_entries(archive)
    expect("7z.add: 条目", set(got), {"plain.txt"}, results)
    expect("7z.add: 内容", seven_zip_content(archive, "plain.txt"), sha(open(src, "rb").read()), results)


def seven_zip_bytes(path, name):
    out = subprocess.run(["7z", "e", "-so", path, name], capture_output=True)
    return out.stdout


def check_7z_add_tar7z(build, results):
    """tar.7z：tar 打包后经 -si 交给 7z，产物必须能被 7z 列出且内容一致。"""
    work = tempfile.mkdtemp(prefix="sem-tar7z-")
    archive = os.path.join(work, "out.tar.7z")
    src = os.path.join(CORPUS, "prefill", "plain.txt")
    doc, err = run(build, "cli7z", "add", src,
                   ["--archive", archive, "--emit", "application/x-7z-compressed", "--input", src])
    if err or doc is None:
        return expect("7z.add.tar7z: 执行", err, "可解析的输出", results)
    expect("7z.add.tar7z: 返回", doc["result"]["finish"], 0, results)
    if not os.path.exists(archive):
        return expect("7z.add.tar7z: 产物存在", "缺失", "非空 tar.7z", results)
    got = seven_zip_entries(archive)
    # 7z 把 stdin 流当成单个条目，名字取压缩包名去掉 .7z
    expect("7z.add.tar7z: 条目", list(got), ["out.tar"], results)
    if "out.tar" not in got:
        return
    import io
    import tarfile
    with tarfile.open(fileobj=io.BytesIO(seven_zip_bytes(archive, "out.tar"))) as tf:
        expect("7z.add.tar7z: 内层 tar 条目", tf.getnames(), ["plain.txt"], results)
        expect("7z.add.tar7z: 内容", sha(tf.extractfile("plain.txt").read()),
               sha(open(src, "rb").read()), results)


def check_plugins_scan(build, results):
    """插件目录扫描：KPluginLoader::findPlugins 必须读全所有插件的元数据。"""
    work = tempfile.mkdtemp(prefix="sem-scan-")
    bin_path = os.path.join(build, "bin", "tests", "conformance")

    # 把构建树里所有插件 .so 软链进一个目录，模拟安装后的插件目录
    # 构建树里可能有历史残留（同一插件名在 3rdparty/ 和 own-*/ 各有一份），
    # 按文件名去重，保留最新的那份
    newest = {}
    for root, _dirs, files in os.walk(build):
        if "CMakeFiles" in root or "_autogen" in root:
            continue
        for name in files:
            if name.endswith(".so") and name.startswith("lib") and "plugin" in name:
                path = os.path.join(root, name)
                if name not in newest or os.path.getmtime(path) > os.path.getmtime(newest[name]):
                    newest[name] = path
    so_files = sorted(newest.values())
    if not so_files:
        return expect("plugins.scan: 找到插件", 0, ">0", results)

    scan_dir = os.path.join(work, "plugins")
    os.makedirs(scan_dir, exist_ok=True)
    for path in so_files:
        os.symlink(path, os.path.join(scan_dir, os.path.basename(path)))

    env = dict(os.environ)
    env["LD_LIBRARY_PATH"] = os.path.join(build, "archive-api")
    env["QT_QPA_PLATFORM"] = "offscreen"
    proc = subprocess.run([bin_path, "--op", "scan", "--plugin-dir", scan_dir],
                          capture_output=True, text=True, env=env)
    try:
        doc = json.loads(proc.stdout)
    except Exception as exc:  # noqa: BLE001
        return expect("plugins.scan: 输出可解析", str(exc), "JSON", results)

    plugins = doc.get("plugins", [])
    expect("plugins.scan: 数量", len(plugins), len(so_files), results)
    ids = sorted(p["id"] for p in plugins)
    expect("plugins.scan: 无空 id", [i for i in ids if not i], [], results)
    expect("plugins.scan: 名称非空", [p["id"] for p in plugins if not p["name"]], [], results)
    expect("plugins.scan: mime 非空", [p["id"] for p in plugins if not p["mimeCount"]], [], results)


def check_clipzip_add(build, results):
    """clipzip(pzip 子进程)：压缩产物必须能被标准 zip 实现读出。"""
    work = tempfile.mkdtemp(prefix="sem-clipzipadd-")
    archive = os.path.join(work, "out.zip")
    src = os.path.join(CORPUS, "prefill", "plain.txt")
    doc, err = run(build, "clipzip", "add", src,
                   ["--archive", archive, "--emit", "application/zip", "--input", src])
    if err or doc is None:
        return expect("clipzip.add: 执行", err, "可解析的输出", results)
    expect("clipzip.add: 返回", doc["result"]["finish"], 0, results)
    if not os.path.exists(archive):
        return expect("clipzip.add: 产物存在", "缺失", "非空 zip", results)

    with zipfile.ZipFile(archive) as zf:
        expect("clipzip.add: 条目", sorted(zf.namelist()), ["plain.txt"], results)
        expect("clipzip.add: 内容", sha(zf.read("plain.txt")),
               sha(open(src, "rb").read()), results)
    expect("clipzip.add: CRC 自检", zipfile.ZipFile(archive).testzip(), None, results)



def check_7z_extract_partial(build, results):
    """部分解压：只解选中的条目，落到目标路径，且不留临时目录。"""
    work = tempfile.mkdtemp(prefix="sem-7zpartial-")
    archive = os.path.join(work, "f.7z")
    make_7z_fixture(archive)
    target = os.path.join(work, "out")
    os.makedirs(target, exist_ok=True)
    doc, err = run(build, "cli7z", "extract", archive,
                   ["--partial", "--entry", "a.txt", "--entry", "sub/c.txt",
                    "--target", target, "--keep"])
    if err or doc is None:
        return expect("7z.extract.partial: 执行", err, "可解析的输出", results)

    expect("7z.extract.partial: 返回", doc["result"]["finish"], 0, results)

    landed = sorted(os.path.relpath(os.path.join(dp, f), target)
                    for dp, _, fs in os.walk(target) for f in fs)
    expect("7z.extract.partial: 落盘文件", landed, ["a.txt", os.path.join("sub", "c.txt")], results)
    expect("7z.extract.partial: a.txt 内容",
           sha(open(os.path.join(target, "a.txt"), "rb").read()),
           sha(FILES["a.txt"]), results)
    expect("7z.extract.partial: sub/c.txt 内容",
           sha(open(os.path.join(target, "sub", "c.txt"), "rb").read()),
           sha(FILES["sub/c.txt"]), results)
    left = [d for d in os.listdir(target) if d.startswith(".")]
    expect("7z.extract.partial: 不留临时目录", left, [], results)


CHECKS = {
    "add": check_add,
    "delete": check_delete,
    "delete.tbz2": lambda b, r: check_delete(b, r, "w:bz2", "r:bz2", ".bz2"),
    "rename": check_rename,
    "7z.add": check_7z_add,
    "7z.add.tar7z": check_7z_add_tar7z,
    "plugins.scan": check_plugins_scan,
    "clipzip.add": check_clipzip_add,
    "7z.delete": check_7z_delete,
    "7z.rename": check_7z_rename,
    "7z.extract.partial": check_7z_extract_partial,
    "zip.add": check_zip_add,
    "zip.delete": check_zip_delete,
    "zip.rename": check_zip_rename,
    "pigz.add": check_pigz_add,
    "pigz.noop": check_pigz_noop,
}


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("--build", required=True)
    ap.add_argument("--only", action="append")
    args = ap.parse_args()

    names = args.only or list(CHECKS)
    results = []
    for name in names:
        try:
            CHECKS[name](args.build, results)
        except Exception as e:  # noqa: BLE001
            results.append((f"{name}: 异常", False, str(e)))

    failed = 0
    for name, ok, detail in results:
        print(f"[{' OK ' if ok else 'FAIL'}] {name}")
        if not ok:
            print(f"      {detail}")
            failed += 1
    print(f"\n语义校验: {len(results) - failed}/{len(results)} 通过")
    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
