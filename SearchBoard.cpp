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
bool SearchBoard::isBritishPatrolLine(const GridCoordinate& zone) const {
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

// Is this zone in the Irish Sea?
bool SearchBoard::isIrishSea(const GridCoordinate& zone) const {
	return layers[IrishSea].isBitOn(zone);
}

// Is this zone on a convoy route?
bool SearchBoard::isConvoyRoute(const GridCoordinate& zone) const {
	return layers[ConvoyRoutes].isBitOn(zone);
}

// Get a random sea zone within a given area
//   Area is defined by center zone and radius (inclusive)
GridCoordinate SearchBoard::randSeaZone(
	const GridCoordinate& center, int radius) const
{
	std::vector<GridCoordinate> seaZones;
	auto area = center.getArea(radius);
	for (auto zone: area) {
		if (isSeaZone(zone)) {
			seaZones.push_back(zone);
		}
	}
	assert(!seaZones.empty());
	return randomElem(seaZones);
}

// Is this zone within the given distance from some type of zone?
//   Pass one of the isXZone() functions as last argument
bool SearchBoard::isNearZoneType(const GridCoordinate& zone, int distance, 
		bool (SearchBoard::*zoneType)(const GridCoordinate& zone) const) const 
{
	assert(distance >= 0);
	auto area = zone.getArea(distance);
	for (auto zone: area) {
		if ((this->*zoneType)(zone)) {
			return true;			
		}
	}
	return false;	
}

// Is this zone within the area of the British patrol line?
//   That is: In or east of white dots on the map (Rule 10.211)
bool SearchBoard::isInsidePatrolLine(const GridCoordinate& zone) const {
	char row = zone.getRow();
	int col = zone.getCol();
	while (col > 0) {
		if (isBritishPatrolLine(GridCoordinate(row, col))) {
			return true;
		}
		col--;	
	}
	return false;	
}

// Get a list of all the German ports
std::vector<GridCoordinate> SearchBoard::getAllGermanPorts() const {
	return layers[GermanPorts].getAllOn();
}

// Get a list of all the convoy route zones
std::vector<GridCoordinate> SearchBoard::getAllConvoyRoutes() const {
	return layers[ConvoyRoutes].getAllOn();
}
