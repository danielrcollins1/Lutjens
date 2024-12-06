#include "SearchBoard.h"
#include "Utils.h"
#include <cassert>

// Singleton instance
SearchBoard* SearchBoard::theInstance = nullptr;

// Singleton instance accessor
SearchBoard* SearchBoard::instance() {
	if (!theInstance) {
		theInstance = new SearchBoard;
	}
	return theInstance;
}

// Constructor
SearchBoard::SearchBoard() {
}

// Print all layers (for testing)
void SearchBoard::print() const {
	for (auto layer: layers) {
		layer.print();	
	}
}

// Is this zone at sea?
bool SearchBoard::isSeaZone(const GridCoordinate& zone) const {
	return layers[SeaZones].isBitOn(zone);
}

// Is this zone fog-prone?
bool SearchBoard::isFogZone(const GridCoordinate& zone) const {
	return layers[FogZones].isBitOn(zone);
}

// Is this zone on a British coast?
bool SearchBoard::isBritishCoast(const GridCoordinate& zone) const {
	return layers[BritishCoast].isBitOn(zone);
}

// Is this zone on the British Patrol line?
bool SearchBoard::isBritishPatrol(const GridCoordinate& zone) const {
	return layers[BritishPatrol].isBitOn(zone);
}

// Is this zone at a British port?
bool SearchBoard::isBritishPort(const GridCoordinate& zone) const {
	return layers[BritishPorts].isBitOn(zone);
}

// Is this zone at a German port?
bool SearchBoard::isGermanPort(const GridCoordinate& zone) const {
	return layers[GermanPorts].isBitOn(zone);
}

// Is this zone on a convoy route?
bool SearchBoard::isConvoyRoute(const GridCoordinate& zone) const {
	return layers[ConvoyRoutes].isBitOn(zone);
}

// Get a random sea zone within one of this
GridCoordinate SearchBoard::randSeaWithinOne
	(const GridCoordinate& zone) const 
{
	// Get all within one
	std::vector<GridCoordinate> radius1(7);
	radius1.push_back(zone);
	auto adj = zone.getAdjacent();
	radius1.insert(radius1.end(), adj.begin(), adj.end());
	
	// Filter by sea zones
	std::vector<GridCoordinate> seaZones;
	for (auto zone: radius1) {
		if (SearchBoard::isSeaZone(zone)) {
			seaZones.push_back(zone);
		}
	}
	
	// Return a random one
	return randomElem(seaZones);
}

// Find the column of the British patrol line
//   (limit of general search) for a given row
int SearchBoard::getPatrolLimitCol(char row) const {
	assert('E' <= row && row <= 'Z');  // Rule 10.211
	int col = 1;
	while (!isBritishPatrol(GridCoordinate(row, col++)));
	return col;
}

// Is this zone within the given distance from some type of zone?
//   Pass one of the isXZone() functions as last argument
bool SearchBoard::isNearZoneType(const GridCoordinate& zone, int distance, 
		bool (SearchBoard::*zoneType)(const GridCoordinate& zone) const) const 
{
	// Check rhombus given distance around the zone
	assert(distance >= 0);
	int d = distance;
	for (char row = zone.getRow() - d; row <= zone.getRow() + d; row++) {
		for (int col = zone.getCol() - d; col <= zone.getCol() + d; col++) {
			GridCoordinate gc(row, col);
			if (gc.distanceFrom(zone) <= distance
				&& (this->*zoneType)(gc))
			{
				return true;
			}
		}
	}
	return false;	
}
