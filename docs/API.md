# namegen API

Everything is in `namegen.h`, namespace `NameGen`, with two helpers in the global namespace.

## Patterns

A pattern is a template. At the top level and inside `<>`, these letters are replaced by a
random entry of their table (`Generator::SymbolMap()`); every other character is itself.

| Symbol | Table | Entries | Examples |
| --- | --- | ---: | --- |
| `s` | generic syllable | 115 | `ach`, `dra`, `ough` |
| `S` | syllable ending in a vowel | 217 | `bra`, `gio` |
| `b` | syllable ending in a consonant | 365 | `bal`, `dier` |
| `v` | vowel | 6 | `a`, `y` |
| `V` | vowel or vowel pair | 22 | `ae`, `ou` |
| `c` | consonant | 21 | `b`, `z` |
| `B` | consonant or cluster to begin a word | 43 | `chr`, `sw` |
| `C` | consonant or cluster anywhere in a word | 36 | `ck`, `nd` |
| `i` | insult | 47 | `dolt`, `twit` |
| `m` | mushy name | 43 | `snookum` |
| `M` | mushy name ending | 23 | `cakes` |
| `D` | consonant for a stupid person's name | 21 | `gl`, `th` |
| `d` | syllable for a stupid person's name (begins with a vowel) | 36 | `oob`, `ump` |

| Syntax | Meaning | Example |
| --- | --- | --- |
| `(...)` | literal group: every character is itself | `s(dim)`: a syllable, then `dim` |
| `<...>` | pattern group: symbols as above | `(foo)<v\|c>` |
| `\|` | a choice between the alternatives of a group, each equally likely | `(foo\|bar)` |
| `!` | capitalize the next component (outside `()`) | `!s`; `v!s` gives `eRod` |
| `~` | reverse the next component (outside `()`) | `~(foo)` gives `oof`; `~<sV'i>` |

Empty groups and alternatives are allowed: `<c|v|>` is a consonant, a vowel, or nothing.
Wrappers nest: `!~(foo)` gives `Oof` and `~!(foo)` gives `ooF`. Inside `()`, `!` and `~` are
characters. The constructor throws `std::invalid_argument`: `"Unbalanced brackets"` for a `)`
or `>` that closes nothing, `"Unexpected ')' in pattern"` or `"Unexpected '>' in pattern"`
for one that closes the other kind of group, and `"Missing closing bracket"` for a group left
open.

By default a name's runs are collapsed: at most one of `a h i j q u v w x y` in a row and at
most two of any other character (`(aaa)` gives `a`, `(bbbb)` gives `bb`).

The header defines nineteen patterns as macros: `MIDDLE_EARTH`, `JAPANESE_NAMES_CONSTRAINED`,
`JAPANESE_NAMES_DIVERSE`, `CHINESE_NAMES`, `GREEK_NAMES`, `HAWAIIAN_NAMES_1`,
`HAWAIIAN_NAMES_2`, `OLD_LATIN_PLACE_NAMES`, `DRAGONS_PERN`, `DRAGON_RIDERS`, `POKEMON`, and
eight `FANTASY_*` sets (`FANTASY_VOWELS_R`, `FANTASY_S_A`, `FANTASY_H_L`, `FANTASY_N_L`,
`FANTASY_K_N`, `FANTASY_J_G_Z`, `FANTASY_K_J_Y`, `FANTASY_S_E`).

## Generator

```cpp
NameGen::Generator(const std::string& pattern, bool collapse_triples = true);
std::string toString() const;                      // a name from this thread's engine()
std::string generate(std::mt19937& engine) const;  // a name from the caller's engine
void generate(std::string& name) const;            // the same, written over name, reusing its storage
void generate(std::string& name, std::mt19937& engine) const;
size_t combinations() const;  // paths through the pattern; SIZE_MAX when they do not fit
size_t min() const;           // shortest name in bytes, before collapsing
size_t max() const;           // longest name in bytes
void add(std::unique_ptr<Generator>&& g);          // appends a generator to this one
static const std::unordered_map<std::string, const std::vector<std::string>>& SymbolMap();
```

A Generator does not change once built, so one Generator serves any number of threads.
A choice among n >= 2 alternatives takes one 32-bit engine output, and more only when that
output is rejected (`2^32 mod n` of the 2^32 values are, so that each alternative has
probability exactly 1/n); a choice of one alternative and a literal character take none.

## One name without compiling

```cpp
std::string NameGen::generate(const std::string& pattern, std::mt19937& engine, bool collapse_triples = true);
std::string NameGen::generate(const char* pattern, std::mt19937& engine, bool collapse_triples = true);
void NameGen::generate(std::string& name, const char* pattern, size_t length, std::mt19937& engine,
                       bool collapse_triples = true);
```

For a pattern used once: it reads the pattern's brackets, then walks the pattern, finding
each group's alternatives as it enters the group. For the same engine state it gives the
name `Generator(pattern, collapse_triples).generate(engine)` gives, and it throws the same
`std::invalid_argument` for a malformed pattern. A Generator is faster from the second name
on.

## Engines

```cpp
std::mt19937& NameGen::engine();  // this thread's engine
```

Each thread has its own engine, seeded from the clock and the thread's id the first time the
thread draws. `NameGen::engine().seed(n)` makes the thread's next names repeat;
`generate(engine)` draws from an engine the caller owns. A seed gives the same names with
every compiler and standard library, because the library computes each choice from the
engine's outputs itself (`test/test.cc` pins four patterns at seed 42). `NameGen::generate()`
and a Generator agree for a seed.

## Building a tree by hand

A tree of these classes can be composed by hand; it draws as a compiled pattern does, so a
tree that spells a pattern gives that pattern's names for a seed:

| Class | Produces |
| --- | --- |
| `Random(std::vector<std::unique_ptr<Generator>>&&)` | one of its generators, each equally likely; `combinations()` is their sum |
| `Sequence(...)`, `Generator()` | all of its generators in order; `combinations()` is their product |
| `Literal(const std::string&)` | its text |
| `Capitalizer(std::unique_ptr<Generator>&&)` | its generator's output, first character upper-cased with `towupper` |
| `Reverser(std::unique_ptr<Generator>&&)` | its generator's output reversed by character |
| `Collapser(std::unique_ptr<Generator>&&)` | its generator's output with its runs collapsed |

A class derived from one of these produces its own `toString()`, so an override of
`toString()` keeps its effect; such a class draws from the thread's engine, also inside
`generate()`.

## Characters and locales

Capitalizer, Reverser and Collapser work on characters. Text of ASCII bytes (1 to 127) is
transformed as bytes. Other text is decoded with `mbsrtowcs` in the current C locale
(`setlocale`), transformed, and encoded back with `wcsrtombs`: in a UTF-8 locale `~(été)`
reverses three characters; in the C locale bytes above 127 do not decode and the result is
empty; a NUL ends the text. `towupper` follows the locale too: where the C library maps a
Turkish locale's `i` to `İ`, `!(i)` gives `İ`.

```cpp
std::wstring towstring(const std::string& s);  // "" when s does not decode
std::string tostring(const std::wstring& s);   // "" when s does not encode
```
