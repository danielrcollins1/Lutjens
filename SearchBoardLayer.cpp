#include "SearchBoardLayer.h"
#include "CSVReader.h"
#include "Utils.h"
#include <iostream>
#include <cassert>
using namespace std;

// Constructor from CSV file data
//    Expects data file stored as transpose of printed board (flip here)
SearchBoardLayer::SearchBoardLayer(const std::string &filename) {
	assert(sizeof(rowData[0]) == 4);
	assert(MAX_COL <= sizeof(rowData[0]) * 8);
	this->filename = filename;
	vector<vector<string>> fileData = CSVReader::readFile(filename);
	assert(fileData.size() == MAX_COL);
	for (int col = MIN_COL; col <= MAX_COL; col++) {
		int colIdx = col - MIN_COL;
		assert(fileData[colIdx].size() == NUM_ROWS);
		for (char row = MIN_ROW; row <= MAX_ROW; row++) {
			int rowIdx = row - MIN_ROW;
			if (fileData[colIdx][rowIdx] == "1") {
				rowData[rowIdx] |= (1 << colIdx);
			}
		}
	}
}

// Check if a given bit is on
//   Convert from board coordinate values to data indeces
//   Return false for any locations outside data store ranges
bool SearchBoardLayer::isBitOn(GridCoordinate coord) const {
	char row = coord.getRow();
	int col = coord.getCol();	
	if (MIN_ROW <= row && row <= MAX_ROW
		&& MIN_COL <= col && col <= MAX_COL)
	{
		int rowIdx = row - MIN_ROW;
		int colIdx = col - MIN_COL;
		return rowData[rowIdx] & (1 << colIdx);
	}
	return false;
}

// Print the layer (for testing)
//   Note the printed board has hidden negative columns in top-left;
//   so we handle that with distinct column start print value.
void SearchBoardLayer::print() const {
	cout << filename << endl;
	for (char row = MIN_ROW; row <= MAX_ROW; row++) {
		
		// Account for column shearing
		bool fullSpaceEnds = row % 2;
		int rowIdx = row - MIN_ROW;
		int colStart = MIN_PRINT_COL + rowIdx / 2;
		int colEnd = 18 + (rowIdx + 1) / 2;
		
		// Print this row
		cout << row << ": ";
		if (fullSpaceEnds) cout << '|';
		for (int col = colStart; col <= colEnd; col++) {
			GridCoordinate zone(row, col);
			cout << (isBitOn(zone) ? '@' : '-');
			if (col < colEnd || fullSpaceEnds) {
				cout << '|';
			}
		}
		cout << endl;
	}
	cout << endl;
}

// Get a list of all the zones with bits on
vector<GridCoordinate> SearchBoardLayer::getAllOn() const {
	vector<GridCoordinate> zoneList;
	for (char row = MIN_ROW; row <= MAX_ROW; row++) {
		for (int col = MIN_COL; col <= MAX_COL; col++) {
			GridCoordinate zone(row, col);
			if (isBitOn(zone)) {
				zoneList.push_back(zone);
			}
		}
	}
	return zoneList;	
}
