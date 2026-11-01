// The differential check shared by test/diff_test.cc and fuzz/fuzz_pattern.cc: one
// pattern through the library, the frozen original (test/frozen) and the reference
// interpreter (test/reference.h), with engines seeded alike.
//
// Include this from one translation unit per program: it includes the frozen
// original's namegen_old.cc, whose engine `rng` is static to that file.
#pragma once

#include "frozen/namegen_old.cc"
#include "namegen.h"
#include "reference.h"

#include <clocale>
#include <cstdio>
#include <cstdlib>
#include <memory>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #x); std::abort(); } } while (0)

namespace check {

// The library's mapping from a draw to an alternative.
constexpr ref::Pick library_pick = ref::Pick::lemire;

struct Stats {
	unsigned long patterns = 0, invalid = 0, names = 0, same_as_old = 0, differ_from_old = 0, reference = 0;
	unsigned long saturated = 0;
};

inline Stats& stats() {
	static Stats s;
	return s;
}

inline bool ascii(const std::string& p) {
	for (unsigned char c : p) {
		if (c == 0 || c > 0x7f) {
			return false;
		}
	}
	return true;
}

// The most alternatives any one draw chooses among.
inline std::size_t widest_choice(const ref::Node& n) {
	std::size_t w = n.kind == ref::Node::Symbol ? n.table->size() : n.kind == ref::Node::Choice ? n.kids.size() : 1;
	for (auto& k : n.kids) {
		std::size_t v = widest_choice(k);
		w = v > w ? v : w;
	}
	return w;
}

inline bool c_locale() {
	const char* l = std::setlocale(LC_CTYPE, nullptr);
	return l && (std::string(l) == "C" || std::string(l) == "POSIX");
}

template <typename G>
std::unique_ptr<G> build(const std::string& pattern, bool collapse, std::string& error) {
	try {
		return std::unique_ptr<G>(new G(pattern, collapse));
	} catch (const std::invalid_argument& e) {
		error = e.what();
	}
	return nullptr;
}

// The reference's tree built from the library's public classes (Random, Sequence,
// Literal, Capitalizer, Reverser, Collapser): the library's other way to draw a name.
inline std::unique_ptr<NameGen::Generator> tree(const ref::Node& n) {
	std::vector<std::unique_ptr<NameGen::Generator>> kids;
	switch (n.kind) {
		case ref::Node::Char:
			return std::unique_ptr<NameGen::Generator>(new NameGen::Literal(std::string(1, n.c)));
		case ref::Node::Empty:
			return std::unique_ptr<NameGen::Generator>(new NameGen::Literal(""));
		case ref::Node::Symbol:
			for (auto& w : *n.table) {
				kids.emplace_back(new NameGen::Literal(w));
			}
			return std::unique_ptr<NameGen::Generator>(new NameGen::Random(std::move(kids)));
		case ref::Node::Sequence:
		case ref::Node::Choice:
			for (auto& k : n.kids) {
				kids.push_back(tree(k));
			}
			if (n.kind == ref::Node::Choice) {
				return std::unique_ptr<NameGen::Generator>(new NameGen::Random(std::move(kids)));
			}
			return std::unique_ptr<NameGen::Generator>(new NameGen::Sequence(std::move(kids)));
		case ref::Node::Capitalize:
			return std::unique_ptr<NameGen::Generator>(new NameGen::Capitalizer(tree(n.kids[0])));
		case ref::Node::Reverse:
			return std::unique_ptr<NameGen::Generator>(new NameGen::Reverser(tree(n.kids[0])));
	}
	return nullptr;
}

// Checks one pattern with `seeds` engines and `names` names from each: the compiled
// Generator, the one-shot NameGen::generate(), the tree of public classes and (ASCII,
// C locale) the reference must give the same names for the same engine state; the
// frozen original must give them too where the pattern makes no choice.
inline void pattern(const std::string& p, bool collapse, unsigned seed, int seeds = 2, int names = 4) {
	auto& st = stats();
	++st.patterns;
	std::string old_error, new_error, ref_error, one_error;
	auto o = build<NameGenOld::Generator>(p, collapse, old_error);
	auto n = build<NameGen::Generator>(p, collapse, new_error);
	std::unique_ptr<ref::Pattern> r;
	try {
		r.reset(new ref::Pattern(p));
	} catch (const std::invalid_argument& e) {
		ref_error = e.what();
	}
	try {
		std::mt19937 any(seed);
		NameGen::generate(p, any, collapse);
	} catch (const std::invalid_argument& e) {
		one_error = e.what();
	}
	CHECK(old_error == new_error);
	CHECK(ref_error == new_error);
	CHECK(one_error == new_error);
	CHECK(!o == !n && !n == !r);
	if (!n) {
		++st.invalid;
		return;
	}

	CHECK(n->min() == o->min() && n->min() == ref::min_length(r->root));
	CHECK(n->max() == o->max() && n->max() == ref::max_length(r->root));
	// combinations() is the exact count, or the largest size_t when the count does not
	// fit; the original wrapped around there.
	ref::wide exact = ref::combinations(r->root);
	CHECK(n->combinations() == static_cast<std::size_t>(exact));
	if (exact < static_cast<ref::wide>(std::numeric_limits<std::size_t>::max())) {
		CHECK(n->combinations() == o->combinations());
	} else {
		++st.saturated;
	}

	NameGen::Generator t;
	t.add(collapse ? std::unique_ptr<NameGen::Generator>(new NameGen::Collapser(tree(r->root))) : tree(r->root));
	CHECK(t.combinations() == n->combinations() && t.min() == n->min() && t.max() == n->max());

	const bool same_as_old = widest_choice(r->root) <= 1;  // no choice: no draw anywhere
	const bool with_reference = ascii(p) && c_locale();
	std::string one, reused;
	for (int s = 0; s < seeds; ++s) {
		std::mt19937::result_type value = seed * 7919u + static_cast<unsigned>(s);
		rng.seed(value);                    // the frozen original's engine
		std::mt19937 e(value), e_ref(value), e_tree(value), e_one(value), e_reused(value);
		NameGen::engine().seed(value);      // this thread's engine, for toString()
		std::mt19937 e_again(value);
		for (int k = 0; k < names; ++k) {
			std::string a = o->toString();
			std::string b = n->generate(e);
			++st.names;
			CHECK(n->toString() == n->generate(e_again));
			CHECK(t.generate(e_tree) == b);
			NameGen::generate(one, p.data(), p.size(), e_one, collapse);
			CHECK(one == b);
			n->generate(reused, e_reused);
			CHECK(reused == b);
			if (same_as_old) {
				CHECK(a == b);
				++st.same_as_old;
			} else if (a != b) {
				++st.differ_from_old;
			}
			if (with_reference) {
				CHECK(b == r->generate(e_ref, library_pick, collapse));
				++st.reference;
			}
		}
	}
}

// Random patterns: well formed from the grammar, or arbitrary bytes.
class Patterns {
public:
	explicit Patterns(unsigned seed) : rng_(seed) {}

	// `alternatives` bounds the alternatives of a group; `symbols` allows symbol
	// letters at symbol level; `bytes` allows non-ASCII bytes and NUL.
	std::string well_formed(int depth, int alternatives, bool symbols, bool bytes) {
		std::string out;
		group(out, depth, false, alternatives, symbols, bytes);
		return out;
	}

	std::string garbage(std::size_t length) {
		static const char alphabet[] = "<>()|!~sSbvVcBCimMDdaxy' ";
		std::string out;
		for (std::size_t i = 0; i < length; ++i) {
			unsigned r = below(20);
			if (r == 0) {
				out += static_cast<char>(below(256));
			} else {
				out += alphabet[below(sizeof(alphabet) - 1)];
			}
		}
		return out;
	}

	unsigned below(unsigned n) { return std::uniform_int_distribution<unsigned>(0, n - 1)(rng_); }

private:
	void group(std::string& out, int depth, bool literal, int alternatives, bool symbols, bool bytes) {
		int alts = static_cast<int>(below(static_cast<unsigned>(alternatives) + 1));  // 0: empty group
		for (int a = 0; a < alts; ++a) {
			if (a) {
				out += '|';
			}
			int parts = static_cast<int>(below(5));
			for (int i = 0; i < parts; ++i) {
				component(out, depth, literal, alternatives, symbols, bytes);
			}
		}
	}

	void component(std::string& out, int depth, bool literal, int alternatives, bool symbols, bool bytes) {
		static const char letters[] = "aehijquxyz'- AZ";
		static const char symbol_letters[] = "sSbvVcBCimMDd";
		static const char* const raw[] = {"\xc3\xa9", "\xc3\x89", "\xce\xb1", "\xff", "\x80"};
		unsigned r = below(12);
		if (r < 2 && depth > 0) {
			bool lit = below(2) != 0;
			out += lit ? '(' : '<';
			group(out, depth - 1, lit, alternatives, symbols, bytes);
			out += lit ? ')' : '>';
		} else if (r < 4) {
			out += below(2) ? '!' : '~';  // a wrapper at symbol level, a character in ()
		} else if (r < 7 && symbols && !literal) {
			out += symbol_letters[below(sizeof(symbol_letters) - 1)];
		} else if (r < 8 && bytes) {
			unsigned i = below(6);
			if (i == 5) {
				out += '\0';
			} else {
				out += raw[i];
			}
		} else {
			char c = letters[below(sizeof(letters) - 1)];
			if (!literal && !symbols && NameGen::Generator::SymbolMap().count(std::string(1, c))) {
				// i is a symbol: without symbols it goes in a literal group, "(i)", which
				// keeps "!(i)" and the Turkish capital in this class.
				out += '(';
				out += c;
				out += ')';
			} else {
				out += c;
			}
		}
	}

	std::mt19937 rng_;
};

}  // namespace check
