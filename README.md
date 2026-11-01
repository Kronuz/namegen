# namegen

Part of [Kronuz C++ Libraries](https://github.com/Kronuz/cpp-libraries), a collection of small, reusable C++ libraries.

[![CI](https://github.com/Kronuz/namegen/actions/workflows/ci.yml/badge.svg)](https://github.com/Kronuz/namegen/actions/workflows/ci.yml)
[![License: Unlicense](https://img.shields.io/badge/license-Unlicense-blue.svg)](UNLICENSE)
![C++14](https://img.shields.io/badge/C%2B%2B-14-blue.svg)

Fantasy name generator for C++14: compiles a RinkWorks-style name pattern once, then draws
names from a seedable engine. A pattern such as `!sV'i` (a capitalized syllable, a vowel or
vowel pair, an apostrophe, an insult) compiles into a tree of choices, and each name is one
walk of the tree with one draw per choice. Reach for it to name characters, places or cluster
nodes, and to get the same names again from the same seed.

## ✨ Highlights

- **The RinkWorks pattern syntax.** Thirteen symbol tables, literal `()` and symbol `<>`
  groups, `|` choices, `!` to capitalize and `~` to reverse, and nineteen ready-made
  patterns such as `MIDDLE_EARTH` and `GREEK_NAMES`.
- **Compiled once.** A `Generator` holds the pattern's tree; a name is one walk of it, with
  no heap allocation besides the returned string.
- **Seeded names.** `generate(engine)` draws from a `std::mt19937` the caller owns, and a seed
  gives the same names with every compiler and standard library (the tests pin them on
  Linux, macOS and FreeBSD).
- **Uniform choices.** Each alternative of a choice and each entry of a symbol has
  probability 1/n.
- **Shared across threads.** A compiled `Generator` is read-only, and `toString()` draws from
  the calling thread's own engine.
- **Counted.** `combinations()`, `min()` and `max()` give the pattern's size; the count
  saturates at `SIZE_MAX`.
- **Checked against the original and a reference.** A differential test and a fuzz target
  run every pattern through the library, the original C++ port and a reference interpreter
  of the syntax.

## 🚀 Quick start

```cpp
#include "namegen.h"

#include <cstdio>
#include <random>

int main() {
	NameGen::Generator generator("!sV'i");
	std::mt19937 engine(42);
	for (int i = 0; i < 3; ++i) {
		std::printf("%s\n", generator.generate(engine).c_str());
	}
	std::printf("%zu combinations\n", generator.combinations());
	// Shyu'loaf
	// Ingi'doof
	// Baney'munch
	// 118910 combinations
}
```

[`examples/demo.cc`](examples/demo.cc) prints names from several patterns;
`namegen_demo PATTERN [COUNT]` prints names from yours.

## 📦 Installation

Requires a C++14 compiler and CMake 3.14 or newer; there are no other dependencies. CI builds
and tests it on Linux (x86_64 with GCC and Clang, arm64), macOS (arm64, x86_64) and FreeBSD,
plus ASan + UBSan and TSan builds.

```cmake
include(FetchContent)
FetchContent_Declare(namegen
  GIT_REPOSITORY https://github.com/Kronuz/namegen.git
  GIT_TAG        c0f939329b39cd4b2a7d8852d08f36a5d7558de4)  # or main to track the tip
FetchContent_MakeAvailable(namegen)

target_link_libraries(my_app PRIVATE namegen::namegen)
```

```cpp
#include "namegen.h"
```

`fantasyname::fantasyname` and `fantasyname`, the target names of the
[Kronuz/fantasyname](https://github.com/Kronuz/fantasyname) fork's CMakeLists, are aliases of
`namegen::namegen`.

## 🎯 When to use it

Naming characters, places, items or servers from a pattern, at random or reproducibly from a
seed.

Not for:

- Unique identifiers: names repeat, and `combinations()` counts paths through the pattern,
  not distinct names (two paths can spell one name, and collapsing merges more).
- Untrusted patterns of any size: drawing and destroying recurse once per level of nesting,
  and a pattern nested about 110,000 levels deep exhausts an 8 MB stack (GCC 11, -O2, Linux).
- Non-ASCII text without a matching locale: `!`, `~` and collapsing decode such text in the
  current C locale (`setlocale`), and in the C locale it does not decode and the name is
  empty.
- The C or JavaScript ports' names: a seed gives different names in each, since each draws
  from its own generator.

## ⚡ Performance

Nanoseconds per name on five patterns, shortest first: the median of 9 interleaved rounds
of `namegen_bench`. `namegen` draws a name from a compiled `Generator` (`toString()`);
`one-shot` compiles the pattern and draws one name (`Generator(pattern).toString()`), the work
one call of the C port does; `C port` is the C library of
[skeeto/fantasyname](https://github.com/skeeto/fantasyname) (`c/namegen.h` at `e5a4752`),
which parses the pattern as it draws; the last column is the C port's time over namegen's
(above 1, namegen is faster). Both choose uniformly from the same tables, and
`namegen_bench --check` checks that every name either one produces is a name of the pattern;
the C port draws from its own generator, so its names for a seed differ, and it has no `S` or
`b` symbol, so the Xapiand row has no C port time.

Apple M4 Pro, macOS 26:

| pattern | namegen (ns) | one-shot (ns) | C port (ns) | × faster than the C port |
| --- | ---: | ---: | ---: | ---: |
| `sV'i` | 68 | 262 | 27 | 0.40 |
| `!<b<\|\|v\|V><\|\|s\|S\|b>>` (Xapiand's node names) | 84 | 834 | — | — |
| `GREEK_NAMES` | 95 | 1,800 | 103 | 1.09 |
| `MIDDLE_EARTH` | 99 | 3,913 | 354 | 3.6 |
| `JAPANESE_NAMES_DIVERSE` | 116 | 20,439 | 1,449 | 12.5 |

Intel Core i9-9980HK, macOS 15 x86-64:

| pattern | namegen (ns) | one-shot (ns) | C port (ns) | × faster than the C port |
| --- | ---: | ---: | ---: | ---: |
| `sV'i` | 130 | 786 | 55 | 0.43 |
| `!<b<\|\|v\|V><\|\|s\|S\|b>>` (Xapiand's node names) | 142 | 2,805 | — | — |
| `GREEK_NAMES` | 208 | 6,479 | 217 | 1.04 |
| `MIDDLE_EARTH` | 246 | 13,129 | 849 | 3.5 |
| `JAPANESE_NAMES_DIVERSE` | 309 | 72,427 | 2,996 | 9.7 |

Apple clang 17 with libc++ on both, CMake Release (`-O3`), the C port built by the same
clang. Compile once and draw many: drawing allocates nothing besides the returned string,
while compiling allocates the tree (13 allocations for `sV'i`, 1,258 for
`JAPANESE_NAMES_DIVERSE`), which makes `one-shot` 10x to 30x slower than a C port call.
[docs/perf/](docs/perf/README.md) has every row, the allocations, the A/A spread (within 3%),
a Linux GCC run and the original C++ port's numbers.

```sh
cmake -B bench -DCMAKE_BUILD_TYPE=Release -DNAMEGEN_BUILD_BENCH=ON && cmake --build bench
./bench/namegen_bench --check && ./bench/namegen_bench
```

## 📚 Documentation

- [docs/API.md](docs/API.md): the pattern syntax, the classes, engines and locales.
- [ARCHITECTURE.md](ARCHITECTURE.md): the tree, the draw, the transforms and the tests.
- [docs/perf/](docs/perf/README.md): every benchmark row, the original's numbers and the
  allocation counts.
- [CHANGELOG.md](CHANGELOG.md): dated changes, and the history of the C++ port.
- [AGENTS.md](AGENTS.md): repository map, invariants and traps.
- [Catalog entry](https://github.com/Kronuz/cpp-libraries/blob/main/AGENTS.md#namegen) in
  Kronuz C++ Libraries.

## 🛠️ Building and testing

```sh
cmake -B build && cmake --build build && ctest --test-dir build
./build/namegen_demo
cmake -B bench -DCMAKE_BUILD_TYPE=Release -DNAMEGEN_BUILD_BENCH=ON && cmake --build bench
./bench/namegen_bench
```

CTest runs the smoke test, the differential test against the original and the reference
interpreter, the choice test, the threads test and the fuzz driver; `-DNAMEGEN_FUZZ=ON`
builds the libFuzzer target with Clang. `namegen_bench` times compiling and drawing against
the original and the C port.

## 🤝 Contributing

Issues and pull requests are welcome. Read [AGENTS.md](AGENTS.md) and the catalog's
[rules of engagement](https://github.com/Kronuz/cpp-libraries/blob/main/AGENTS.md#rules-of-engagement)
first; a seed's names are part of the contract, so a change must not alter them.

## 📄 License

Public domain, under the [Unlicense](UNLICENSE). The C++ port is by Germán Méndez Bravo, with
fixes by Christopher Wellons, Raine "Gravecat" Simmons, Erich Erstu and kevin
([CHANGELOG.md](CHANGELOG.md)). It implements the pattern syntax of Christopher Wellons'
[fantasyname](https://github.com/skeeto/fantasyname), after the
[RinkWorks Fantasy Name Generator](http://www.rinkworks.com/namegen/).
