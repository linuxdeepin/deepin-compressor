#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later
"""归档后端性能基准。

用法：
  python3 tools/perf/run_perf_bench.py \
      --archive-cli /path/to/archive-cli \
      --workdir /tmp/dc-perf \
      --iterations 3
"""

from __future__ import annotations

import argparse
import csv
import hashlib
import json
import os
import re
import shutil
import statistics
import subprocess
import sys
import tempfile
import time
from dataclasses import dataclass
from pathlib import Path


@dataclass
class Measurement:
    mode: str
    operation: str
    fmt: str
    iteration: int
    elapsed_ms: float
    peak_rss_kb: int
    archive_bytes: int
    ok: bool


def run_cli(cli: str, operation: str, archive: Path, mime: str,
            target: Path | None = None, inputs: list[Path] | None = None,
            level: int = 6, threads: int = 1) -> tuple[dict, float, int]:
    command = [cli, "--op", operation, "--path", str(archive),
               "--mime", mime, "--level", str(level), "--threads", str(threads)]
    if target is not None:
        command += ["--target", str(target), "--overwrite"]
    for item in inputs or []:
        command += ["--input", str(item)]

    time_bin = Path("/usr/bin/time")
    time_file = Path(tempfile.mktemp(prefix="dc-time-"))
    timed = ([str(time_bin), "-v", "-o", str(time_file)] + command
             if time_bin.exists() else command)
    started = time.perf_counter()
    completed = subprocess.run(timed, capture_output=True, text=True, timeout=1800)
    elapsed_ms = (time.perf_counter() - started) * 1000.0

    peak_rss = 0
    try:
        if time_bin.exists():
            time_text = time_file.read_text(errors="replace")
            match = re.search(r"Maximum resident set size \(kbytes\):\s*(\d+)", time_text)
            if match:
                peak_rss = int(match.group(1))
    finally:
        time_file.unlink(missing_ok=True)

    payload = {}
    for line in reversed(completed.stdout.strip().splitlines()):
        line = line.strip()
        if line.startswith("{"):
            try:
                payload = json.loads(line)
                break
            except json.JSONDecodeError:
                continue
    payload["_exit"] = completed.returncode
    return payload, elapsed_ms, peak_rss


def build_dataset(workdir: Path, size_mb: int, file_count: int) -> list[Path]:
    data_dir = workdir / "data"
    data_dir.mkdir(parents=True, exist_ok=True)
    files: list[Path] = []

    payload = (b"deepin-compressor benchmark payload\n" * 128)
    for index in range(file_count):
        path = data_dir / f"file_{index:04d}.txt"
        with path.open("wb") as handle:
            handle.write(payload)
        files.append(path)

    large = data_dir / "large.bin"
    with large.open("wb") as handle:
        chunk = b"\0" * (1024 * 1024)
        for _ in range(size_mb):
            handle.write(chunk)
    files.append(large)
    return files


def median(values: list[float]) -> float:
    return statistics.median(values) if values else 0.0


def file_manifest(root: Path) -> dict[str, tuple[int, str]]:
    manifest: dict[str, tuple[int, str]] = {}
    for path in sorted(root.rglob("*")):
        if path.is_file():
            data = path.read_bytes()
            manifest[str(path.relative_to(root))] = (len(data), hashlib.sha256(data).hexdigest())
    return manifest


def main() -> int:
    parser = argparse.ArgumentParser(description="归档后端性能基准")
    parser.add_argument("--archive-cli", required=True)
    parser.add_argument("--workdir", default=None)
    parser.add_argument("--iterations", type=int, default=3)
    parser.add_argument("--size-mb", type=int, default=8)
    parser.add_argument("--file-count", type=int, default=50)
    parser.add_argument("--output", default=None, help="CSV 输出路径")
    parser.add_argument("--keep", action="store_true")
    args = parser.parse_args()

    workdir = Path(args.workdir) if args.workdir else Path(tempfile.mkdtemp(prefix="dc-perf-"))
    workdir.mkdir(parents=True, exist_ok=True)
    print(f"workdir: {workdir}")

    files = build_dataset(workdir, args.size_mb, args.file_count)
    print(f"dataset: {len(files)} files, {args.size_mb} MiB large file")

    formats = [
        ("zip", "application/zip", ".zip"),
        ("7z", "application/x-7z-compressed", ".7z"),
        ("tar.gz", "application/x-compressed-tar", ".tar.gz"),
        ("tar.bz2", "application/x-bzip-compressed-tar", ".tar.bz2"),
        ("tar.xz", "application/x-xz-compressed-tar", ".tar.xz"),
    ]
    mode = "new"
    data_dir = workdir / "data"
    source_manifest = file_manifest(data_dir)
    measurements: list[Measurement] = []
    failures: list[str] = []

    for fmt, mime, suffix in formats:
        reference: Path | None = None
        for iteration in range(args.iterations):
            archive = workdir / f"{fmt}-{mode}.{iteration}{suffix}"
            archive.unlink(missing_ok=True)
            payload, elapsed, peak = run_cli(
                args.archive_cli, "create", archive, mime,
                inputs=files, level=6, threads=1)
            ok = (payload["_exit"] == 0 and bool(payload.get("result", {}).get("ok"))
                  and archive.exists() and archive.stat().st_size > 0)
            if ok:
                target = workdir / "validate-create"
                shutil.rmtree(target, ignore_errors=True)
                target.mkdir()
                validation, _, _ = run_cli(args.archive_cli, "extract",
                                           archive, mime, target=target)
                ok = (validation["_exit"] == 0 and bool(validation.get("result", {}).get("ok"))
                      and file_manifest(target) == source_manifest)
                shutil.rmtree(target)
            if not ok:
                failures.append(f"create failed/content mismatch: {fmt}/{mode}/#{iteration}")
            measurements.append(Measurement(
                mode=mode, operation="create", fmt=fmt, iteration=iteration,
                elapsed_ms=elapsed, peak_rss_kb=peak,
                archive_bytes=archive.stat().st_size if archive.exists() else 0,
                ok=ok))
            if ok:
                reference = archive
            print(f"create {fmt:8s} {mode:6s} #{iteration}: {elapsed:8.1f} ms "
                  f"rss={peak:7d} KB size={archive.stat().st_size if archive.exists() else 0} ok={ok}")

        if not reference or not reference.exists():
            failures.append(f"no reference archive: {fmt}")
            continue

        for iteration in range(args.iterations):
            target = workdir / f"extract-{fmt}-{mode}-{iteration}"
            shutil.rmtree(target, ignore_errors=True)
            target.mkdir(parents=True, exist_ok=True)
            payload, elapsed, peak = run_cli(
                args.archive_cli, "extract", reference, mime, target=target)
            extracted_manifest = file_manifest(target)
            ok = (payload["_exit"] == 0 and bool(payload.get("result", {}).get("ok"))
                  and extracted_manifest == source_manifest)
            if not ok:
                failures.append(f"extract failed/content mismatch: {fmt}/{mode}/#{iteration}")
            measurements.append(Measurement(
                mode=mode, operation="extract", fmt=fmt, iteration=iteration,
                elapsed_ms=elapsed, peak_rss_kb=peak,
                archive_bytes=reference.stat().st_size,
                ok=ok))
            print(f"extract {fmt:8s} {mode:6s} #{iteration}: {elapsed:8.1f} ms "
                  f"rss={peak:7d} KB ok={ok}")
            shutil.rmtree(target, ignore_errors=True)

    output = Path(args.output) if args.output else workdir / "perf-results.csv"
    with output.open("w", newline="", encoding="utf-8") as handle:
        writer = csv.writer(handle)
        writer.writerow(["mode", "operation", "format", "iteration", "elapsed_ms",
                         "peak_rss_kb", "archive_bytes", "ok"])
        for item in measurements:
            writer.writerow([item.mode, item.operation, item.fmt, item.iteration,
                             f"{item.elapsed_ms:.1f}", item.peak_rss_kb,
                             item.archive_bytes, item.ok])

    print("\n=== Summary (median) ===")
    print(f"{'format':10s} {'op':8s} {'median_ms':>10s}")
    for fmt, _, _ in formats:
        for operation in ("create", "extract"):
            values = [m.elapsed_ms for m in measurements
                      if m.fmt == fmt and m.operation == operation and m.mode == mode and m.ok]
            print(f"{fmt:10s} {operation:8s} {median(values):10.1f}")

    print(f"\nCSV: {output}")
    if failures:
        print("Failures:")
        for failure in failures:
            print("  -", failure)

    if not args.keep and args.workdir is None:
        shutil.rmtree(workdir, ignore_errors=True)

    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
