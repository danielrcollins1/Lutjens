#include "BritishPlayerComputer.h"
#include "GameDirector.h"
#include <cassert>

// Try searching
bool BritishPlayerComputer::trySearch() {
	return true;
}

// Basic search in first few turns
//   Free coastal search & basic air search
void BritishPlayerComputer::resolveSearch() {
	doFreeCoastalSearch();
	auto director = GameDirector::instance();
	int visibility = director->getVisibility();
	int turn = director->getTurn();
	
	// First turn: 2 spaces up to row A, and one up to row F
	if (turn == 4) {
		assert(visibility == 4);
		GridCoordinate search[] = {"A15", "B15", "G16"};
		for (auto zone: search) {
			director->checkSearch(zone);
		}
	}

	// Second turn: Search all A15-F15
	//   Patrol search strength 6, but 4 in one zone 
	//     (Plymouth pair up to row C)
	else if (turn == 5) {
		GridCoordinate search[] = {"A15", "B15", "C15", "E15", "F16"};
		if (visibility <= 6) {
			for (auto zone: search) {
				director->checkSearch(zone);
			}
		}
		if (visibility <= 4) {
			director->checkSearch("D15");			
		}
	}
	
	// Third turn: Patrol all A15-F15
	//   Plus one extra around D16
	else if (turn == 6) {
		GridCoordinate search[] = {"A15", "B15", "C15", "E15", "F16", "D17"};
		if (visibility <= 6) {
			for (auto zone: search) {
				director->checkSearch(zone);
			}
		}
	}

	// Fourth turn: Night turn (LR patrol search strength 3).
	// Plymouth LR recon return from area.
	// Eire units must be row F or below.
	// No one can be on row A (if maximized search earlier)
	// Can patrol B15-D15 w/Scapa & Hvaliford
	// Consider landing this turn to refit & go in morning?
	else if (turn == 7) {
		GridCoordinate search[] = {"B15", "C15", "D15", "F16"};
		if (visibility <= 3) {
			for (auto zone: search) {
				director->checkSearch(zone);
			}
		}
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

// Do our free coastal search
void BritishPlayerComputer::doFreeCoastalSearch() {
	for (char row = 'A'; row < 'Z'; row++) {
		for (int col = 1; col < 29; col++) {
			GridCoordinate zone(row, col);
			if (isFreeSearchable(zone)) {
				GameDirector::instance()->checkSearch(zone);
			}
		}
	}
}

// Non-coastal extra free search zones
const GridCoordinate extraFreeSearch[] = {"D9", "F15", "G18"};

// Do we get free search capacity in this zone? (Rule 7.27)
bool BritishPlayerComputer::isFreeSearchable(const GridCoordinate& zone) {
	auto director = GameDirector::instance();
	int visibility = director->getVisibility();
	if (visibility <= 3
		|| (visibility == 4 && (!director->isInNight(zone))))
	{

		// Coast of Britain or Ireland
		if (SearchBoard::instance()->isBritishCoast(zone)) {
			return true;	
		}
		
		// Other free search zones
		for (auto extra: extraFreeSearch) {
			if (extra == zone) {
				return true;	
			}
		}
	}
	return false;
}
