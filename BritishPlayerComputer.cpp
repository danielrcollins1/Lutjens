#include "BritishPlayerComputer.h"
#include "GameDirector.h"
#include <cassert>
using namespace std;

// Constructor
BritishPlayerComputer::BritishPlayerComputer() {
	coastalFreeSearchList = compileCoastalFreeSearchZones();
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
		int strength = director->isInNight(zone) ? 
			nightStrength : dayStrength;
		if (strength >= visibility) {
			director->checkSearch(zone);			
		}
	}
}

// Basic search in first few turns
//   Free coastal search & basic air search
void BritishPlayerComputer::resolveSearch() {
	searchZones(coastalFreeSearchList, 4, 3);
	int turn = GameDirector::instance()->getTurn();
	
	// First turn: 2 spaces up to row A, and one up to row F
	if (turn == 4) {
		vector<GridCoordinate> search =
			{"A15", "B15", "G16"};
		searchZones(search, 4, 2);
	}

	// Second turn: Search all A15-F15
	//   Patrol search strength 6, but 4 in one zone 
	//     (Plymouth pair up to row C)
	else if (turn == 5) {
		vector<GridCoordinate> search = 
			{"A15", "B15", "C15", "D15", "E15", "F16"};
		searchZones(search, 6, 3);
	}
	
	// Third turn: Patrol all A15-F15
	//   Plus one extra around D16
	else if (turn == 6) {
		vector<GridCoordinate> search =
			{"A15", "B15", "C15", "E15", "F16", "D17"};
		searchZones(search, 6, 3);
	}

	// Fourth turn: Night turn (LR patrol search strength 3).
	// Plymouth LR recon return from area.
	// Eire units must be row F or below.
	// No one can be on row A (if maximized search earlier)
	// Can patrol B15-D15 w/Scapa & Hvaliford
	// Consider: Land this turn to refit & go in morning?
	//   (Esp. if visibility poor.)
	else if (turn == 7) {
		vector<GridCoordinate> search = 
			{"B15", "C15", "D15", "F16"};
		searchZones(search, 6, 3);
	}

	// Fifth turn: All LR recon units land
	// (if maximized before this turn)
	else if (turn == 8) {
		// no search
	}

	// Sixth turn: All LR recon units refuel & refit
	// (if maximized at start)
	else if (turn == 9) {
		// no search
	}
}

// Compile the list of free coastal search spaces
vector<GridCoordinate> BritishPlayerComputer::compileCoastalFreeSearchZones() 
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
