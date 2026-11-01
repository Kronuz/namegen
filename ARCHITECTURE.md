# Architecture

namegen compiles a pattern once into a table of its groups and walks the pattern's text for
every name, jumping to the alternative each choice draws. This file covers the compiled
pattern, the one-shot path, the draw, the transforms and the tests that hold them.

## The compiled pattern

`Generator(pattern)` holds one internal node, `Compiled`, with the pattern's text and two
arrays. `scan()` reads the pattern once, checking the brackets as the original parser did
(the same errors in the same order), and its `Recorder` records every group (the top level
is group 0, the others numbered in the order their brackets open) with where it ends, the
first group past it, and its alternatives; an alternative records where it starts, the first
group inside it, and where the `!` and `~` it carries in start (`!|s` capitalizes the `s`,
as in the original). The same scan counts `combinations()`, `min()` and `max()` as the
original's tree did: a choice sums, a sequence multiplies, saturating at `SIZE_MAX`.

`execute()` writes a name: from the chosen alternative of the top level it reads the text,
writing a run of plain characters at once and a symbol's word from the flattened tables
(`symbol_tables()`), and at a group it draws an alternative, jumps to it, and at the
alternative's end jumps past the group. It keeps the output position and the current group
in locals, and copies whole 16-byte blocks (`copy_blocks()`), so the tables and the compiled
text carry 16 bytes of padding and the output 16 bytes of room. Its group frames and waiting
wrappers are two explicit stacks, in place up to 32 levels and on the heap past that, so
nesting is data, not recursion. A name up to 240 bytes is written in a buffer on the stack
and copied once into the result.

`Compiled` stands for the original's root (a `Collapser` over the top group, or the top group
itself), and generators added to it join it as they joined that root, so a subclass that
reaches `generators` keeps the original's names and counts.

## The one-shot path

`NameGen::generate(pattern, engine)` records the pattern with the same `scan()` and a
`Recorder` that does not count, in arrays on the stack (on the heap past 64 groups or 256
alternatives), and takes a run of characters and symbols as one component; a pattern with no
brackets and no bars is its top level's only alternative and skips the recording. Then it
runs the same `execute()`, which reads the caller's pattern with plain `memcpy` where 16
bytes past a run are not the pattern's. Same records, same walk: it draws the same choices
from the same engine and gives the name `Generator(pattern).generate(engine)` gives.

Trees built by hand from `Random`, `Sequence`, `Literal` and the wrappers are walked by
their virtual `append(out, engine)`, which a pattern-built Generator does not use.

## Drawing a name

A choice among n >= 2 alternatives takes one 32-bit output `x` of the `std::mt19937` and
picks the high half of `x * n` (Lemire's multiply-shift), drawing again when the low half
falls among the `2^32 mod n` values that would favor some alternatives, so each has
probability exactly 1/n. A choice of one alternative and a literal character take no draw.
The compiled walk, the one-shot walk and the hand-built classes all draw this way, so they
agree for a seed, and the library computes the choice itself, so a seed gives the same
names with every compiler and standard library. `toString()` passes the thread's engine and
`generate()` the caller's.

A class derived from one of the library's classes is detected with `typeid` and produces its
own `toString()`, so a user override keeps its effect.

## Transforms

`!`, `~` and collapsing (and `Capitalizer`, `Reverser` and `Collapser`) are defined on
characters of the C locale: decode with `mbsrtowcs`, transform, encode with `wcsrtombs`.
Bytes 1 to 127 decode to themselves in every locale the C libraries ship, so text of only
those bytes is transformed in place in the output, with no conversion and no allocation. A
name from a pattern of only those bytes skips the check until a transform leaves ASCII (a
Turkish locale's `İ`). A NUL (which ends the decoded text), a
byte above 127, or a capital that `towupper` takes outside ASCII sends the text through the
conversion.

## Engines and threads

`NameGen::engine()` is a `thread_local` `std::mt19937`, seeded on a thread's first draw from
the clock and the thread's id. A compiled pattern is read-only, so one Generator serves many
threads.

## Tests

- `test/test.cc`: the syntax, counts, errors, the predefined patterns, uniform choices, the
  reusable-buffer and one-shot forms, nesting 200,000 deep, a subclass adding to a compiled
  pattern, and the names of seed 42 on four patterns (the cross-platform pin).
- `test/diff_test.cc` with `test/check.h`: every pattern through the compiled path, the
  one-shot walk, a hand-built tree of the public classes, the frozen original (`test/frozen`)
  and the reference interpreter (`test/reference.h`), seeded alike, in every locale the host
  has. Names equal the reference's everywhere, the four library forms equal each other, and
  the original's where no choice has more than one alternative; the exception and its
  message, `min()`, `max()` and `combinations()` (exact, or saturated) match.
- `test/choose_test.cc`: the choice against a stub engine: each output's alternative, the
  rejected outputs, and every alternative's share over 2^20 outputs per `n`.
- `test/thread_test.cc`: one Generator in eight threads (ThreadSanitizer in CI), seeding.
- `fuzz/fuzz_pattern.cc`: the differential check on arbitrary bytes, as libFuzzer and as the
  CTest driver `namegen_fuzz`.
- `tools/mutants.py`: breaks the draw, the recorder, the walk and the fast paths 24 ways; each
  must fail a test.
