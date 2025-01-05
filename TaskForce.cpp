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
}

// Detach a ship
void TaskForce::detach(Ship* ship) {
	assert(includes(ship));
	auto it = find(shipList.begin(), shipList.end(), ship);
	shipList.erase(it);
	(*it)->leaveTaskForce();
}

// Detach all ships
void TaskForce::dissolve() {
	for (auto& ship: shipList) {
		ship->leaveTaskForce();	
	}
	shipList.clear();
}

// Do we control a given ship?
bool TaskForce::includes(Ship* ship) const {
	return hasElem(shipList, ship);
}

// Is this group empty?
bool TaskForce::isEmpty() const {
	return shipList.empty();
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

// Get the lead ship
Ship* TaskForce::getFlagship() const {
	assert(!isEmpty());	
	return shipList[0];	
}

// Get a description with only ship types
string TaskForce::getTypeDesc() const {
	assert(!isEmpty());	
	string desc = "Task Force (";
	for (unsigned i = 0; i < shipList.size(); i++) {
		desc += (i ? ", " : "") + shipList[i]->getClassTypeName();
	}
	desc += ")";
	return desc;
}

// Get a description with full information
string TaskForce::getFullDesc() const {
	assert(!isEmpty());	
	string desc = "Task Force " + to_string(identifier) + ": ";
	for (auto& ship: shipList) {
		desc += "  " + ship->getLongDesc() + "\n";		
	}
	return desc;
}

// Do movement for one turn
void TaskForce::doMovement() {
	assert(!isEmpty());	
	auto flagship = getFlagship();
	flagship->doMovement();
	for (auto& ship: shipList) {
		if (ship != flagship) {
			ship->moveWithTaskForce();
		}
	}
}
