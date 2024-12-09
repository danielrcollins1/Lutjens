#include "Ship.h"
#include "Utils.h"
#include "GameDirector.h"
#include <cassert>
#include <algorithm>
using namespace std;

// Ship type abbreviations
const string Ship::typeAbbr[NUM_TYPES]
	= {"BB", "BC", "PB", "CA", "CL", "CV", "DD", "SS"};

// Ship type full names
const string Ship::typeName[NUM_TYPES]
	= {"Battleship", "Battlecruiser", "Pocket Battleship", 
		"Heavy Cruiser", "Light Cruiser", "Aircraft Carrier", 
		"Destroyer", "Submarine"};

// Constructor
Ship::Ship(string name, Type type, 
	int evasion, int midships, int fuel) 
{
	this->name = name;
	this->type = type;
	this->evasionMax = evasion;
	this->midshipsMax = midships;
	this->fuelMax = fuel;
	position = GridCoordinate::NO_ZONE;
	onPatrol = false;
	tookMoveTurn = false;
	loseMoveTurn = false;
	fuelLost = 0;
	midshipsLost = 0;
	evasionLostTemp = 0;
	evasionLostPerm = 0;
}

// Get the name
string Ship::getName() const {
	return name;	
}

// Get the full name of this type
string Ship::getTypeName() const {
	return typeName[type];	
}

// Get a descriptor of type & evasion (e.g., shadow prompt)
string Ship::getTypeAndEvasion() const {
	return getTypeName() 
		+ " (evrtg " + to_string(getEvasion()) + ")";
}

// Get a short descriptor (e.g., naval combat prompt)
string Ship::getShortDesc() const {
	return name
		+ " (" + typeAbbr[type]
		+ ", evrtg " + to_string(getEvasion())
		+ ", mships " + to_string(getMidships())
		+ ")";
}

// Get a long descriptor (e.g., logging purposes)
string Ship::getLongDesc() const {
	return name
		+ " (" + typeAbbr[type]
		+ ", evrtg " + to_string(getEvasion())
		+ ", mships " + to_string(getMidships())
		+ ", fuel " + to_string(getFuel())
		+ ", pos " + position.toString()
		+ (onPatrol ? ", patrol" : "")
		+ ")";
}

// Get the current evasion
int Ship::getEvasion() const {
	return max(0, evasionMax - evasionLostTemp - evasionLostPerm);
}

// Get the current midships
int Ship::getMidships() const {
	return max(0, midshipsMax - midshipsLost);
}

// Get the current fuel
int Ship::getFuel() const {
	return max(0, fuelMax - fuelLost);	
}

// Set starting position
void Ship::setPosition(const GridCoordinate& zone) {
	assert(moveHistory.empty());
	position = zone;
}

// Get current position
GridCoordinate Ship::getPosition() const {
	return position;	
}

// Are we on patrol?
bool Ship::isOnPatrol() const {
	return onPatrol;	
}

// Do setup in first phase of turn
void Ship::doAvailability() {
	tookMoveTurn = false;
	exposureHistory.push_back(false);
}

// Clear all waypoints
void Ship::clearWaypoints() {
	waypoints.clear();	
}

// Add a waypoint
void Ship::addWaypoint(const GridCoordinate& coord) {
	waypoints.push_back(coord);
}

// How many board spaces can we move this turn?
//   See Basic Game Tables Card: Movement on Search Board
int Ship::maxSpeed() const {
	int evasion = getEvasion();
	if (evasion <= 6) {
		return 0;
	}
	else if (evasion <= 15 || getFuel() <= 0) {
		int turn = GameDirector::instance()->getTurn();
		return turn % 2 ? 0 : 1;
	}
	else if (evasion <= 24) {
		return 1;		
	}
	else {
		int lastTurnSpeed = moveHistory.empty() ? 
			0 : moveHistory.back().size();
		return lastTurnSpeed > 1 ? 1 : 2;
	}
}

// Check if we reached a waypoint
void Ship::checkForWaypoint() {
	if (!waypoints.empty()) {
		if (position == waypoints[0]) {
			waypoints.erase(waypoints.begin());
		}
	}
}

// Do movement for turn
void Ship::doMovement() {
	auto board = SearchBoard::instance();
	vector<GridCoordinate> moves;

	// Abort if we already moved
	if (tookMoveTurn) {
		return;	
	}
	
	// Set to patrol if no waypoints
	onPatrol = waypoints.empty()
		&& !board->isGermanPort(position);

	// Select speed to move
	int speed = maxSpeed();
	if (getFuel() == 1 || rollDie(6) == 1) {
		speed = min(1, speed);	
	}
	if (onPatrol || loseMoveTurn) {
		speed = 0;
		loseMoveTurn = false;
	}
	
	// Try to perform movement
	for (int step = 0; step < speed; step++) {
		auto next = getNextZone();
		if (next != position) {
			position = next;
			moves.push_back(position);
			checkForWaypoint();
		}
	}

	// Account for fuel, history
	if (moves.size() == 2) {
		fuelLost++;	
	}
	moveHistory.push_back(moves);
	tookMoveTurn = true;
	checkEvasionRepair();
}

// Get one step towards destination
GridCoordinate Ship::getNextZone() const {
	if (!waypoints.empty()) {
		GridCoordinate dest = waypoints[0];
		int dist = position.distanceFrom(dest);
		vector<GridCoordinate> adjacent = position.getAdjacent();
		vector<GridCoordinate> options;
		for (auto zone: adjacent) {
			if (isAccessible(zone) 
				&& zone.distanceFrom(dest) < dist) 
			{
				options.push_back(zone);
			}
		}
		return randomElem(options);
	}
	return position;
}

// Are we in port?
bool Ship::isInPort() const {
	auto board = SearchBoard::instance();
	return board->isGermanPort(position);
}

// Did we enter port last turn?
bool Ship::enteredPort() const {
	return isInPort()
		&& !moveHistory.empty()
		&& !moveHistory.back().empty();
}

// Is this zone accessible to German ships?
bool Ship::isAccessible(const GridCoordinate& zone) const {
	auto board = SearchBoard::instance();
	return board->isSeaZone(zone)         // Rule 5.17
		&& !board->isBritishPort(zone)    // Rule 5.18
		&& !board->isBritishCoast(zone);  // Allowed by errata, but
		                                  // avoid for AI (b/c Rule 7.27)
}

// Did we move into/through a given zone this turn?
bool Ship::movedThrough(const GridCoordinate& zone) const {
	assert(tookMoveTurn);
	return hasElem(moveHistory.back(), zone);
}

// Are we sunk?
bool Ship::isSunk() const {
	return getMidships() <= 0;	
}

// Are we in the night?
bool Ship::isInNight() const {
	return GameDirector::instance()->isInNight(position);	
}

// Ae we in fog?
bool Ship::isInFog() const {
	return GameDirector::instance()->isInFog(position);
}

// Set if we must lose a move turn
void Ship::setLoseMoveTurn() {
	loseMoveTurn = true;
}

// Note that we have been located by the enemy
void Ship::setExposed() {
	assert(!exposureHistory.empty());
	exposureHistory.back() = true;
}

// Check if we were exposed on a given turn
bool Ship::wasExposed(int turnsAgo) const {
	assert(0 <= turnsAgo 
		&& turnsAgo < (int) exposureHistory.size());
	return exposureHistory.rbegin()[turnsAgo];
}

// How far did we move on the search board this turn?
int Ship::getSpeed() const {
	assert(tookMoveTurn);
	return 	moveHistory.back().size();
}

// Take damage to our midships
void Ship::loseMidships(int loss) {
	midshipsLost += loss;
	applyTempEvasionLoss(loss);
}
		
// Take damage to our evasion rating (permanently)
void Ship::loseEvasion(int loss) {
	evasionLostPerm += loss;
}

// Apply temporary evasion loss from midships hit (Rule 9.72)
//   For simplicity, we use the Bismarck & Prinz Eugen reductions
//   for all ships here (presumably only German ships)
void Ship::applyTempEvasionLoss(int midshipsLost) {
	int lossPerMidships;
	switch (type) {
		case BB: case BC: case PB:
			lossPerMidships = 1;   // Rule 9.724
			break;
		case CA: case CL:
			lossPerMidships = 3;   // Rule 9.725
			break;		
		default:
			lossPerMidships = 0;
			clog << "ERROR: Unhandled ship type "
				<< " for temp evasion loss.\n";
	}
	evasionLostTemp += lossPerMidships * midshipsLost;
}

// Check for evasion repair following movement (Rule 9.728)
void Ship::checkEvasionRepair() {
	assert(tookMoveTurn);
	if (evasionLostTemp && getSpeed() <= 1) {
		int repair = rollDie(6) * 2 - 4;
		repair = max(0, repair);
		repair = min(repair, evasionLostTemp);
		if (repair) {
			clog << name << " repairs " 
				<< repair << " evasion factor(s).\n";
			evasionLostTemp -= repair;
			assert(evasionLostTemp >= 0);			
		}
	}
}
