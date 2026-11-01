#!/usr/bin/env python3
"""Breaks namegen.cc in the ways listed below, one at a time, and requires the tests to
fail on each: the check that the tests see the fast paths and the fixes.

    python3 tools/mutants.py BUILD_DIR [--tsan]

BUILD_DIR is a configured build of this repository; namegen.cc is restored at the end.
Two mutants need more than a plain build: the shared engine is a data race only
ThreadSanitizer sees (pass --tsan with a -fsanitize=thread build), and the Turkish
capital I only shows where the C library's tr_TR.UTF-8 maps i to U+0130 (glibc with the
locale installed; macOS keeps I), so it is skipped elsewhere.
"""
import ctypes
import ctypes.util
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
    ("choose() scales by n - 1", "size_t i = static_cast<size_t>(u * static_cast<double>(n));", "size_t i = static_cast<size_t>(u * static_cast<double>(n - 1) + 0.5);"),
    ("choose() takes u = 1.0 past the end", "return i < n ? i : n - 1;", "return i;"),
    ("combinations() wraps", "return a != 0 && b > static_cast<size_t>(-1) / a ? static_cast<size_t>(-1) : a * b;", "return a * b;"),
    ("one engine for every thread", "thread_local std::mt19937 rng(", "static std::mt19937 rng(", "tsan"),
]

TESTS = ["namegen_test", "namegen_choose_test", "namegen_diff_test", "namegen_fuzz", "namegen_thread_test"]


def run_tests(build):
    r = subprocess.run(["cmake", "--build", build, "-j4", "--target"] + TESTS, capture_output=True, text=True)
    if r.returncode != 0:
        return "build failed"
    for t in TESTS:
        r = subprocess.run([f"{build}/{t}"] + (["2000"] if t == "namegen_fuzz" else []), capture_output=True, text=True)
        if r.returncode != 0:
            return f"{t} fails"
    return None


def turkish_capital_i():
    libc = ctypes.CDLL(ctypes.util.find_library("c"))
    libc.setlocale.restype = ctypes.c_char_p
    if not libc.setlocale(6 if sys.platform.startswith("linux") else 0, b"tr_TR.UTF-8"):
        return False
    return libc.towupper(ord("i")) == 0x130


def main():
    build = sys.argv[1]
    tsan = "--tsan" in sys.argv[2:]
    turkish = turkish_capital_i()
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
        subprocess.run(["cmake", "--build", build, "-j4", "--target"] + TESTS, capture_output=True)
    if survivors:
        print(f"{len(survivors)} mutants survived")
        sys.exit(1)
    print(f"all {len(MUTANTS) - skipped} mutants run were killed ({skipped} skipped)")


if __name__ == "__main__":
    main()
