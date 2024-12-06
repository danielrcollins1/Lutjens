/*
	Name: CSVReader.cpp
	Copyright: 2022
	Author: Daniel R. Collins
	Date: 27-11-22 16:24
	Description: Implementation of simple CSVReader class.
*/
#include "CSVReader.h"
#include <fstream>
#include <sstream>
#include <iostream>

// Global constants
const char COMMA = ',';
const char QUOTE = '\"';

// Read one CSV file to vector of vectors of strings
vector<vector<string>> CSVReader::readFile(const string &filename) {
	std::ifstream inFile(filename);
	vector<vector<string>> retVal;
	if (!inFile) {
		std::cerr << "Error: Could not open given file.\n";
	} else {
		string line;
		while (getline(inFile, line)) {
			vector<string> separatedLine = splitLine(line);
			retVal.push_back(separatedLine);
		}
		inFile.close();
	}
	return retVal;
}

// Split a line with commas and quotes
vector<string> CSVReader::splitLine (const string &line) {
	unsigned int pos = 0;
	vector<string> strVec;
	while (pos < line.length()) {
		string tok = "";

		// Field is quoted
		if (line.at(pos) == QUOTE) {
			pos++;
			while (pos < line.length() && line.at(pos) != QUOTE) {
				tok += line.at(pos++);
			}
			while (pos < line.length() && line.at(pos) != COMMA) {
				pos++; // eat to next comma
			}
			strVec.push_back(tok);
			pos++;
		}

		// Field is non-quoted
		else {
			while (pos < line.length() && line.at(pos) != COMMA) {
				tok += line.at(pos++);
			}
			strVec.push_back(tok);
			pos++;
		}
	}
	return strVec;
}
