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
    ("plain() accepts NUL", "static_cast<unsigned char>(static_cast<unsigned char>(p[i]) - 1) >= 0x7f;", "static_cast<unsigned char>(p[i]) > 0x7f;"),
    ("plain() accepts non-ASCII", "static_cast<unsigned char>(static_cast<unsigned char>(p[i]) - 1) >= 0x7f;", "static_cast<unsigned char>(p[i]) == 0;"),
    ("a name is taken for ASCII", "return out.ascii || plain(out.p + from, out.pos - from);", "return true;"),
    ("a pattern is taken for ASCII", "ascii = plain(pattern.data(), pattern.size());", "ascii = true;"),
    ("a one-shot pattern is taken for ASCII", "out.ascii = plain(pattern, length);", "out.ascii = true;"),
    ("capitalize keeps a non-ASCII upper case", "if (upper > 0 && upper <= 0x7f) {", "if (upper > 0) {", "tr_TR"),
    ("reverse skips the in-place reversal", "std::reverse(p + from, p + out.pos);", ";"),
    ("collapse keeps three", "out += cnt < keep.n[static_cast<unsigned char>(ch)];", "out += cnt <= keep.n[static_cast<unsigned char>(ch)];"),
    ("collapse starts past the first pair", "out.pos = static_cast<size_t>(collapse(it - 1, end) - p);", "out.pos = static_cast<size_t>(collapse(it, end) - p);"),
    ("collapse misses a pair at the end", "for (char* it = p + from + 1; it < end; ++it) {", "for (char* it = p + from + 1; it + 1 < end; ++it) {"),
    ("a block copy stops short", "for (size_t k = 0; k < n; k += 16) {", "for (size_t k = 0; k + 16 < n; k += 16) {"),
    ("a word loses its last letter", "size_t n = t.begins[j + 1] - w;", "size_t n = t.begins[j + 1] - w - 1;"),
    ("a pick never draws again", "while (low < threshold) {", "while (false) {"),
    ("a pick takes the low half", "return static_cast<uint32_t>(m >> 32);", "return static_cast<uint32_t>(m % n);"),
    ("a group of one alternative draws", "uint32_t j = n >= 2 ? pick(e, n) : 0;", "uint32_t j = n >= 1 ? pick(e, n) : 0;"),
    ("an alternative drops its carried wrappers", "begin_alt(f, i + 1, f.pending);", "begin_alt(f, i + 1, NONE);"),
    ("a flat one-shot pattern is empty", "Group top{static_cast<uint32_t>(length), 1, component ? 1u : 0u, 0};", "Group top{static_cast<uint32_t>(length), 1, 0u, 0};"),
    ("a one-shot run swallows a wrapper", "classes_here[static_cast<unsigned char>(p[i + 1])] <= SYMBOL", "classes_here[static_cast<unsigned char>(p[i + 1])] <= REVERSE"),
    ("a run swallows a wrapped character", "if (pending.size() == pending_base) {", "if (true) {"),
    ("a symbol drops its wrappers", "\t\t\t\t\twrap_from(pending_base, from);\n\t\t\t\t}\n\t\t\t\t++pos;", "\t\t\t\t}\n\t\t\t\t++pos;"),
    ("a group drops its wrappers", "if (pending.size() != wrap_base) {", "if (false) {"),
    ("a group's end loses the group numbering", "pos = end + 1;\n\t\t\t\tnext = after;", "pos = end + 1;"),
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
