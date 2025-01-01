/*
	Name: Test Components
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 24-11-24 00:09
	Description: Test basic objects in the Lutjens program
*/
#include <iostream>
#include <cassert>
#include <ctime>
#include "SearchBoard.h"
#include "GridCoordinate.h"
#include "Ship.h"
#include "Utils.h"
#include "Navigator.h"
using namespace std;

// Test SearchBoard
void testSearchBoard() {
	cout << "\nPress [Enter] to view search board.\n";
	cin.get();
	SearchBoard::instance()->print();
	cout << "Done search board test.\n";
}

// Test object sizes
void testObjectSizes() {
	assert(sizeof(GridCoordinate) == 2);
	assert(sizeof(SearchBoardLayer) <= 128);
	cout << "Done object size tests.\n";
}

// Test GridCoordinate construction
void testCoordinateConstructors() {
	assert(GridCoordinate() == GridCoordinate::NO_ZONE);
	assert(GridCoordinate('B', 3).toString() == "B3");	
	assert(GridCoordinate("P18").toString() == "P18");
	assert(GridCoordinate(string("Z20")).toString() == "Z20");
	cout << "Done coordinate constructor tests.\n";
}

// Test GridCoordinate distance calculator
void testCoordinateDistances() {
	GridCoordinate anchor("K10");
	assert(anchor.distanceFrom("F5") == 5);
	assert(anchor.distanceFrom("F10") == 5);
	assert(anchor.distanceFrom("H2") == 8);
	assert(anchor.distanceFrom("F5") == 5);
	assert(anchor.distanceFrom("K5") == 5);
	assert(anchor.distanceFrom("K12") == 2);
	assert(anchor.distanceFrom("P19") == 9);
	assert(anchor.distanceFrom("R14") == 7);
	assert(anchor.distanceFrom("K10") == 0);
	cout << "Done coordinate distance tests.\n";
}

// Test GridCoordinate local area list
void testCoordinateArea(const GridCoordinate& coord) {
	cout << "Nearby to " << coord << ": ";
	printVec(coord.getArea(1));
}

// Test ship construction
void testShipConstruction() {
	Ship ship("Bismarck", Ship::Type::BB, 29, 10, 13, "F20");
	cout << "Ship test: " << ship << endl;
}

// Test A* pathfinding navigation
void testNavigatorPath(const GridCoordinate& src, 
	const GridCoordinate& dest) 
{
	cout << "Navigator path from " << src << " to " << dest << ": ";
	Ship ship("Prinz Eugen", Ship::Type::CA, 32, 4, 10, src);
	vector<GridCoordinate> path = Navigator::findSeaRoute(ship, dest);
	printVec(path);
}

// Test the British patrol line limit finder
void testPatrolLineLimits() {
	cout << "British patrol line limits: ";
	for (char row = 'E'; row <= 'Z'; row++) {
		int col = SearchBoard::instance()->getPatrolLimitForRow(row);
		cout << GridCoordinate(row, col) << " ";
	}
	cout << endl;	
}

// Main test driver
int main(int argc, char** argv) {
	srand(time(0));

	// Test basic stuff
	testObjectSizes();
	testCoordinateConstructors();
	testCoordinateDistances();
	testCoordinateArea("J16");
	testShipConstruction();
	testNavigatorPath("F20", "B7");
	testNavigatorPath("F20", "P23");
	testPatrolLineLimits();
	testSearchBoard();
	return 0;
}
