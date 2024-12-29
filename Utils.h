/*
	Name: Utils
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 24-11-24 23:34
	Description: Utility functions.
*/
#ifndef UTILS_H
#define UTILS_H
#include <vector>
#include <cassert>
#include <iostream>
#include <algorithm>

// Seed the random number generator.
void seedRandom();

// Dice-rolling prototypes.
int rand(int bound);
int dieRoll(int sides);
int diceRoll(int num, int sides);

// Get a Y/N response from stdin.
bool getUserYes();

// Check if a C-string is all digits.
bool isAllDigits(const char* s);

// Check if an integer is in a given interval
bool isInInterval(int min, int value, int max);

// Push an array onto a vector
template <class T>
void pushArray(std::vector<T>& vec, const T array[], int size) {
	for (int i = 0; i < size; i++) {
		vec.push_back(array[i]);
	}
}

// Check if a vector has a given element
template <class T>
bool hasElem(const std::vector<T>& vec, const T& value) {
	return find(vec.begin(), vec.end(), value) != vec.end();
}

// Get random element from a vector
template<class T>
T randomElem(const std::vector<T>& vec) {
	assert(vec.size() > 0);
	return vec[rand() % vec.size()];
}

// Print a vector
template<class T>
void printVec(std::vector<T> vec) {
	for (auto val: vec) {
		std::cout << val << " ";
	}
	std::cout << std::endl;
}

#endif