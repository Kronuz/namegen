> Status: CURRENT (2026-11-01) benchmark results for namegen; maintainer: the namegen workstream

# namegen performance

`bench/bench.cc` times three operations on five patterns, for the library, the frozen
original (`test/frozen`, the fork's `c++/` at a87942a) and the C port of the generator
([skeeto/fantasyname](https://github.com/skeeto/fantasyname) `c/namegen.h` at `e5a4752`,
fetched by the bench build and compiled with the same compiler and flags as C).

- `compile`: `NameGen::Generator(pattern)`, built and destroyed.
- `toString`: one name from a compiled Generator, from the thread's engine.
- `one-shot`: `Generator(pattern).toString()`, the work one C port call does.
- `C port`: `namegen(buf, 256, pattern, &seed)`, which parses the pattern as it draws.
- `toString'`: `toString` again; its distance from `toString` is the A/A spread.

Each arm is calibrated to about 20 ms per round; the arms are interleaved within each of 9
rounds and the table reports each arm's median in ns per operation, with heap allocations
per operation counted by a replaced `operator new`. `namegen_bench --check` first checks that
every name of every arm is a name the pattern can produce (matched against the reference
interpreter's tree). The C port draws from its own generator, so its names differ from the
library's for a seed; it has no `S` or `b` symbol, so the Xapiand row has no C port arm.

```sh
cmake -B bench -DCMAKE_BUILD_TYPE=Release -DNAMEGEN_BUILD_BENCH=ON && cmake --build bench
./bench/namegen_bench --check && ./bench/namegen_bench
```

The patterns: `sV'i`; Xapiand's node names `!<b<||v|V><||s|S|b>>`; `GREEK_NAMES`;
`MIDDLE_EARTH`; `JAPANESE_NAMES_DIVERSE` (the largest predefined pattern, about 1,700 bytes).

## Batch 1: Linux x86-64 VM, GCC 11.2 (2026-11-01, main c0f9393)

> Status: CURRENT (2026-11-01) AMD EPYC 7763, 8 vCPUs of a shared VM (load about 5),
> GCC 11.2 with libstdc++, CMake Release (-O3), `quiet strict` hold; two runs. Raw output in
> the campaign's `perf/namegen/vm-linux-gcc11-run{1,2}.txt`.

Run 2, ns per operation (allocations per operation in parentheses):

| pattern | compile | original compile | toString | original toString | one-shot | C port |
| --- | ---: | ---: | ---: | ---: | ---: | ---: |
| `sV'i` | 402 (17) | 9,898 (225) | 121 (0) | 423 (5.05) | 518 | 48.4 |
| Xapiand | 1,222 (56) | 52,555 (1,193) | 148 (0) | 574 (6.88) | 1,459 | — |
| `GREEK_NAMES` | 4,895 (134) | 28,659 (781) | 193 (0) | 504 (4.61) | 3,282 | 188 |
| `MIDDLE_EARTH` | 6,211 (242) | 17,764 (835) | 210 (0) | 596 (4.83) | 6,167 | 604 |
| `JAPANESE_NAMES_DIVERSE` | 48,456 (1,278) | 112,160 (3,690) | 293 (0) | 659 (4.55) | 46,164 | 2,543 |

The A/A arm (`toString'`) is within 2% of `toString` in run 2 and within 8% in run 1. The
compile rows allocate hundreds of times and move by up to 55% between the two runs on this
shared host (`GREEK_NAMES` compile: 3,148 and 4,895 ns); the toString rows move by at most 5%
except `MIDDLE_EARTH` (290 and 210 ns).

Against the original, on the same build: compiling is 2.3x to 43x faster with 2.9x to 21x
fewer allocations, and a name is 2.2x to 3.9x faster with no allocation besides the result
(the original made 4.5 to 7 per name). Against the C port: a name from a compiled generator is
2.5x slower on `sV'i`, even on `GREEK_NAMES`, and 2.9x and 8.7x faster on `MIDDLE_EARTH` and
`JAPANESE_NAMES_DIVERSE`, where the C port rescans the whole pattern for every name; compiling
and drawing once (`one-shot`) is 10x to 18x slower than one C port call.
