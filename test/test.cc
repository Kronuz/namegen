// Smoke test: the pattern syntax, the generator tree's counts and lengths, errors.
#include "namegen.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <random>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <vector>

static int checks = 0;
#define CHECK(x) do { ++checks; if (!(x)) { std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #x); std::abort(); } } while (0)

static std::string error_of(const std::string& pattern) {
	try {
		NameGen::Generator g(pattern);
	} catch (const std::invalid_argument& e) {
		return e.what();
	}
	return "";
}

// Every name `pattern` gives in `draws` calls.
static std::set<std::string> names(const std::string& pattern, int draws = 2000, bool collapse = true) {
	NameGen::Generator g(pattern, collapse);
	std::set<std::string> seen;
	for (int i = 0; i < draws; ++i) {
		seen.insert(g.toString());
	}
	return seen;
}

int main() {
	using NameGen::Generator;

	// Literal groups, capitalization, reversal.
	CHECK(Generator("(foo)").toString() == "foo");
	CHECK(Generator("!(foo)").toString() == "Foo");
	CHECK(Generator("~(foo)").toString() == "oof");
	CHECK(Generator("!~(foo)").toString() == "Oof");
	CHECK(Generator("~!(foo)").toString() == "ooF");
	CHECK(Generator("(foo|foo)").toString() == "foo");
	CHECK(Generator("(a!b~c)").toString() == "a!b~c");  // ! and ~ are literal inside ()
	CHECK(Generator("x'y").toString() == "x'y");        // non-symbol letters are literal
	CHECK(Generator("").toString().empty());
	CHECK(Generator("<>").toString().empty());
	CHECK(Generator("()").toString().empty());
	CHECK(Generator("(|)").toString().empty());
	CHECK(names("(foo|bar)") == (std::set<std::string>{"foo", "bar"}));
	CHECK(names("<c|v|>").count(""));
	CHECK(names("~<(ab)(cd)>") == (std::set<std::string>{"dcba"}));
	CHECK(names("~(ab)(cd)") == (std::set<std::string>{"bacd"}));

	// Symbols.
	auto vowels = names("v");
	CHECK(vowels == (std::set<std::string>{"a", "e", "i", "o", "u", "y"}));
	for (auto& n : names("!v")) {
		CHECK(n.size() == 1 && n[0] >= 'A' && n[0] <= 'Z');
	}
	const auto& symbols = Generator::SymbolMap();
	CHECK(symbols.size() == 13);
	for (const char* k : {"s", "S", "b", "v", "V", "c", "B", "C", "i", "m", "M", "D", "d"}) {
		CHECK(symbols.count(k) == 1);
	}
	for (auto& n : names("s", 5000)) {
		auto& s = symbols.at("s");
		CHECK(std::find(s.begin(), s.end(), n) != s.end());
	}

	// Every alternative of a choice, and every entry of a symbol, is equally likely.
	{
		std::mt19937 engine(7);
		Generator choice("(a|b|c|d|f)");
		Generator vowel("v");
		std::map<std::string, int> seen;
		const int draws = 60000;
		for (int i = 0; i < draws; ++i) {
			++seen[choice.generate(engine)];
			++seen[vowel.generate(engine)];
		}
		CHECK(seen.size() == 5 + 6 - 1);  // "a" is a choice and a vowel
		for (const char* n : {"b", "c", "d", "f"}) {
			CHECK(seen[n] > draws / 5 * 95 / 100 && seen[n] < draws / 5 * 105 / 100);
		}
		for (const char* n : {"e", "i", "o", "u", "y"}) {
			CHECK(seen[n] > draws / 6 * 95 / 100 && seen[n] < draws / 6 * 105 / 100);
		}
	}

	// A seed gives the same names on every platform and standard library: the draw is
	// the library's own arithmetic over std::mt19937's outputs.
	{
		auto first4 = [](const char* pattern) {
			Generator g(pattern);
			std::mt19937 engine(42);
			std::string names;
			for (int i = 0; i < 4; ++i) {
				names += (i ? " " : "") + g.generate(engine);
			}
			return names;
		};
		CHECK(first4("!sV'i") == "Shyu'loaf Ingi'doof Baney'munch Uska'lump");
		CHECK(first4("!<b<||v|V><||s|S|b>>") == "Selna Liamest Crisavei Baleubai");
		CHECK(first4(MIDDLE_EARTH) == "ereadurta bren gororbistor brenorbisgil");
		CHECK(first4(GREEK_NAMES) == "creudios yqios mygios pheybyos");
	}

	// Collapsing runs of three: one of a, h, i, j, q, u, v, w, x, y; two of anything else.
	CHECK(Generator("(aaa)").toString() == "a");
	CHECK(Generator("(bbbb)").toString() == "bb");
	CHECK(Generator("(abba)").toString() == "abba");
	CHECK(Generator("(AAA)").toString() == "AA");
	CHECK(Generator("(aaa)", false).toString() == "aaa");

	// combinations(), min(), max().
	CHECK(Generator("v").combinations() == 6);
	CHECK(Generator("V").combinations() == 22);
	CHECK(Generator("sv").combinations() == symbols.at("s").size() * 6);
	CHECK(Generator("<v|c>").combinations() == 6 + 21);
	CHECK(Generator("(a|b|c)").combinations() == 3);
	CHECK(Generator("(foo)").combinations() == 1);
	CHECK(Generator("").combinations() == 1);
	CHECK(Generator("ssssssssssssssssssss").combinations() == static_cast<size_t>(-1));  // 115^20 saturates
	CHECK(Generator("(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)"
	                "(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)"
	                "(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)"
	                "(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)(a|b)").combinations()
	      == (static_cast<size_t>(1) << 63));                                                // 2^63 is exact

	CHECK(Generator("s").min() == 2);
	CHECK(Generator("s").max() == 4);
	CHECK(Generator("(a|bcd)v").min() == 2);
	CHECK(Generator("(a|bcd)v").max() == 4);
	CHECK(Generator("").min() == 0 && Generator("").max() == 0);
	CHECK(Generator().toString().empty() && Generator().combinations() == 1);

	// Errors.
	CHECK(error_of(")") == "Unbalanced brackets");
	CHECK(error_of(">") == "Unbalanced brackets");
	CHECK(error_of("(") == "Missing closing bracket");
	CHECK(error_of("<s") == "Missing closing bracket");
	CHECK(error_of("<)") == "Unexpected ')' in pattern");
	CHECK(error_of("(>") == "Unexpected '>' in pattern");
	CHECK(error_of("s(v)<c>").empty());

	// The predefined patterns compile and produce names.
	for (const char* p : {MIDDLE_EARTH, JAPANESE_NAMES_CONSTRAINED, JAPANESE_NAMES_DIVERSE, CHINESE_NAMES,
	                      GREEK_NAMES, HAWAIIAN_NAMES_1, HAWAIIAN_NAMES_2, OLD_LATIN_PLACE_NAMES, DRAGONS_PERN,
	                      DRAGON_RIDERS, POKEMON, FANTASY_VOWELS_R, FANTASY_S_A, FANTASY_H_L, FANTASY_N_L,
	                      FANTASY_K_N, FANTASY_J_G_Z, FANTASY_K_J_Y, FANTASY_S_E}) {
		Generator g(p);
		CHECK(g.combinations() > 1);
		for (int i = 0; i < 100; ++i) {
			auto n = g.toString();
			CHECK(!n.empty() && n.size() <= g.max());
		}
	}

	// Composing a tree by hand.
	{
		std::vector<std::unique_ptr<Generator>> alternatives;
		alternatives.emplace_back(new NameGen::Literal("x"));
		alternatives.emplace_back(new NameGen::Literal("yz"));
		NameGen::Random r(std::move(alternatives));
		CHECK(r.combinations() == 2 && r.min() == 1 && r.max() == 2);
		Generator root;
		root.add(std::unique_ptr<Generator>(new NameGen::Literal("<")));
		root.add(std::unique_ptr<Generator>(new NameGen::Capitalizer(std::unique_ptr<Generator>(new NameGen::Literal("ab")))));
		root.add(std::unique_ptr<Generator>(new NameGen::Reverser(std::unique_ptr<Generator>(new NameGen::Literal("cd")))));
		CHECK(root.toString() == "<Abdc");
		CHECK(root.min() == 5 && root.max() == 5 && root.combinations() == 1);
	}

	// The wide-string helpers.
	CHECK(towstring("abc") == L"abc");
	CHECK(tostring(L"abc") == "abc");

	std::printf("all namegen tests passed (%d checks)\n", checks);
	return 0;
}
