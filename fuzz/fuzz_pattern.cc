// Fuzz target: any bytes as a pattern, through the library, the frozen original and the
// reference interpreter (test/check.h): the same exception or none, the same counts and
// lengths, and the same names from engines seeded alike.
//
// Built with -fsanitize=fuzzer it is a libFuzzer target (NAMEGEN_FUZZ=ON). With
// NAMEGEN_FUZZ_DRIVER it has a main() instead: `namegen_fuzz [iterations] [files...]`
// replays the files, then runs random patterns and mutations of them (the CTest test).
#include "../test/check.h"

#include <clocale>
#include <cstddef>
#include <cstdint>
#include <string>

extern "C" int LLVMFuzzerTestOneInput(const std::uint8_t* data, std::size_t size) {
	static bool locale = std::setlocale(LC_ALL, "C") != nullptr;
	(void)locale;
	if (size > 4096) {
		return 0;
	}
	std::string p(reinterpret_cast<const char*>(data), size);
	check::pattern(p, true, static_cast<unsigned>(size), 1, 3);
	check::pattern(p, false, static_cast<unsigned>(size) + 1, 1, 3);
	return 0;
}

#ifdef NAMEGEN_FUZZ_DRIVER
#include <fstream>
#include <iterator>
#include <vector>

int main(int argc, char** argv) {
	long iterations = argc > 1 ? std::atol(argv[1]) : 10000;
	std::vector<std::string> corpus = {"sV'i", "!<b<||v|V><||s|S|b>>", GREEK_NAMES, DRAGONS_PERN, "(a|b)<c|>!~s"};
	for (int i = 2; i < argc; ++i) {
		std::ifstream in(argv[i], std::ios::binary);
		corpus.emplace_back(std::istreambuf_iterator<char>(in), std::istreambuf_iterator<char>());
	}
	auto run = [](const std::string& p) {
		LLVMFuzzerTestOneInput(reinterpret_cast<const std::uint8_t*>(p.data()), p.size());
	};
	for (auto& p : corpus) {
		run(p);
	}
	check::Patterns gen(2026);
	for (long i = 0; i < iterations; ++i) {
		std::string p;
		switch (gen.below(4)) {
			case 0: p = gen.well_formed(4, 5, true, true); break;
			case 1: p = gen.garbage(gen.below(64)); break;
			default: {
				// A mutation of a corpus entry: bytes replaced, inserted or removed.
				p = corpus[gen.below(static_cast<unsigned>(corpus.size()))];
				std::string noise = gen.garbage(4);
				for (unsigned m = gen.below(4) + 1; m > 0; --m) {
					std::size_t at = p.empty() ? 0 : gen.below(static_cast<unsigned>(p.size()));
					switch (gen.below(3)) {
						case 0: if (!p.empty()) p[at] = noise[m % noise.size()]; break;
						case 1: p.insert(at, 1, noise[m % noise.size()]); break;
						default: if (!p.empty()) p.erase(at, 1); break;
					}
				}
				if (gen.below(8) == 0) {
					corpus.push_back(p);
				}
			}
		}
		run(p);
	}
	auto& st = check::stats();
	std::printf("namegen fuzz: %lu patterns (%lu invalid), %lu names, %lu against the reference\n",
	            st.patterns, st.invalid, st.names, st.reference);
	return 0;
}
#endif
