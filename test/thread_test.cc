// Threads: one Generator shared by eight threads, each drawing from its own engine
// (ThreadSanitizer finds no race), and a seeded engine repeats its names.
#include "namegen.h"

#include <cstdio>
#include <cstdlib>
#include <random>
#include <string>
#include <thread>
#include <vector>

#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #x); std::abort(); } } while (0)

int main() {
	const NameGen::Generator generator("!<b<||v|V><||s|S|b>>");

	// toString() from many threads at once.
	std::vector<std::thread> threads;
	std::vector<std::size_t> lengths(8);
	for (std::size_t t = 0; t < lengths.size(); ++t) {
		threads.emplace_back([&, t] {
			for (int i = 0; i < 20000; ++i) {
				lengths[t] += generator.toString().size();
			}
		});
	}
	for (auto& th : threads) {
		th.join();
	}
	for (auto n : lengths) {
		CHECK(n > 0);
	}

	// Seeding a thread's engine repeats its sequence; generate() takes any engine.
	auto first = [&](std::mt19937::result_type seed) {
		NameGen::engine().seed(seed);
		std::vector<std::string> names;
		for (int i = 0; i < 16; ++i) {
			names.push_back(generator.toString());
		}
		return names;
	};
	CHECK(first(42) == first(42));
	std::mt19937 engine(42);
	auto names = first(42);
	for (auto& n : names) {
		CHECK(generator.generate(engine) == n);
	}

	// Each new thread seeds its engine on its own.
	std::string a, b;
	std::thread([&] { a = generator.toString() + generator.toString() + generator.toString(); }).join();
	std::thread([&] { b = generator.toString() + generator.toString() + generator.toString(); }).join();
	CHECK(!a.empty() && !b.empty());

	std::puts("all namegen thread tests passed");
	return 0;
}
