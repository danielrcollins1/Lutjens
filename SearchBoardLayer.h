/*
	Name: SearchBoardLayer
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 23-11-24 18:17
	Description: Provides one layer of binary information
		for the Search Board (e.g., sea, coast, etc.).
		Indexes are 1-based (to match printed game board).
*/
#ifndef SEARCHBOARDLAYER_H
#define SEARCHBOARDLAYER_H
#include "GridCoordinate.h"
#include <string>

// Search Board Layer class
class SearchBoardLayer
{
	public:
		SearchBoardLayer(const std::string &filename);
		bool isBitOn(GridCoordinate coord) const;
		void print() const;

	private:
		
		// Data
		std::string filename;
		typedef unsigned int uint32;
		static const char MIN_ROW = 'A';
		static const char MAX_ROW = 'Z';
		static const int MIN_COL = 1;
		static const int MAX_COL = 29;
		static const int MIN_PRINT_COL = -2;
		static const int MAX_PRINT_COL = 32;
		static const int NUM_ROWS = MAX_ROW - MIN_ROW + 1;
		uint32 rowData[NUM_ROWS] = {0};
};

#endif