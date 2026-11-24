#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later
"""恶意归档安全验收：路径穿越、绝对路径、符号链接链、长文件名、压缩炸弹。

用法：
  python3 tools/security/run_security_checks.py \
      --archive-cli /path/to/archive-cli \
      --workdir /tmp/dc-security
"""

from __future__ import annotations

import argparse
import io
import json
import os
import shutil
import subprocess
import sys
import tarfile
import tempfile
import zipfile
from dataclasses import dataclass, field
from pathlib import Path


@dataclass
class CaseResult:
    name: str
    passed: bool
    detail: str = ""
    extra: dict = field(default_factory=dict)


def run_cli(cli: str, operation: str, archive: Path, mime: str, target: Path | None) -> dict:
    command = [cli, "--mode", "new", "--op", operation, "--path", str(archive), "--mime", mime]
    if target is not None:
        command += ["--target", str(target), "--overwrite"]
    completed = subprocess.run(command, capture_output=True, text=True, timeout=300)
    output = completed.stdout.strip().splitlines()
    payload = {}
    for line in reversed(output):
        line = line.strip()
        if line.startswith("{"):
            try:
                payload = json.loads(line)
                break
            except json.JSONDecodeError:
                continue
    payload["_exit"] = completed.returncode
    payload["_stderr"] = completed.stderr[-2000:]
    return payload


def list_outside(outside: Path) -> list[str]:
    if not outside.exists():
        return []
    return sorted(str(path.relative_to(outside)) for path in outside.rglob("*"))


def zip_symlink_entry(archive: zipfile.ZipFile, name: str, target: str) -> None:
    info = zipfile.ZipInfo(name)
    info.create_system = 3  # Unix
    info.external_attr = (0o120777 << 16)
    archive.writestr(info, target)


def make_zip_traversal(path: Path, outside: Path) -> None:
    with zipfile.ZipFile(path, "w") as archive:
        archive.writestr("../outside/evil_traversal.txt", b"evil")
        archive.writestr(str(outside / "evil_absolute.txt"), b"evil")


def make_zip_symlink_chain(path: Path, outside: Path) -> None:
    with zipfile.ZipFile(path, "w") as archive:
        zip_symlink_entry(archive, "link", str(outside))
        archive.writestr("link/evil_symlink.txt", b"evil")
        archive.writestr("safe.txt", b"safe")


def make_tar_traversal(path: Path, outside: Path) -> None:
    with tarfile.open(path, "w") as archive:
        for name, data in [
            ("../outside/evil_traversal.txt", b"evil"),
            (str(outside / "evil_absolute.txt"), b"evil"),
        ]:
            info = tarfile.TarInfo(name)
            info.size = len(data)
            archive.addfile(info, io.BytesIO(data))


def make_tar_symlink_chain(path: Path, outside: Path) -> None:
    with tarfile.open(path, "w") as archive:
        link = tarfile.TarInfo("link")
        link.type = tarfile.SYMTYPE
        link.linkname = str(outside)
        archive.addfile(link)

        data = b"evil"
        file_info = tarfile.TarInfo("link/evil_symlink.txt")
        file_info.size = len(data)
        archive.addfile(file_info, io.BytesIO(data))

        safe = b"safe"
        safe_info = tarfile.TarInfo("safe.txt")
        safe_info.size = len(safe)
        archive.addfile(safe_info, io.BytesIO(safe))


def make_long_name_zip(path: Path, outside: Path) -> None:
    with zipfile.ZipFile(path, "w") as archive:
        archive.writestr("a" * 300 + ".txt", b"long")
        archive.writestr(str(outside / "evil_long_abs.txt"), b"evil")


def make_zip_bomb(path: Path, outside: Path, size_mb: int = 20) -> None:
    with zipfile.ZipFile(path, "w", compression=zipfile.ZIP_DEFLATED, compresslevel=9) as archive:
        with archive.open("bomb.bin", "w") as handle:
            chunk = b"\0" * (1024 * 1024)
            for _ in range(size_mb):
                handle.write(chunk)


def check_case(name: str, archive_cli: str, workdir: Path,
               archive: Path, mime: str, outside: Path,
               expected_target_file: str | None = None) -> CaseResult:
    target = workdir / "target"
    target.mkdir(parents=True, exist_ok=True)
    outside.mkdir(parents=True, exist_ok=True)
    before = set(list_outside(outside))

    payload = run_cli(archive_cli, "extract", archive, mime, target)
    after = set(list_outside(outside))
    created_outside = sorted(after - before)

    exit_code = payload.get("_exit", -1)
    result_ok = bool(payload.get("result", {}).get("ok", False))
    crashed = exit_code < 0 or exit_code >= 128

    detail = []
    passed = True
    if crashed:
        passed = False
        detail.append(f"crashed (exit={exit_code})")
    if created_outside:
        passed = False
        detail.append(f"outside writes: {created_outside}")
    if not result_ok and not crashed:
        # 提取失败本身不是安全问题，但需要记录
        detail.append(f"extract result ok=false error={payload.get('result', {}).get('error')}")
    if expected_target_file:
        expected = target / expected_target_file
        if not expected.exists():
            detail.append(f"expected target file missing: {expected_target_file}")

    return CaseResult(name=name, passed=passed, detail="; ".join(detail),
                      extra={"exit": exit_code, "outside_created": created_outside,
                             "result": payload.get("result", {})})


def main() -> int:
    parser = argparse.ArgumentParser(description="恶意归档安全验收")
    parser.add_argument("--archive-cli", required=True, help="archive-cli 可执行文件路径")
    parser.add_argument("--workdir", default=None, help="工作目录（默认临时目录）")
    parser.add_argument("--keep", action="store_true", help="保留工作目录")
    args = parser.parse_args()

    workdir = Path(args.workdir) if args.workdir else Path(tempfile.mkdtemp(prefix="dc-security-"))
    workdir.mkdir(parents=True, exist_ok=True)
    print(f"workdir: {workdir}")

    cases: list[CaseResult] = []

    # 每个 case 使用独立的 outside 目录，避免相互污染
    def case_dir(name: str) -> tuple[Path, Path]:
        base = workdir / name
        base.mkdir(parents=True, exist_ok=True)
        return base, base / "outside"

    base, outside = case_dir("zip_traversal")
    archive = base / "traversal.zip"
    make_zip_traversal(archive, outside)
    cases.append(check_case("zip_traversal", args.archive_cli, base, archive,
                            "application/zip", outside))

    base, outside = case_dir("zip_symlink_chain")
    archive = base / "symlink.zip"
    make_zip_symlink_chain(archive, outside)
    cases.append(check_case("zip_symlink_chain", args.archive_cli, base, archive,
                            "application/zip", outside, "safe.txt"))

    base, outside = case_dir("tar_traversal")
    archive = base / "traversal.tar"
    make_tar_traversal(archive, outside)
    cases.append(check_case("tar_traversal", args.archive_cli, base, archive,
                            "application/x-tar", outside))

    base, outside = case_dir("tar_symlink_chain")
    archive = base / "symlink.tar"
    make_tar_symlink_chain(archive, outside)
    cases.append(check_case("tar_symlink_chain", args.archive_cli, base, archive,
                            "application/x-tar", outside, "safe.txt"))

    # 目标目录中已存在的 symlink 文件不能被归档内容写穿
    base, outside = case_dir("zip_existing_symlink")
    target = base / "target"
    target.mkdir(parents=True, exist_ok=True)
    outside.mkdir(parents=True, exist_ok=True)
    victim = outside / "victim.txt"
    victim.write_text("original", encoding="utf-8")
    (target / "victim.txt").symlink_to(victim)
    archive = base / "attack.zip"
    with zipfile.ZipFile(archive, "w") as z:
        z.writestr("victim.txt", b"evil")
    payload = run_cli(args.archive_cli, "extract", archive, "application/zip", target)
    passed = (victim.read_text(encoding="utf-8") == "original")
    cases.append(CaseResult(
        name="zip_existing_symlink",
        passed=passed,
        detail=("existing symlink was overwritten" if not passed else ""),
        extra={"exit": payload.get("_exit"), "result": payload.get("result", {})}))

    base, outside = case_dir("long_name")
    archive = base / "long.zip"
    make_long_name_zip(archive, outside)
    cases.append(check_case("long_name", args.archive_cli, base, archive,
                            "application/zip", outside))

    base, outside = case_dir("zip_bomb")
    archive = base / "bomb.zip"
    make_zip_bomb(archive, outside)
    cases.append(check_case("zip_bomb", args.archive_cli, base, archive,
                            "application/zip", outside, "bomb.bin"))

    print("\n=== Security check summary ===")
    failed = 0
    for result in cases:
        status = "PASS" if result.passed else "FAIL"
        if not result.passed:
            failed += 1
        print(f"[{status}] {result.name}: {result.detail}")
        if result.extra:
            print(f"        exit={result.extra.get('exit')} "
                  f"outside={result.extra.get('outside_created')} "
                  f"result={result.extra.get('result')}")

    if not args.keep and args.workdir is None:
        shutil.rmtree(workdir, ignore_errors=True)

    return 1 if failed else 0


if __name__ == "__main__":
    sys.exit(main())
