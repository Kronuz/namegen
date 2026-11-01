// Differential test: the library against the frozen original (test/frozen) and the
// reference interpreter (test/reference.h), with engines seeded alike, over the
// predefined patterns, random well-formed patterns and arbitrary bytes, in every locale
// the host has of C, C.UTF-8, en_US.UTF-8, tr_TR.UTF-8 and en_US.ISO8859-1.
#include "check.h"

#include <clocale>
#include <cstdio>
#include <string>
#include <vector>

int main(int argc, char** argv) {
	int rounds = argc > 1 ? std::atoi(argv[1]) : 1;

	const std::vector<std::string> fixed = {
		"", "s", "v", "V", "sV'i", "!s(dim)", "<c|v|>!s", "~<sV'i>", "!~(foo)", "~!(foo)", "(aaa)", "(|)", "|",
		"<|>", "!<b<||v|V><||s|S|b>>", "(a|b|c|d)", "!|s", "s!", "<s!>", "~", "!~", std::string("a\0b", 3),
		"(\xc3\xa9t\xc3\xa9)", "!(\xc3\xa9t\xc3\xa9)", "~(\xc3\xa9t\xc3\xa9)", "\xff!s", "(iii)",
		"ssssssssssssssssssss", "<<<<<<<<<<v>>>>>>>>>>", MIDDLE_EARTH, JAPANESE_NAMES_CONSTRAINED,
		JAPANESE_NAMES_DIVERSE, CHINESE_NAMES, GREEK_NAMES, HAWAIIAN_NAMES_1, HAWAIIAN_NAMES_2,
		OLD_LATIN_PLACE_NAMES, DRAGONS_PERN, DRAGON_RIDERS, POKEMON, FANTASY_VOWELS_R, FANTASY_S_A,
		FANTASY_H_L, FANTASY_N_L, FANTASY_K_N, FANTASY_J_G_Z, FANTASY_K_J_Y, FANTASY_S_E,
	};

	const char* locales[] = {"C", "C.UTF-8", "en_US.UTF-8", "tr_TR.UTF-8", "en_US.ISO8859-1"};
	int ran = 0;
	for (const char* locale : locales) {
		if (!std::setlocale(LC_ALL, locale)) {
			std::printf("locale %s: not available\n", locale);
			continue;
		}
		++ran;
		auto before = check::stats();
		check::Patterns gen(12345);
		unsigned seed = 1;
		for (int round = 0; round < rounds; ++round) {
			for (auto& p : fixed) {
				check::pattern(p, true, seed++);
				check::pattern(p, false, seed++);
			}
			for (int i = 0; i < 1500; ++i) {
				auto p = gen.well_formed(3, 4, true, false);
				check::pattern(p, gen.below(2) != 0, seed++);
			}
			for (int i = 0; i < 1500; ++i) {
				// At most two alternatives per choice and no symbols: the class where every
				// pick, the original's and the uniform one, agrees.
				auto p = gen.well_formed(3, 2, false, true);
				check::pattern(p, gen.below(2) != 0, seed++);
			}
			for (int i = 0; i < 1500; ++i) {
				auto p = gen.garbage(gen.below(40));
				check::pattern(p, gen.below(2) != 0, seed++);
			}
		}
		auto& after = check::stats();
		std::printf("locale %-15s %lu patterns (%lu invalid), %lu names: %lu equal to the original, %lu differ (wider choices), %lu checked against the reference\n",
		            locale, after.patterns - before.patterns, after.invalid - before.invalid, after.names - before.names,
		            after.same_as_old - before.same_as_old, after.differ_from_old - before.differ_from_old,
		            after.reference - before.reference);
	}
	CHECK(ran >= 1);
	auto& st = check::stats();
	CHECK(st.reference > 0 && st.same_as_old > 0);
	if (check::library_pick != ref::Pick::legacy) {
		CHECK(st.differ_from_old > 0);
	}
	std::printf("all namegen differential checks passed (%lu saturated counts)\n", st.saturated);
	return 0;
}
