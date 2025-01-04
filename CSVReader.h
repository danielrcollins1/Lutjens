/*
	Name: CSVReader.h
	Copyright: 2022
	Author: Daniel R. Collins
	Date: 27-11-22 16:15
	Description: Interface to simple CSV reader class.
*/
#ifndef CSVREADER_H
#define CSVREADER_H

#include <string>
#include <vector>
using std::string;
using std::vector;

class CSVReader {
	public:
		static vector<vector<string>> readFile(const string &filename);

	private:
		static vector<string> splitLine (const string &line);
};

#endif
