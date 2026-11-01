// The choice among alternatives, checked on the draw itself: namegen.cc is compiled into
// this test (it does not link the library) so that its choose() takes a stub engine.
//
// Oracle: each of n alternatives has probability 1/n, as the reference JavaScript
// (Math.floor(Math.random() * n)) and the C port of the generator have it. Sweeping the
// engine's high output over 2^20 evenly spaced values must give every alternative
// 2^20 / n of them, to within one; and the largest draw, whose u rounds to 1.0, must give
// the last alternative.
#include "../namegen.cc"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #x); std::abort(); } } while (0)

namespace {

// Returns `low`, then `high`, like a 32-bit engine.
struct Stub {
	std::uint32_t low, high;
	int calls = 0;
	std::uint32_t operator()() { return calls++ % 2 == 0 ? low : high; }
};

std::size_t pick(std::uint32_t low, std::uint32_t high, std::size_t n) {
	Stub e{low, high};
	std::size_t i = choose(e, n);
	CHECK(e.calls == 2);  // two outputs per choice, whatever n is
	return i;
}

}  // namespace

int main() {
	const std::size_t ns[] = {1, 2, 3, 4, 5, 6, 7, 10, 22, 36, 47, 115, 360, 1000, 4096, 65537};
	const std::uint32_t steps = 1u << 20;
	for (std::size_t n : ns) {
		std::vector<std::uint32_t> count(n);
		std::size_t last = 0;
		for (std::uint32_t k = 0; k < steps; ++k) {
			std::uint32_t high = k << 12;
			std::size_t i = pick(0x9e3779b9u, high, n);
			CHECK(i < n);
			CHECK(i >= last);  // the choice grows with the draw
			last = i;
			++count[i];
		}
		for (std::size_t i = 0; i < n; ++i) {
			std::uint32_t want = steps / static_cast<std::uint32_t>(n);
			CHECK(count[i] + 1 >= want && count[i] <= want + 1);
		}
		CHECK(pick(0, 0, n) == 0);
		CHECK(pick(0xffffffffu, 0xffffffffu, n) == n - 1);  // u rounds to 1.0
		CHECK(pick(0, 0xffffffffu, n) == n - 1);
	}
	std::puts("all namegen choice tests passed");
	return 0;
}
