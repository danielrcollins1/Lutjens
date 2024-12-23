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
using namespace std;

// Test SearchBoard
void testSearchBoard() {
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

// Test GridCoordinate adjacency list
void testCoordinateAdjacency(const GridCoordinate& coord) {
	cout << "Adjacent to " << coord << ": ";
	printVec(coord.getAdjacent());
}

// Get one step towards destination
GridCoordinate getOneStep(const GridCoordinate& src, 
	const GridCoordinate& dest) 
{
	auto board = SearchBoard::instance();
	int dist = src.distanceFrom(dest);
	vector<GridCoordinate> adjacent = src.getAdjacent();
	vector<GridCoordinate> options;
	for (auto gc: adjacent) {
		if (board->isSeaZone(gc) 
			&& !board->isBritishCoast(gc)
			&& gc.distanceFrom(dest) < dist)
		{
			options.push_back(gc);
		}
	}
	return randomElem(options);
}

// Test simple pathfinding
void testPathFinding(const GridCoordinate& src, 
	const GridCoordinate& dest) 
{
	cout << "Path from " << src << " to " << dest << ": ";
	auto loc = src;
	while (loc != dest) {
		loc = getOneStep(loc, dest);
		cout << loc.toString() << " ";
	}
	cout << endl;
}

// Test pathfinding to randomized destination
void testPathRandDest(const GridCoordinate& src, 
	const GridCoordinate& destCenter) 
{
	auto board = SearchBoard::instance();
	GridCoordinate dest = board->randSeaWithinOne(destCenter);
	testPathFinding(src, dest);
}

// Test ship construction
void testShipConstruction() {
	Ship ship("Bismarck", Ship::Type::BB, 29, 10, 13);
	ship.setPosition("F20");
	cout << "Ship test: " << ship << endl;
}

// Main test driver
int main(int argc, char** argv) {
	srand(time(0));
	testSearchBoard();
	testObjectSizes();
	testCoordinateConstructors();
	testCoordinateDistances();
	testCoordinateAdjacency("J16");
	testPathFinding("F20", "B11");
	testPathRandDest("F20", "B11");
	testShipConstruction();
	return 0;
}
