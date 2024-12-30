#include "Utils.h"
#include <cstdlib>
#include <ctime>

// Seed the random number generator.
void seedRandom() {
	srand(time(0));	
}

// Randomize a number from 0 to 1
double randDecimal() {
	return (double) rand() / RAND_MAX;	
}

// Randomize a number from 0 to bound
int rand(int bound) {
	return rand() % bound;	
}

// Roll one die.
int dieRoll(int sides) {
	return rand(sides) + 1;	
}

// Roll several dice and sum.
int diceRoll(int num, int sides) {
	int sum = 0;
	for (int i = 0; i < num; i++) {
		sum += dieRoll(sides);
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
