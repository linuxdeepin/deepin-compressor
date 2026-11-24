#!/usr/bin/env python3
# SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
# SPDX-License-Identifier: GPL-3.0-or-later

import argparse
import os
import subprocess
import sys


def list_tests(binary: str, env: dict[str, str]) -> list[str]:
    output = subprocess.check_output([binary, "--gtest_list_tests"], env=env, text=True)
    tests: list[str] = []
    suite = ""
    for line in output.splitlines():
        if not line.startswith("  "):
            suite = line.strip()
            continue
        if suite:
            tests.append(suite + line.strip())
    return tests


def run(binary: str, test_filter: str, env: dict[str, str]) -> int:
    print(f"[isolated-test] {test_filter}", flush=True)
    return subprocess.run(
        [binary, f"--gtest_filter={test_filter}", "--gtest_brief=1"],
        env=env,
    ).returncode


def main() -> int:
    parser = argparse.ArgumentParser()
    parser.add_argument("binary")
    parser.add_argument("--isolate-suite", action="append", default=[])
    args = parser.parse_args()

    env = os.environ.copy()
    env.setdefault("QT_QPA_PLATFORM", "offscreen")

    excluded = [suite.rstrip(".") + ".*" for suite in args.isolate_suite]
    filter_arg = "--gtest_filter=" + ":".join(f"-{suite}" for suite in excluded)
    if subprocess.run([args.binary, filter_arg, "--gtest_brief=1"], env=env).returncode != 0:
        return 1

    tests = list_tests(args.binary, env)
    isolated = [
        test for test in tests
        if any(test.startswith(suite) for suite in args.isolate_suite)
    ]
    failures: list[str] = []
    for test in isolated:
        if run(args.binary, test, env) != 0:
            failures.append(test)

    print(
        f"[isolated-test] isolated={len(isolated)} failures={len(failures)}",
        flush=True,
    )
    for test in failures:
        print(f"[isolated-test] FAILED {test}", flush=True)
    return 1 if failures else 0


if __name__ == "__main__":
    sys.exit(main())
