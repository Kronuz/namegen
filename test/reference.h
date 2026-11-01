// The test oracle: a reference interpreter of the pattern syntax, written from the
// description in namegen.h and independent of the library's generator tree. It covers
// patterns of ASCII bytes 1 to 127 (the case where capitalizing, reversing and
// collapsing are byte operations in every locale).
//
// It draws from the engine where the library does: once for every literal character,
// every symbol and every group with two or more alternatives, and never for an empty
// group. `Pick` maps a draw to an alternative: `legacy` as the original C++ port did
// (round a draw over [0, n - 1] to the nearest integer, which gives the first and the
// last alternative half the weight of the others), `uniform` as the reference
// JavaScript (`Math.floor(Math.random() * n)`) and the C port do.
#pragma once

#include "namegen.h"

#include <algorithm>
#include <cstddef>
#include <limits>
#include <random>
#include <stdexcept>
#include <string>
#include <vector>

namespace ref {

enum class Pick { legacy, uniform };

inline std::size_t pick(std::mt19937& engine, std::size_t n, Pick mode) {
	if (mode == Pick::legacy) {
		std::uniform_real_distribution<double> distribution(0, n - 1);
		return static_cast<std::size_t>(static_cast<int>(distribution(engine) + 0.5));
	}
	double u = std::uniform_real_distribution<double>(0.0, 1.0)(engine);
	std::size_t i = static_cast<std::size_t>(u * static_cast<double>(n));
	return i < n ? i : n - 1;
}

struct Node {
	enum Kind { Char, Empty, Symbol, Sequence, Choice, Capitalize, Reverse } kind;
	explicit Node(Kind k) : kind(k) {}
	char c = 0;
	const std::vector<std::string>* table = nullptr;
	std::vector<Node> kids;
};

class Parser {
public:
	explicit Parser(const std::string& p) : p_(p) {}

	Node parse() {
		Node top = group(false);
		if (pos_ != p_.size()) {
			throw std::logic_error("parser stopped early");
		}
		return top;
	}

private:
	// One group's alternatives, then its node. `literal` is a () group. The group ends
	// at its closing bracket, or at the end of the pattern for the top level.
	Node group(bool literal) {
		std::vector<std::vector<Node>> alternatives;
		std::vector<int> wrappers;  // pushed by ! (0) and ~ (1); the last is the innermost
		auto add = [&](Node n) {
			for (auto w = wrappers.rbegin(); w != wrappers.rend(); ++w) {
				Node wrapped{*w == 0 ? Node::Capitalize : Node::Reverse};
				wrapped.kids.push_back(std::move(n));
				n = std::move(wrapped);
			}
			wrappers.clear();
			if (alternatives.empty()) {
				alternatives.emplace_back();
			}
			alternatives.back().push_back(std::move(n));
		};
		const bool top = depth_ == 0;
		while (pos_ < p_.size()) {
			char c = p_[pos_++];
			switch (c) {
				case '<':
				case '(':
					++depth_;
					add(group(c == '('));
					--depth_;
					break;
				case '>':
				case ')':
					if (top) {
						throw std::invalid_argument("Unbalanced brackets");
					}
					if (c == '>' && literal) {
						throw std::invalid_argument("Unexpected '>' in pattern");
					}
					if (c == ')' && !literal) {
						throw std::invalid_argument("Unexpected ')' in pattern");
					}
					return produce(alternatives);
				case '|':
					if (alternatives.empty()) {
						alternatives.emplace_back();
					}
					alternatives.emplace_back();
					break;
				case '!':
				case '~':
					if (!literal) {
						wrappers.push_back(c == '!' ? 0 : 1);
						break;
					}
					add(character(c));
					break;
				default:
					if (!literal) {
						auto& symbols = NameGen::Generator::SymbolMap();
						auto it = symbols.find(std::string(1, c));
						if (it != symbols.end()) {
							Node n{Node::Symbol};
							n.table = &it->second;
							add(std::move(n));
							break;
						}
					}
					add(character(c));
					break;
			}
		}
		if (!top) {
			missing_ = true;
		}
		return produce(alternatives);
	}

	static Node character(char c) {
		Node n{Node::Char};
		n.c = c;
		return n;
	}

	static Node produce(std::vector<std::vector<Node>>& alternatives) {
		if (alternatives.empty()) {
			return Node{Node::Empty};
		}
		std::vector<Node> sequences;
		for (auto& a : alternatives) {
			Node s{Node::Sequence};
			s.kids = std::move(a);
			sequences.push_back(std::move(s));
		}
		if (sequences.size() == 1) {
			return std::move(sequences[0]);
		}
		Node choice{Node::Choice};
		choice.kids = std::move(sequences);
		return choice;
	}

	const std::string& p_;
	std::size_t pos_ = 0;
	int depth_ = 0;

public:
	bool missing_ = false;
};

inline void eval(const Node& n, std::string& out, std::mt19937& engine, Pick mode) {
	switch (n.kind) {
		case Node::Char:
			pick(engine, 1, mode);
			out += n.c;
			break;
		case Node::Empty:
			break;
		case Node::Symbol:
			out += (*n.table)[pick(engine, n.table->size(), mode)];
			break;
		case Node::Sequence:
			for (auto& k : n.kids) {
				eval(k, out, engine, mode);
			}
			break;
		case Node::Choice:
			eval(n.kids[pick(engine, n.kids.size(), mode)], out, engine, mode);
			break;
		case Node::Capitalize: {
			std::size_t start = out.size();
			eval(n.kids[0], out, engine, mode);
			if (start < out.size() && out[start] >= 'a' && out[start] <= 'z') {
				out[start] = static_cast<char>(out[start] - 'a' + 'A');
			}
			break;
		}
		case Node::Reverse: {
			std::size_t start = out.size();
			eval(n.kids[0], out, engine, mode);
			std::reverse(out.begin() + static_cast<std::ptrdiff_t>(start), out.end());
			break;
		}
	}
}

// Runs of three: one of a, h, i, j, q, u, v, w, x, y is kept, two of anything else.
inline std::string collapse(const std::string& s) {
	std::string out;
	int run = 0;
	char prev = 0;
	for (char c : s) {
		run = c == prev ? run + 1 : 0;
		int keep = std::string("ahijquvwxy").find(c) != std::string::npos ? 1 : 2;
		if (run < keep) {
			out += c;
		}
		prev = c;
	}
	return out;
}

// Counts with exact arithmetic, saturated at `cap` (2^64 - 1 as size_t, the library's
// result when the count does not fit).
using wide = unsigned __int128;
inline wide sat(wide v) {
	const wide cap = std::numeric_limits<std::size_t>::max();
	return v > cap ? cap : v;
}

inline wide combinations(const Node& n) {
	switch (n.kind) {
		case Node::Char:
		case Node::Empty:
			return 1;
		case Node::Symbol:
			return n.table->size();
		case Node::Choice: {
			wide total = 0;
			for (auto& k : n.kids) {
				total = sat(total + combinations(k));
			}
			return total ? total : 1;
		}
		default: {
			wide total = 1;
			for (auto& k : n.kids) {
				total = sat(total * combinations(k));
			}
			return total;
		}
	}
}

inline std::size_t min_length(const Node& n) {
	switch (n.kind) {
		case Node::Char:
			return 1;
		case Node::Empty:
			return 0;
		case Node::Symbol: {
			std::size_t m = std::numeric_limits<std::size_t>::max();
			for (auto& s : *n.table) {
				m = s.size() < m ? s.size() : m;
			}
			return m;
		}
		case Node::Choice: {
			std::size_t m = std::numeric_limits<std::size_t>::max();
			for (auto& k : n.kids) {
				std::size_t v = min_length(k);
				m = v < m ? v : m;
			}
			return m;
		}
		default: {
			std::size_t total = 0;
			for (auto& k : n.kids) {
				total += min_length(k);
			}
			return total;
		}
	}
}

inline std::size_t max_length(const Node& n) {
	switch (n.kind) {
		case Node::Char:
			return 1;
		case Node::Empty:
			return 0;
		case Node::Symbol: {
			std::size_t m = 0;
			for (auto& s : *n.table) {
				m = s.size() > m ? s.size() : m;
			}
			return m;
		}
		case Node::Choice: {
			std::size_t m = 0;
			for (auto& k : n.kids) {
				std::size_t v = max_length(k);
				m = v > m ? v : m;
			}
			return m;
		}
		default: {
			std::size_t total = 0;
			for (auto& k : n.kids) {
				total += max_length(k);
			}
			return total;
		}
	}
}

// A compiled pattern: throws std::invalid_argument with the library's messages.
struct Pattern {
	Node root{Node::Empty};

	explicit Pattern(const std::string& p) {
		Parser parser(p);
		root = parser.parse();
		if (parser.missing_) {
			throw std::invalid_argument("Missing closing bracket");
		}
	}

	std::string generate(std::mt19937& engine, Pick mode, bool collapse_triples) const {
		std::string out;
		eval(root, out, engine, mode);
		return collapse_triples ? collapse(out) : out;
	}
};

}  // namespace ref
