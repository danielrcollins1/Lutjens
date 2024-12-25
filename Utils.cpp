#include "Utils.h"
#include <cstdlib>
#include <ctime>

// Seed the random number generator.
void seedRandom() {
	srand(time(0));	
}

// Roll one die.
int rollDie(int sides) {
	return rand() % sides + 1;	
}

// Roll multiple dice and sum.
int rollDice(int num, int sides) {
	int sum = 0;
	for (int i = 0; i < num; i++) {
		sum += rollDie(sides);
	}
	return sum;
}

// Get a Y/N response from stdin.
bool getUserYes() {
	std::string input;
	std::cin >> input;
	return tolower(input[0]) == 'y';
}

// Check if a C-string is all digits.
bool isAllDigits(const char* s) {
	while (*s) {
		if (!isdigit(*s)) {
			return false;
		}
		s++;
	}
	return true;
}

// Check if an integer is in a given interval
bool isInInterval(int min, int value, int max) {
	return min <= value && value <= max;	
}
