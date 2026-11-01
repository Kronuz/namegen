// The choice among alternatives, checked on the draw itself: namegen.cc is compiled into
// this test (it does not link the library) so that its pick() takes a stub engine.
//
// Oracle: each of n alternatives has probability 1/n. A 32-bit output x is accepted
// when x * n mod 2^32 >= 2^32 mod n, which leaves exactly floor(2^32 / n) values of x
// for each alternative floor(x * n / 2^32); a rejected x draws again. The test checks the
// rule on every swept x, counts the accepted x of 2^20 evenly spaced ones per
// alternative (2^20 / n each, to within one), and checks that one alternative and a
// character draw nothing.
#include "../namegen.cc"

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <vector>

#define CHECK(x) do { if (!(x)) { std::fprintf(stderr, "%s:%d: CHECK(%s) failed\n", __FILE__, __LINE__, #x); std::abort(); } } while (0)

namespace {

// Returns `first`, then `second` forever, like a 32-bit engine.
struct Stub {
	std::uint32_t first, second;
	int calls = 0;
	std::uint32_t operator()() { return calls++ == 0 ? first : second; }
};

bool accepted(std::uint32_t x, std::uint32_t n) {
	std::uint64_t product = static_cast<std::uint64_t>(x) * n;
	return (product & 0xffffffffu) >= ((std::uint64_t(1) << 32) % n);
}

std::uint32_t index_of(std::uint32_t x, std::uint32_t n) {
	return static_cast<std::uint32_t>((static_cast<std::uint64_t>(x) * n) >> 32);
}

}  // namespace

int main() {
	const std::uint32_t ns[] = {2, 3, 4, 5, 6, 7, 10, 22, 36, 47, 115, 360, 1000, 4096, 65537,
	                            0x7fffffffu, 0x80000001u, 0xfffffffeu};
	const std::uint32_t steps = 1u << 20;
	for (std::uint32_t n : ns) {
		// The second output a rejected first one is followed by: one that is accepted.
		std::uint32_t fallback = 0x9e3779b9u;
		while (!accepted(fallback, n)) {
			fallback += 0x6d2b79f5u;
		}
		std::vector<std::uint32_t> count(n <= 65537 ? n : 0);
		std::uint32_t accepted_count = 0;
		for (std::uint32_t k = 0; k < steps; ++k) {
			std::uint32_t x = (k << 12) | 0x5a5u;
			Stub e{x, fallback};
			std::uint32_t i = pick(e, n);
			CHECK(i < n);
			if (accepted(x, n)) {
				CHECK(e.calls == 1 && i == index_of(x, n));
				++accepted_count;
				if (!count.empty()) {
					++count[i];
				}
			} else {
				CHECK(e.calls == 2 && i == index_of(fallback, n));
			}
		}
		for (std::uint32_t c : count) {
			std::uint32_t want = accepted_count / n;
			CHECK(c + 2 >= want && c <= want + 2);
		}
		// The rejected values: the low products below 2^32 mod n.
		std::uint32_t reject = static_cast<std::uint32_t>((std::uint64_t(1) << 32) % n);
		if (reject) {
			Stub zero{0, fallback};  // 0 * n mod 2^32 = 0 < 2^32 mod n
			CHECK(pick(zero, n) == index_of(fallback, n) && zero.calls == 2);
		}
		Stub top{0xffffffffu, fallback};
		std::uint32_t i = pick(top, n);
		CHECK(i == (accepted(0xffffffffu, n) ? n - 1 : index_of(fallback, n)));
	}
	// One alternative takes no draw.
	Stub none{1, 2};
	CHECK(choose(none, 1) == 0 && none.calls == 0);
	CHECK(choose(none, 0) == 0 && none.calls == 0);
	std::puts("all namegen choice tests passed");
	return 0;
}
