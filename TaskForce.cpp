#include "TaskForce.h"
#include "Utils.h"
#include <cassert>
using namespace std;

// Construct a new task force
TaskForce::TaskForce (int id) {
	identifier = id;
	clog << getName() << " forming\n";
}

// Destructor
TaskForce::~TaskForce() {
}

// Get our identifying number
int TaskForce::getId() const {
	return identifier;	
}

// Attach a ship
void TaskForce::attach(Ship* ship) {
	assert(ship->getGeneralType() != Ship::SUBMARINE); // Rule 22.14
	shipList.push_back(ship);
	ship->joinTaskForce(this);
	clog << getName() << " attached " << ship->getName() << "\n";
}

// Detach a ship
void TaskForce::detach(Ship* ship) {
	auto it = find(shipList.begin(), shipList.end(), ship);
	assert(it != shipList.end());
	shipList.erase(it);
	ship->leaveTaskForce();
	clog << getName() << " detached " << ship->getName() << "\n";
}

// Detach all ships
void TaskForce::dissolve() {
	clog << getName() << " dissolving\n";
	while (!shipList.empty()) {
		detach(shipList.front());
	}
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
const Ship* TaskForce::getFlagship() const {
	assert(!isEmpty());
	return shipList.front();	
}

// Get the lead ship (non-const)
Ship* TaskForce::getFlagship() {
	// Per Scott Meyers: stackoverflow.com/a/123995/5407757
	return const_cast<Ship*>
		(static_cast<const TaskForce&>(*this).getFlagship());
}

// Get a ship by index
Ship* TaskForce::getShip(int idx) {
	assert(isInInterval(0, idx, shipList.size() - 1));
	return shipList[idx];
}

// Give all follower ships an order
void TaskForce::orderFollowers(Ship::OrderType order) {
	for (unsigned i = 1; i < shipList.size(); i++) {
		shipList[i]->orderAction(order);
	}
}

// Equality operator
bool TaskForce::operator==(const TaskForce& other) const {
	return identifier == other.identifier;	
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

// Get our position
GridCoordinate TaskForce::getPosition() const {
	return getFlagship()->getPosition();
}

// How far did we move on the search board this turn?
int TaskForce::getSpeedThisTurn() const {
	return getFlagship()->getSpeedThisTurn();
}

// Get the max speed class on the search board
int TaskForce::getMaxSpeedClass() const {
	int lowest = INT_MAX;
	for (auto& ship: shipList) {
		int speedClass = ship->getMaxSpeedClass();
		if (speedClass < lowest) {
			lowest = speedClass;
		}
	}
	assert(lowest < INT_MAX);
	return lowest;
}

// How many board spaces can we move this turn?
int TaskForce::getMaxSpeedThisTurn() const {
	int lowest = INT_MAX;
	for (auto& ship: shipList) {
		int speed = ship->getMaxSpeedThisTurn();
		if (speed < lowest) {
			lowest = speed;
		}
	}
	assert(lowest < INT_MAX);
	return lowest;
}

// Get the standard evasion level
//   That is: Evasion of the slowest ship
int TaskForce::getEvasion() const {
	int lowest = INT_MAX;
	for (auto& ship: shipList) {
		int evasion = ship->getEvasion();
		if (evasion < lowest) {
			lowest = evasion;
		}
	}
	assert(lowest < INT_MAX);
	return lowest;
}

// Get the attack evasion level
//   That is: Evasion of the fastest ship (Rule 9.222)
int TaskForce::getAttackEvasion() const {
	int highest = 0;
	for (auto& ship: shipList) {
		if (ship->getType() != Ship::CV) { // Errata
			int evasion = ship->getEvasion();
			if (evasion > highest) {
				highest = evasion;
			}
		}
	}
	return highest;
}

// Get our current search strength
int TaskForce::getSearchStrength() const {

	// If on patrol, get highest ship strength (Rule 5.45)
	if (isOnPatrol()) {
		int highest = 0;
		for (auto& ship: shipList) {
			int strength = ship->getSearchStrength();
			if (strength > highest) {
				highest = strength;
			}
		}
		return highest;
	}
	
	// If not on patrol, strength is 1 (per game counters)
	return 1;
}

// Are any ships afloat?
bool TaskForce::isAfloat() const {
	for (auto& ship: shipList) {
		if (ship->isAfloat()) {
			return true;
		}
	}
	return false;
}

// Are we on the search board?
bool TaskForce::isOnBoard() const {
	return isAfloat()
		&& getPosition() != GridCoordinate::OFFBOARD;	
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

// Did any ship sink a convoy on a given turn?
bool TaskForce::wasConvoySunk(unsigned turnsAgo) const {
	for (auto& ship: shipList) {
		if (ship->wasConvoySunk(turnsAgo)) {
			return true;
		}
	}
	return false;	
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
			ship->followShip(*flagship);
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

// Note that we sank a convoy
void TaskForce::setConvoySunk() {
	for (auto& ship: shipList) {
		ship->setConvoySunk();
	}
}

// Note that we were detected in any way
void TaskForce::setDetected() {
	for (auto& ship: shipList) {
		ship->setDetected();
	}
}
