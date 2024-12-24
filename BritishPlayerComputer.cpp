#include "BritishPlayerComputer.h"
#include "GameDirector.h"
#include "Utils.h"
#include <cassert>
using namespace std;

// Constructor
BritishPlayerComputer::BritishPlayerComputer() {
	coastalFreeSearchList = getCoastalFreeSearchZones();
	initialAirPatrols = planInitialAirPatrols();
}

// Try searching
bool BritishPlayerComputer::trySearch() {
	return true;
}

// Search a vector of zones at given strengths
void BritishPlayerComputer::searchZones(
	const std::vector<GridCoordinate>& zones,
	int dayStrength, int nightStrength)
{
	auto director = GameDirector::instance();
	int visibility = director->getVisibility();
	for (auto zone: zones) {
		if (!director->isInFog(zone)) {
			int strength = director->isInNight(zone) ? 
				nightStrength : dayStrength;
			if (strength >= visibility) {
				director->searchGermanShips(zone);			
			}
		}
	}
}

// Basic search in first few turns
void BritishPlayerComputer::resolveSearch() {
	searchZones(coastalFreeSearchList, 4, 3);
	searchZones(getShipPatrolZones(), 4, 2);
	searchZones(getAirPatrolZones(), 6, 3);
}

// Compile the list of free coastal search spaces
vector<GridCoordinate> BritishPlayerComputer::getCoastalFreeSearchZones() 
{
	
	// Start with special island spaces
	vector<GridCoordinate> list = {"D9", "F15", "G18"};
			
	// Add everything from map layer of British/Irish coast
	for (char row = 'A'; row < 'Z'; row++) {
		for (int col = 1; col < 29; col++) {
			GridCoordinate zone(row, col);
			if (SearchBoard::instance()->isBritishCoast(zone)) {
				list.push_back(zone);
			}
		}
	}
	return list;
}

// Get list of ship patrol zones
vector<GridCoordinate> BritishPlayerComputer::getShipPatrolZones()
{
	// Standard ship patrols
	vector<GridCoordinate> list = {"B7", "D12", "E13", "F14", "G15", "H16"};
	
	// Suffolk somewhere off Iceland
	GridCoordinate suffolk('D', 3 + rollDice(2, 3));
	list.push_back(suffolk);
	return list;
}

// Distribute initial air patrol numbers
//   An abstracted estimate of early air search strength
vector<int> BritishPlayerComputer::planInitialAirPatrols() {
	const int NUM_DAYS = 5;
	const int TOTAL_AIR_PATROLS = 20;
	vector<int> patrolsPerDay(NUM_DAYS);
	for (int i = 0; i < TOTAL_AIR_PATROLS; i++) {
		int day = rollDice(2, 3) - 2;
		patrolsPerDay[day]++;
	}
	return patrolsPerDay;
}

// Get list of air patrol zones
vector<GridCoordinate> BritishPlayerComputer::getAirPatrolZones()
{
	vector<GridCoordinate> list;
	int turnsElapsed = GameDirector::instance()->getTurn()
		- GameDirector::START_TURN;
	if (turnsElapsed < (int) initialAirPatrols.size()) {
		int numPatrolsToday = initialAirPatrols[turnsElapsed];
		for (int i = 0; i < numPatrolsToday; i++) {
			GridCoordinate searchZone;
			do {
				searchZone = pickAirPatrolZone();
			} while (hasElem(list, searchZone));
			list.push_back(searchZone);			
		}
	}
	return list;
}

// Get a random zone for an air patrol
//   Column pick suggested by MicroBismarck LRS resultd
GridCoordinate BritishPlayerComputer::pickAirPatrolZone() {
	int col;
	char row = 'A' + rollDie(6) - 1;
	switch (rollDie(13)) {
		case 1: col = 18; break;
		case 2: col = 17; break;
		case 3: col = 16; break;
		default: col = 15; break;		
	}
	return GridCoordinate(row, col);	
}
