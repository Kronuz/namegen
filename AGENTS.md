# AGENTS.md

Working notes for agents modifying this repository. For the design read `ARCHITECTURE.md`;
for usage read `README.md` and `docs/API.md`. This file covers the repo layout, how to build
and test, the invariants you must not break, and the traps.

## Repo map

```
namegen.h               The API: Generator, Random, Sequence, Literal, Capitalizer, Reverser, Collapser, engine(), generate(), the pattern macros.
namegen.cc              The scan and its recorder, the compiled pattern, the one-shot path, the walk (execute), the draw, the symbol tables, the transforms.
test/test.cc            Smoke test: syntax, counts, errors, predefined patterns, uniform choices, seed-42 names.
test/diff_test.cc       Differential test (CTest namegen_diff) over test/check.h, in every locale the host has.
test/check.h            One pattern through the library, test/frozen and test/reference.h, engines seeded alike.
test/reference.h        Reference interpreter of the pattern syntax: the oracle.
test/frozen/            The fork's namegen.h/.cc at a87942a, renamed NameGenOld. Never edit.
test/choose_test.cc     The choice against a stub engine (compiles namegen.cc itself).
test/thread_test.cc     One Generator in eight threads; seeded engines.
fuzz/fuzz_pattern.cc    libFuzzer target (NAMEGEN_FUZZ=ON) and, with NAMEGEN_FUZZ_DRIVER, the CTest driver namegen_fuzz.
bench/bench.cc          Compile, toString, generate and one-shot against test/frozen and the C port (NAMEGEN_BUILD_BENCH=ON).
bench/cport.c           The C port (skeeto/fantasyname c/namegen.h, fetched by the bench build) behind one function.
tools/mutants.py        Breaks the draw, the recorder, the walk and the fast paths 24 ways; the tests must fail on each.
examples/demo.cc        A tour, or a command line: namegen_demo [pattern [count]].
Makefile                The original's make build of the demo (C++11).
CMakeLists.txt          STATIC library namegen (+ namegen::namegen, fantasyname, fantasyname::fantasyname).
UNLICENSE               Public domain.
```

## Build and test

```sh
cmake -B build && cmake --build build && ctest --test-dir build
python3 tools/mutants.py build            # after touching a fast path or a fix
cmake -B bench -DCMAKE_BUILD_TYPE=Release -DNAMEGEN_BUILD_BENCH=ON && cmake --build bench
./bench/namegen_bench --check && ./bench/namegen_bench
```

The sanitizer and fuzz builds: `-DCMAKE_CXX_FLAGS="-fsanitize=address,undefined"` (Linux;
AddressSanitizer hangs on some macOS hosts), `-fsanitize=thread` for `namegen_thread`, and
Clang with `-DNAMEGEN_FUZZ=ON` for `namegen_libfuzzer`.

## Conventions

- **C++14.** The target requests `cxx_std_14`, what the code needs (`std::make_unique`); it
  also builds as C++11 through its own fallback (the Makefile does). Keep it that way.
- **No dependencies** beyond the standard library. The bench fetches the C port; the
  library never uses it.
- **Filenames are stable.** `namegen.h` and `namegen.cc` keep their names: Xapiand includes
  `"namegen.h"` and compiles `namegen.cc`.
- Tabs for indentation, double quotes in code, no em dashes in prose.

## Load-bearing invariants

- **A seed's names are the contract.** A choice among n >= 2 takes one 32-bit engine output
  through `pick()` (Lemire's multiply-shift, drawing again on a rejected output); a choice of
  one and a literal character take none. A change to the number of draws, their order, or
  the mapping changes every seeded sequence: it is a behavior change, listed in
  `CHANGELOG.md`, with `test/test.cc`'s seed-42 names updated on purpose.
- **Four forms, one name.** For an engine state, `Generator(p).generate(e)`, `generate(name,
  e)`, `NameGen::generate(p, e)` and a hand-built tree of the public classes give the same
  name; `test/check.h` checks it on every pattern. The compiled and one-shot paths share
  `scan()`, `Recorder` and `execute()`; the one-shot skips (runs of components, flat patterns)
  must record what the counting recorder records.
- **`Compiled` stands in for the original's root.** It counts, measures and draws as the
  original's `Collapser` or top group, and `add()` to it (a subclass reaching `generators`)
  joins as it joined that root.
- **ASCII in place, everything else through the conversion.** `plain()` admits bytes 1 to
  127 only; a NUL, a byte above 127, or a non-ASCII capital from `towupper` takes
  `transform_decoded()`, which must give what `towstring`/`tostring` give (empty on a
  decoding error, cut at a NUL).
- **A derived class keeps its `toString()`.** Each library class's `append()` tests
  `typeid(*this)` and falls back to `toString()` for a derived type.
- **`test/check.h` defines the differences from the original.** Names equal the
  reference's (uniform choice, the draw above) everywhere, and the original's only where no
  choice has more than one alternative; `combinations()` equals the exact count saturated at
  `SIZE_MAX`.

## Traps

- **`test/frozen/` and `test/check.h` go in one translation unit per program**: the frozen
  file's engine `rng` is file-static and the test seeds it directly.
- **`test/choose_test.cc` includes `namegen.cc`**; it must not link the library.
- **Only hand-built trees recurse.** The scan and both walks keep explicit stacks, so a
  pattern nests as deep as memory allows (`test/test.cc` draws at 200,000 levels); a tree of
  the public classes recurses once per level to draw, count and destroy. The reference
  interpreter and the frozen original recurse too, so the fuzz target caps inputs at 4096
  bytes.
- **Block copies need padding.** `copy_blocks()` reads and writes up to 15 bytes past a run:
  the symbol tables and the compiled text end in `kSlack` NULs, the walk keeps `kSlack` bytes
  of room in the output, and it copies from a caller's pattern with `memcpy` when the run is
  within `kSlack` of its end. AddressSanitizer catches a slip; a Release build does not.
- **`min()` ignores collapsing**, so a collapsed name can be shorter than `min()`.
- **The Turkish capital needs glibc's tr_TR.UTF-8.** Only there does `towupper` take `i`
  outside ASCII (macOS keeps `I`); the Linux CI jobs install `locales-all` so `namegen_diff`
  covers that path, and `tools/mutants.py` skips its mutant on hosts without the mapping.
- **The C port is a comparator, not an oracle**: it uses its own generator, so seeds do not
  match; `namegen_bench --check` checks that its names are names of the pattern.

## Provenance

The C++ port of [skeeto/fantasyname](https://github.com/skeeto/fantasyname), written by
Germán Méndez Bravo in 2015 after Christopher Wellons' JavaScript version, itself after the
RinkWorks Fantasy Name Generator. It lived in the `c++/` directory of the
[Kronuz/fantasyname](https://github.com/Kronuz/fantasyname) fork; this repository carries
that directory's history. Public domain (Unlicense).
