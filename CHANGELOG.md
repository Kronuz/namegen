# Changelog

Notable changes to namegen, newest first. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/); entries are dated because the
library has no releases.

Upgrading a pin from before 2026-10-30: [the consumer upgrade note](https://github.com/Kronuz/cpp-libraries/blob/main/docs/UPGRADING-2026-10.md#namegen) lists what changes for callers, how to find affected inputs, the commits not to pin, and the speedup.

## 2026-11-01

The C++ generator moves from the [Kronuz/fantasyname](https://github.com/Kronuz/fantasyname)
fork's `c++/` directory to this repository, with its history.

### Added

- `NameGen::engine()`, the calling thread's `std::mt19937`, and
  `Generator::generate(std::mt19937&)`, a name from the caller's engine. A seed gives the same
  names with every compiler and standard library.
- The CMake target `namegen::namegen`, with `fantasyname::fantasyname` and `fantasyname`
  as aliases for consumers of the fork's CMakeLists. `namegen.h` and `namegen.cc` keep their
  names.
- Tests: the smoke test, a differential test against the frozen original and a reference
  interpreter of the pattern syntax, the choice swept against a stub engine, a threads
  test, a fuzz target with its own driver, and `tools/mutants.py`. A benchmark against the
  original and the C port (`-DNAMEGEN_BUILD_BENCH=ON`), and `examples/demo.cc`.

### Changed

- Drawing a name allocates nothing on the heap beyond the returned string. Capitalizing,
  reversing and collapsing ASCII text work in place instead of through `mbsrtowcs` and
  `wcsrtombs`, and every node appends to one string; text with other bytes still takes the
  conversion. A draw computes `generate_canonical`'s value itself. Names are unchanged.
- Compiling shares the symbol tables instead of copying every entry into a `Literal` (the
  original allocated more than 115 times for one `s`), joins consecutive literal characters into one node,
  and finds symbols by character instead of by exception. Names, `min()`, `max()` and
  `combinations()` are unchanged.
- Measured with `bench/bench.cc` against the original on the same build: compiling is
  2.8x to 29x faster and a name 2.1x to 2.9x faster on an Apple M4 Pro (Apple clang 17;
  `sV'i`: 5.8 µs and 221 allocations before, 0.20 µs and 13 after; a name 191 ns before,
  68 ns after), 2.9x to 27x and 2.0x to 3.8x on an Intel Core i9-9980HK (Apple clang 17),
  and 2.3x to 43x and 2.2x to 3.9x on a Linux VM (GCC 11). A name allocates nothing besides
  the result. [docs/perf/README.md](docs/perf/README.md) has every row.

### Fixed

- Every alternative of a choice, and every entry of a symbol, has probability 1/n. The
  original rounded a draw over `[0, n - 1]` to the nearest integer, so the first and last
  alternatives came up half as often as the others (`v` gave `a` and `y` 10% of the time,
  `e`, `i`, `o` and `u` 20%). Patterns whose choices have at most two alternatives give the
  original's names for a seed; others give different names. This is a change of behavior.
- `toString()` from several threads at once raced on one shared engine; each thread has
  its own.
- `combinations()` wrapped around when the count exceeded `SIZE_MAX`; it saturates.

## History in the fantasyname fork

- 2026-06-29: `S` and `b` symbol groups (Xapiand's node names); a CMakeLists for
  FetchContent; `const` read-only methods; builds as C++14 without `HAVE_CXX14`. (Germán
  Méndez Bravo)
- 2019-12-29: a virtual destructor for `Group`. (Raine "Gravecat" Simmons)
- 2018-06-13: the engine is seeded from the high-resolution clock instead of
  `std::random_device`. (Christopher Wellons)
- 2018-05-30: `towstring()` resets its source pointer before converting. (Christopher
  Wellons)
- 2018-02-28: builds with `-Wpedantic` (Erich Erstu); a portable Makefile (Christopher
  Wellons).
- 2017-03-31: `emit` renamed `produce` for Qt, where `emit` is reserved. (kevin)
- 2016-12-02: virtual destructor for `Generator`, `const` symbol tables, missing includes,
  `make_unique` only before C++14. (Germán Méndez Bravo)
- 2016-09-08: version 1.0.1, the symbol table behind a function to avoid the static
  initialization order fiasco. (Germán Méndez Bravo)
- 2015-10-18: generators held by `std::unique_ptr`. (Germán Méndez Bravo)
- 2015-09-21: the C++ port of the generator, after the JavaScript version in
  [skeeto/fantasyname](https://github.com/skeeto/fantasyname). (Germán Méndez Bravo)
