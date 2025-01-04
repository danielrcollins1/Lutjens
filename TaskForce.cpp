#include "TaskForce.h"
#include "Utils.h"
#include <cassert>

// Constructor a new task force
TaskForce::TaskForce (int id) {
	identifier = id;	
}

// Get our identifying number
int TaskForce::getId() const {
	return identifier;	
}

// Attach a ship
void TaskForce::attach(Ship* ship) {
	assert(!includes(ship));
	ships.push_back(ship);
}

// Detach a ship
void TaskForce::detach(Ship* ship) {
	assert(includes(ship));
	ships.erase(find(ships.begin(), ships.end(), ship));
}

// Do we control a given ship?
bool TaskForce::includes(Ship* ship) const {
	return hasElem(ships, ship);
}

// Is this group empty?
bool TaskForce::isEmpty() const {
	return ships.empty();	
}

// Get the standard evasion level
//   That is: Evasion of the slowest ship
int TaskForce::getEvasion() const {
	int lowest = INT_MAX;
	for (auto ship: ships) {
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
	for (auto ship: ships) {
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
	for (auto ship: ships) {
		if (ship->getMaxSpeedClass() < lowest) {
			lowest = ship->getMaxSpeedClass();	
		}
	}
	return lowest;
}
