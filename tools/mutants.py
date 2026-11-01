#!/usr/bin/env python3
"""Breaks namegen.cc in the ways listed below, one at a time, and requires the tests to
fail on each: the check that the tests see the fast paths and the fixes.

    python3 tools/mutants.py BUILD_DIR [--tsan]

BUILD_DIR is a configured build of this repository; namegen.cc is restored at the end.
Two mutants need more than a plain build: the shared engine is a data race only
ThreadSanitizer sees (pass --tsan with a -fsanitize=thread build), and the Turkish
capital I only shows in a tr_TR.UTF-8 locale (skipped where the host has none).
"""
import pathlib
import subprocess
import sys

ROOT = pathlib.Path(__file__).resolve().parent.parent
SOURCE = ROOT / "namegen.cc"

# (name, text in namegen.cc, replacement)
MUTANTS = [
    ("plain() accepts NUL", "if (c == 0 || c > 0x7f) {", "if (c > 0x7f) {"),
    ("plain() accepts non-ASCII", "if (c == 0 || c > 0x7f) {", "if (c == 0) {"),
    ("capitalize keeps a non-ASCII upper case", "if (upper > 0 && upper <= 0x7f) {", "if (upper > 0) {", "tr_TR"),
    ("reverse skips the fast path's reversal", "std::reverse(out.begin() + from, out.end());", ";"),
    ("collapse keeps three", "if (cnt < mch) {\n\t\t\t*out++ = ch;", "if (cnt <= mch) {\n\t\t\t*out++ = ch;"),
    ("a run swallows a wrapped character", "if (run && wrappers.empty()) {", "if (run) {"),
    ("a run outlives a split", "\tset.push_back(make_unique<Sequence>());\n\trun = nullptr;\n}", "\tset.push_back(make_unique<Sequence>());\n}"),
    ("a skipped draw takes one output", "e.discard(2 * static_cast<unsigned long long>(draws));", "e.discard(static_cast<unsigned long long>(draws));"),
    ("a draw drops its high half", "double u = (low + high * 4294967296.0) / 18446744073709551616.0;", "double u = (low + high * 4294967296.0 * 0) / 4294967296.0;"),
    ("choose() scales by n", "double scaled = u * static_cast<double>(n - 1) + 0.0;", "double scaled = u * static_cast<double>(n) + 0.0;"),
    ("combinations() wraps", "return a != 0 && b > static_cast<size_t>(-1) / a ? static_cast<size_t>(-1) : a * b;", "return a * b;"),
    ("one engine for every thread", "thread_local std::mt19937 rng(", "static std::mt19937 rng(", "tsan"),
]

TESTS = ["namegen_test", "namegen_diff_test", "namegen_fuzz", "namegen_thread_test"]


def run_tests(build):
    r = subprocess.run(["cmake", "--build", build, "-j4", "--target"] + TESTS, capture_output=True, text=True)
    if r.returncode != 0:
        return "build failed"
    for t in TESTS:
        r = subprocess.run([f"{build}/{t}"] + (["2000"] if t == "namegen_fuzz" else []), capture_output=True, text=True)
        if r.returncode != 0:
            return f"{t} fails"
    return None


def main():
    build = sys.argv[1]
    tsan = "--tsan" in sys.argv[2:]
    locales = subprocess.run(["locale", "-a"], capture_output=True, text=True).stdout.lower()
    turkish = "tr_tr.utf-8" in locales or "tr_tr.utf8" in locales
    original = SOURCE.read_text()
    survivors = []
    skipped = 0
    try:
        for name, old, new, *needs in MUTANTS:
            if needs and ((needs[0] == "tsan" and not tsan) or (needs[0] == "tr_TR" and not turkish)):
                print(f"skipped: {name} (needs {needs[0]})")
                skipped += 1
                continue
            if original.count(old) != 1:
                print(f"? {name}: text not found once")
                survivors.append(name)
                continue
            SOURCE.write_text(original.replace(old, new))
            caught = run_tests(build)
            print(f"{'killed' if caught else 'SURVIVED'}: {name}" + (f" ({caught})" if caught else ""))
            if not caught:
                survivors.append(name)
    finally:
        SOURCE.write_text(original)
    if survivors:
        print(f"{len(survivors)} mutants survived")
        sys.exit(1)
    print(f"all {len(MUTANTS) - skipped} mutants run were killed ({skipped} skipped)")


if __name__ == "__main__":
    main()
