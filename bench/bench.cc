// Benchmark: compiling a pattern, and drawing names from it, for namegen, the frozen
// original (test/frozen) and the C port (skeeto/fantasyname c/namegen.h, fetched by the
// bench build), with the heap allocations of each operation.
//
//   namegen_bench           the table: ns and allocations per operation, median of rounds
//   namegen_bench --check   checks that each arm's names are names the pattern can produce
//
// Arms, interleaved within each round:
//   compile     NameGen::Generator(pattern), compiled and destroyed
//   toString    one name from a compiled Generator, from the thread's engine
//   generate    one name from a compiled Generator into a reused std::string,
//               generate(name, engine): what a C port call into a buffer does
//   one-shot    NameGen::generate(name, pattern, length, engine) into a reused
//               std::string, no Generator: what one C port call does
//   original *  compile and toString on the frozen original
//   C port      namegen(buf, 256, pattern, &seed): parses the pattern and draws in one pass
//   generate'   generate again, the A/A arm: its spread from generate is the noise
#include "../test/check.h"

#include <algorithm>
#include <chrono>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <new>
#include <set>
#include <string>
#include <vector>

extern "C" int cport_namegen(char* dst, unsigned long len, const char* pattern, unsigned long* seed);

static unsigned long allocations = 0;

void* operator new(std::size_t n) {
	++allocations;
	if (void* p = std::malloc(n ? n : 1)) {
		return p;
	}
	throw std::bad_alloc();
}
void operator delete(void* p) noexcept { std::free(p); }
void operator delete(void* p, std::size_t) noexcept { std::free(p); }

static volatile std::size_t sink;

struct Arm {
	const char* name;
	std::function<std::size_t()> op;  // one operation; returns a byte count for the sink
	std::vector<double> ns;
	double allocs = 0;
};

static double now_ns() {
	return std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

static double median(std::vector<double> v) {
	std::sort(v.begin(), v.end());
	return v[v.size() / 2];
}

// Whether `name` is a name `n` can produce (no collapsing), by matching the pattern's
// tree; capitalized letters match either case.
static void match(const ref::Node& n, const std::string& name, std::set<std::size_t> from, std::set<std::size_t>& to) {
	to.clear();
	auto eq = [](char a, char b) { return a == b || (a >= 'a' && a <= 'z' && a - 'a' + 'A' == b); };
	for (std::size_t p : from) {
		switch (n.kind) {
			case ref::Node::Char:
				if (p < name.size() && eq(n.c, name[p])) to.insert(p + 1);
				break;
			case ref::Node::Empty:
				to.insert(p);
				break;
			case ref::Node::Symbol:
				for (auto& s : *n.table) {
					if (name.size() - p >= s.size() && std::equal(s.begin(), s.end(), name.begin() + p, eq)) to.insert(p + s.size());
				}
				break;
			case ref::Node::Sequence: {
				std::set<std::size_t> cur{p}, next;
				for (auto& k : n.kids) {
					match(k, name, cur, next);
					cur.swap(next);
				}
				to.insert(cur.begin(), cur.end());
				break;
			}
			case ref::Node::Choice:
				for (auto& k : n.kids) {
					std::set<std::size_t> out;
					match(k, name, {p}, out);
					to.insert(out.begin(), out.end());
				}
				break;
			default: {
				std::set<std::size_t> out;
				match(n.kids[0], name, {p}, out);
				to.insert(out.begin(), out.end());
			}
		}
	}
}

static bool producible(const ref::Pattern& r, const std::string& name) {
	std::set<std::size_t> out;
	match(r.root, name, {0}, out);
	return out.count(name.size()) != 0;
}

int main(int argc, char** argv) {
	bool check_only = argc > 1 && std::strcmp(argv[1], "--check") == 0;
	double target_ms = argc > 2 ? std::atof(argv[2]) : 20.0;
	const int rounds = 9;

	struct Row { const char* label; const char* pattern; bool cport; };
	const Row rows[] = {
		{"sV'i", "sV'i", true},
		{"Xapiand", "!<b<||v|V><||s|S|b>>", false},
		{"MIDDLE_EARTH", MIDDLE_EARTH, true},
		{"GREEK_NAMES", GREEK_NAMES, true},
		{"JAPANESE_DIVERSE", JAPANESE_NAMES_DIVERSE, true},
	};

	std::setlocale(LC_ALL, "C");
	unsigned long seed = 0x9e3779b9ul;
	char buf[256];

	if (check_only) {
		for (auto& row : rows) {
			NameGen::Generator g(row.pattern, false);
			NameGenOld::Generator o(row.pattern, false);
			ref::Pattern r(row.pattern);
			std::mt19937 engine(1);
			std::string name;
			for (int i = 0; i < 2000; ++i) {
				CHECK(producible(r, g.toString()));
				CHECK(producible(r, o.toString()));
				NameGen::generate(name, row.pattern, std::strlen(row.pattern), engine, false);
				CHECK(producible(r, name));
				if (row.cport) {
					CHECK(cport_namegen(buf, sizeof(buf), row.pattern, &seed) == 0);
					CHECK(producible(r, buf));
				}
			}
		}
		std::puts("namegen bench check: every arm's names are names of the pattern");
		return 0;
	}

	std::printf("%-17s %-20s %10s %10s\n", "pattern", "arm", "ns/op", "allocs/op");
	for (auto& row : rows) {
		const std::string p = row.pattern;
		NameGen::Generator g(p);
		NameGenOld::Generator o(p);
		std::mt19937 engine(0x9e3779b9u);
		std::string name;
		const size_t length = p.size();
		std::vector<Arm> arms = {
			{"compile", [&] { NameGen::Generator x(p); return x.max(); }, {}},
			{"toString", [&] { return g.toString().size(); }, {}},
			{"generate", [&] { g.generate(name, engine); return name.size(); }, {}},
			{"one-shot", [&] { NameGen::generate(name, row.pattern, length, engine); return name.size(); }, {}},
			{"original compile", [&] { NameGenOld::Generator x(p); return x.max(); }, {}},
			{"original toString", [&] { return o.toString().size(); }, {}},
			{"generate'", [&] { g.generate(name, engine); return name.size(); }, {}},
		};
		if (row.cport) {
			arms.insert(arms.begin() + 6, Arm{"C port", [&] {
				cport_namegen(buf, sizeof(buf), row.pattern, &seed);
				return std::strlen(buf);
			}, {}});
		}
		// Calibrate each arm to about target_ms per round, then interleave the rounds.
		std::vector<long> iterations;
		for (auto& a : arms) {
			long n = 1;
			for (;;) {
				double t0 = now_ns();
				for (long i = 0; i < n; ++i) sink = a.op();
				double dt = now_ns() - t0;
				if (dt > target_ms * 1e6 / 4) {
					iterations.push_back(std::max(1L, static_cast<long>(n * (target_ms * 1e6 / dt))));
					break;
				}
				n *= 2;
			}
			unsigned long before = allocations;
			for (int i = 0; i < 1000; ++i) sink = a.op();
			a.allocs = (allocations - before) / 1000.0;
		}
		for (int r = 0; r < rounds; ++r) {
			for (std::size_t k = 0; k < arms.size(); ++k) {
				double t0 = now_ns();
				for (long i = 0; i < iterations[k]; ++i) sink = arms[k].op();
				arms[k].ns.push_back((now_ns() - t0) / static_cast<double>(iterations[k]));
			}
		}
		for (auto& a : arms) {
			std::printf("%-17s %-20s %10.1f %10.2f\n", row.label, a.name, median(a.ns), a.allocs);
		}
	}
	return 0;
}
