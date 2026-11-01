// A runnable tour: with no arguments it prints names from a few patterns; with a
// pattern (and a count) it prints that many names from it.
#include "namegen.h"

#include <clocale>
#include <cstdlib>
#include <iostream>
#include <random>


int main(int argc, char **argv)
{
	std::setlocale(LC_CTYPE, "");

	if (argc == 1) {
		const char* patterns[] = {"sV'i", "!s(dim)", "<c|v|>!s", "~<sV'i>", MIDDLE_EARTH, GREEK_NAMES, POKEMON};
		for (auto pattern : patterns) {
			NameGen::Generator generator(pattern);
			std::cout << pattern << " (" << generator.combinations() << " combinations, "
			          << generator.min() << " to " << generator.max() << " bytes):";
			for (int i = 0; i < 5; i++) {
				std::cout << " " << generator.toString();
			}
			std::cout << "\n";
		}
		return 0;
	}

	int num = 1;
	if (argc > 3 || (argc == 3 && (num = std::atoi(argv[2])) < 0)) {
		std::cerr << "Usage: " << argv[0] << " [<pattern> [num]]\n";
		std::cerr << "  pattern - Template for names to generate (a tour without one).\n";
		std::cerr << "  num     - Number of names to generate.\n";
		return 64;
	}

	char * pattern = argv[1];
	NameGen::Generator generator(pattern);
	std::cerr << "> combinations = " << generator.combinations() << "\n";

	for (int i = 0; i < num; i++) {
		std::cout << generator.toString() << "\n";
	}

	return 0;
}
