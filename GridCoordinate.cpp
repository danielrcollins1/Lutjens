#include "GridCoordinate.h"
#include "Utils.h"
#include <cassert>

// Off-board marker
const GridCoordinate GridCoordinate::OFFBOARD('~', 0);

// Default constructor
GridCoordinate::GridCoordinate() {
	*this = OFFBOARD;
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
	return *this == OFFBOARD ? "OFFBOARD" : row + std::to_string(col);
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

// Get zones in an area nearby this zone
//   Area has this zone as center and given radius (inclusive)
//   Do search within bounding rhombus on map
std::vector<GridCoordinate> GridCoordinate::getArea(int radius) const {
	assert(radius >= 0);
	std::vector<GridCoordinate> area;
	for (char zRow = row - radius; zRow <= row + radius; zRow++) {
		for (int zCol = col - radius; zCol <= col + radius; zCol++) {
			GridCoordinate zone(zRow, zCol);
			if (distanceFrom(zone) <= radius) {
				area.push_back(zone);
			}
		}
	}
	return area;
}

// Compare two coordinates for equality
bool GridCoordinate::operator==(const GridCoordinate& other) const {
	return row == other.row && col == other.col;
}

// Compare two coordinates for in equality
bool GridCoordinate::operator!=(const GridCoordinate& other) const {
	return !(*this == other);
}

// Compare two coordinates by lexicographic order
bool GridCoordinate::operator<(const GridCoordinate& other) const {
	return row < other.row || (row == other.row && col < other.col);
}

// Hashing function
std::size_t GridCoordinateHash::operator()(const GridCoordinate& coord) const 
{
	return std::hash<int>()(coord.getRow()) 
		^ (std::hash<int>()(coord.getCol()) << 1);
}

// Stream insertion operator
std::ostream& operator<<(std::ostream& stream, const GridCoordinate& coord) {
	stream << coord.toString();
	return stream;
}
