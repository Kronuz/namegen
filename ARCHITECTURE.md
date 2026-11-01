# Architecture

namegen compiles a pattern once into a tree of generators and walks the tree for every name.
This file covers the tree, the draw, the transforms and the tests that hold them.

## The tree

`Generator(pattern)` scans the pattern once with a stack of groups. A group collects
alternatives, each a `Sequence`; `|` starts a new one, `!` and `~` push a wrapper that the next
component takes, and a closing bracket turns the group into one node: `Literal("")` for an
empty group, its single `Sequence`, or a `Random` over the alternatives. The root is a
`Collapser` over the top group unless `collapse_triples` is false.

The leaves are two internal node types in `namegen.cc`: a symbol is one node over the shared
table of `SymbolMap()` (found by character in a 256-entry array), and consecutive literal
characters are one run node. The original built a `Random` of one `Literal` per table entry,
and one per character; the internal nodes count, measure and draw as those did, so
`combinations()`, `min()`, `max()` and every name are unchanged. Nothing outside the compiler
reaches them: a caller sees the root and what `generators` holds, which keep the public types.

## Drawing a name

Every node appends to one `std::string` through the protected virtual
`append(out, engine)`. `toString()` passes the thread's engine and `generate()` the caller's.
A class derived from one of the library's classes is detected with `typeid` and produces its
own `toString()`, so a user override keeps its effect.

Every choice takes exactly one draw of two `std::mt19937` outputs, `u = (x1 + x2·2^32) / 2^64`,
the value `std::generate_canonical<double>` takes in libc++, libstdc++ and the MSVC STL, and
picks `floor(u·n)` (the last alternative when `u` rounds to 1.0). A literal character takes a
draw too and discards it, as the original's one-alternative `Random` did, so the engine moves
the same way for a pattern whatever it chooses. The library computes `u` itself, so a seed
gives the same names everywhere, and libstdc++'s run-time logarithms are skipped.

## Transforms

`Capitalizer`, `Reverser` and `Collapser` are defined on characters of the C locale: decode
with `mbsrtowcs`, transform, encode with `wcsrtombs`. Bytes 1 to 127 decode to themselves in
every locale the C libraries ship, so text of only those bytes is transformed in place in the
output string, with no conversion and no allocation. A NUL (which ends the decoded text), a
byte above 127, or a capital that `towupper` takes outside ASCII sends the text through the
conversion.

## Engines and threads

`NameGen::engine()` is a `thread_local` `std::mt19937`, seeded on a thread's first draw from
the clock and the thread's id. A compiled tree is read-only, so one Generator serves many
threads.

## Tests

- `test/test.cc`: the syntax, counts, errors, the predefined patterns, uniform choices, and
  the names of seed 42 on four patterns (the cross-platform pin).
- `test/diff_test.cc` with `test/check.h`: every pattern through the library, the frozen
  original (`test/frozen`) and the reference interpreter (`test/reference.h`), seeded alike,
  in every locale the host has. Names equal the original's where every choice has at most two
  alternatives and the reference's everywhere; the exception and its message, `min()`,
  `max()` and `combinations()` (exact, or saturated) match.
- `test/choose_test.cc`: the choice against a stub engine, swept over 2^20 draws per `n`.
- `test/thread_test.cc`: one Generator in eight threads (ThreadSanitizer in CI), seeding.
- `fuzz/fuzz_pattern.cc`: the differential check on arbitrary bytes, as libFuzzer and as the
  CTest driver `namegen_fuzz`.
- `tools/mutants.py`: breaks the fast paths and fixes 13 ways; each must fail a test.
