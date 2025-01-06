#include "TaskForce.h"
#include "Utils.h"
#include <cassert>
using namespace std;

// Initialize static counter
int TaskForce::numMade = 0;

// Construct a new task force
TaskForce::TaskForce () {
	identifier = ++numMade;
}

// Get our identifying number
int TaskForce::getId() const {
	return identifier;	
}

// Attach a ship
void TaskForce::attach(Ship* ship) {
	assert(!includes(ship));
	shipList.push_back(ship);
	ship->joinTaskForce(this);
	clog << getName() << " attached " << ship->getName() << "\n";
}

// Detach a ship
void TaskForce::detach(Ship* ship) {
	assert(includes(ship));
	shipList.erase(find(shipList.begin(), shipList.end(), ship));
	ship->leaveTaskForce();
	clog << getName() << " detached " << ship->getName() << "\n";
}

// Detach all ships
void TaskForce::dissolve() {
	clog << getName() << " dissolving\n";
	for (auto& ship: shipList) {
		detach(ship);
	}
}

// Do we control a given ship?
bool TaskForce::includes(Ship* ship) const {
	return hasElem(shipList, ship);
}

// Is this group empty?
bool TaskForce::isEmpty() const {
	return shipList.empty();
}

// How many ships do we have?
int TaskForce::getSize() const {
	return shipList.size();	
}

// Get the lead ship
Ship* TaskForce::getFlagship() const {
	if (isEmpty()) {
		assert(false);	
	}
	return shipList[0];	
}

// Get a ship by index
Ship* TaskForce::getShip(int idx) {
	assert(isInInterval(0, idx, shipList.size() - 1));
	return shipList[idx];
}

//
// NavalUnit overrides
//

// Get our name
string TaskForce::getName() const {
	return "Task Force " + to_string(identifier);
}

// Get a description with only ship types
string TaskForce::getTypeDesc() const {
	assert(!isEmpty());	
	string desc = "Task Force (";
	for (unsigned i = 0; i < shipList.size(); i++) {
		desc += (i ? ", " : "") + shipList[i]->getTypeDesc();
	}
	desc += ")";
	return desc;
}

// Get a description with ship names
string TaskForce::getNameDesc() const {
	assert(!isEmpty());	
	string desc = getName() + " (";
	for (unsigned i = 0; i < shipList.size(); i++) {
		desc += (i ? ", " : "") + shipList[i]->getName();
	}
	desc += ")";
	return desc;
}

// Get a description with full information
string TaskForce::getFullDesc() const {
	assert(!isEmpty());	
	string desc = getName();
	for (auto& ship: shipList) {
		desc += "\n  " + ship->getFullDesc();
	}
	return desc;
}

// Accessors
GridCoordinate TaskForce::getPosition() const {
	return getFlagship()->getPosition();
}

// Get the max speed class on the search board
int TaskForce::getMaxSpeedClass() const {
	int lowest = INT_MAX;
	for (auto& ship: shipList) {
		if (ship->getMaxSpeedClass() < lowest) {
			lowest = ship->getMaxSpeedClass();	
		}
	}
	return lowest;
}

// Get the standard evasion level
//   That is: Evasion of the slowest ship
int TaskForce::getEvasion() const {
	int lowest = INT_MAX;
	for (auto& ship: shipList) {
		if (ship->getEvasion() < lowest) {
			lowest = ship->getEvasion();	
		}
	}
	return lowest;
}

// Get the attack evasion level
//   That is: Evasion of the fastest ship
//   See Rule 9.222 (CVs barred by errata)
int TaskForce::getAttackEvasion() const {
	int highest = 0;
	for (auto& ship: shipList) {
		if (ship->getEvasion() > highest
			&& ship->getType() != Ship::CV)
		{		
			highest = ship->getEvasion();	
		}
	}
	return highest;
}

// Are we in day?
bool TaskForce::isInDay() const {
	return getFlagship()->isInDay();	
}
	
// Are we in night?
bool TaskForce::isInNight() const {
	return getFlagship()->isInNight();
}

// Are we in fog?
bool TaskForce::isInFog() const {
	return getFlagship()->isInFog();
}

// Are we in port?
bool TaskForce::isInPort() const {
	return getFlagship()->isInPort();
}

// Are we entering the port this turn?
bool TaskForce::isEnteringPort() const {
	return getFlagship()->isEnteringPort();
}

// Are we required to return to base?
bool TaskForce::isReturnToBase() const {
	return getFlagship()->isReturnToBase();
}

// Are we on patrol?
bool TaskForce::isOnPatrol() const {
	return getFlagship()->isOnPatrol();
}

// Were we located on a given turn?
bool TaskForce::wasLocated(unsigned turnsAgo) const {
	return getFlagship()->wasLocated(turnsAgo);
}

// Were we shadowed on a given turn?
bool TaskForce::wasShadowed(unsigned turnsAgo) const {
	return getFlagship()->wasShadowed(turnsAgo);
}

// Were we combated on a given turn?
bool TaskForce::wasCombated(unsigned turnsAgo) const {
	return getFlagship()->wasCombated(turnsAgo);
}

// Did we move through a space on our last move?
bool TaskForce::movedThrough(const GridCoordinate& zone) const {
	return getFlagship()->movedThrough(zone);
}

// Do movement for one turn
void TaskForce::doMovementTurn() {
	assert(!isEmpty());
	auto flagship = getFlagship();
	flagship->doMovementTurn();
	for (auto& ship: shipList) {
		if (ship != flagship) {
			ship->moveWithShip(*flagship);
		}
	}
}

// Note that we were located by search/shadow
void TaskForce::setLocated() {
	for (auto& ship: shipList) {
		ship->setLocated();		
	}
}

// Note that we were shadowed
void TaskForce::setShadowed() {
	for (auto& ship: shipList) {
		ship->setShadowed();
	}
}

// Note that we were combated
void TaskForce::setCombated() {
	for (auto& ship: shipList) {
		ship->setCombated();
	}
}

// Note that we must lose a turn
void TaskForce::setLoseMoveTurn() {
	for (auto& ship: shipList) {
		ship->setLoseMoveTurn();		
	}
}

// Note that we were detected in any way
void TaskForce::noteDetected() {
	for (auto& ship: shipList) {
		ship->noteDetected();
	}
}

// Clear all orders
void TaskForce::clearOrders() {
	for (auto& ship: shipList) {
		ship->clearOrders();
	}
}
