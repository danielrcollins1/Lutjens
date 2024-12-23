/*
	Name: GridCoordinate
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 23-11-24 23:11
	Description: Grid-coordinates for zones on search board.
		Rows & columns as shown on printed game board.
*/
#ifndef GRIDCOORDINATE_H
#define GRIDCOORDINATE_H
#include <string>
#include <vector>
#include <iostream>

// GridCoordinate class
class GridCoordinate
{
	public:
		GridCoordinate();
		GridCoordinate(const char* s);
		GridCoordinate(const std::string s);
		GridCoordinate(char row, int col);
		char getRow() const { return row; };
		int getCol() const { return col; };
		std::string toString() const;
 		int distanceFrom(const GridCoordinate& dest) const;
		std::vector<GridCoordinate> getAdjacent() const;
		bool operator==(const GridCoordinate& coord) const;
		bool operator!=(const GridCoordinate& coord) const;
		static bool isValid(const char *s);
		static bool isValid(const std::string s);
		static const GridCoordinate NO_ZONE;
		
	private:
		typedef char int8;
		int8 row, col;
};

// Stream insertion operator
std::ostream& operator<<(std::ostream& stream, const GridCoordinate& coord);

#endif