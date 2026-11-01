# Frozen original

`namegen_old.h` and `namegen_old.cc` are `namegen.h` and `namegen.cc` as the
Kronuz/fantasyname fork had them at a87942a (this repository's 70bd983), with namespace
`NameGen` renamed `NameGenOld`, `towstring`/`tostring` renamed `towstring_old`/`tostring_old`
and the include renamed, so they link beside the library. They are the oracle of
`test/diff_test.cc` and the fuzz target, and the baseline of `bench/bench.cc`. Never edit them.
