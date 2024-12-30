#include "GridCoordinate.h"
#include "Utils.h"
#include <cassert>

// Off-map marker
const GridCoordinate GridCoordinate::NO_ZONE('~', 0);

// Top-left (hidden) space
const GridCoordinate GridCoordinate::ORIGIN('A', -2);

// Default constructor
GridCoordinate::GridCoordinate() {
	*this = NO_ZONE;
}

// Construct from character & integer
GridCoordinate::GridCoordinate(char row, int col) {
	this->row = row;
	this->col = col;
}

// Construct from C-string
GridCoordinate::GridCoordinate(const char* s) {
	assert(strlen(s) > 1);
	row = toupper(s[0]);
	col = atoi(s + 1);
}

// Construct from string
GridCoordinate::GridCoordinate(const std::string s): 
	GridCoordinate(s.c_str()) {
}

// Is this a valid C-string for a coordinate?
bool GridCoordinate::isValid(const char *s) {
	return strlen(s) > 1
		&& isalpha(s[0])
		&& isAllDigits(s + 1);
}

// Is this a valid string object for a coordinate?
bool GridCoordinate::isValid(const std::string s) {
	return isValid(s.c_str());	
}

// Get coordinate as string
std::string GridCoordinate::toString() const {
	std::string s = row + std::to_string(col);
	return s;
}

// Distance from another zone
int GridCoordinate::distanceFrom(const GridCoordinate& dest) const {
	int spanLeft, spanRight;
	int rowDiff = abs(row - dest.row);

	// Find span reached freely on destination row
	if (row < dest.row) {
		spanLeft = col;
		spanRight = col + rowDiff;
	}
	else {
		spanLeft = col - rowDiff;
		spanRight = col;
	}

	// Determine added distance along that row
	if (dest.col < spanLeft)
		return spanLeft - dest.col + rowDiff;
	else if (dest.col > spanRight)
		return dest.col - spanRight + rowDiff;
	else
		return rowDiff;	
}

// Get a list of adjacent zones
std::vector<GridCoordinate> GridCoordinate::getAdjacent() const {
	std::vector<GridCoordinate> list(6);
	list[0] = GridCoordinate(row - 1, col);
	list[1] = GridCoordinate(row + 1, col);
	list[2] = GridCoordinate(row, col - 1);
	list[3] = GridCoordinate(row, col + 1);
	list[4] = GridCoordinate(row - 1, col - 1);
	list[5] = GridCoordinate(row + 1, col + 1);
	return list;	
}

// Compare two coordinates for equality
bool GridCoordinate::operator==(const GridCoordinate& coord) const {
	return row == coord.row && col == coord.col;	
}

// Compare two coordinates for in equality
bool GridCoordinate::operator!=(const GridCoordinate& coord) const {
	return !(*this == coord);	
}

// Compare two coordinates for distance from origin
bool GridCoordinate::operator<(const GridCoordinate& coord) const {
	return ORIGIN.distanceFrom(*this) < ORIGIN.distanceFrom(coord);
}

// Stream insertion operator
std::ostream& operator<<(std::ostream& stream, const GridCoordinate& coord) {
	stream << coord.toString();
	return stream;
}

// Hash function
std::size_t GridCoordinate::Hash::operator()(const GridCoordinate& coord) const 
{
	return std::hash<int>()(coord.row) ^ (std::hash<int>()(coord.col) << 1);
}
