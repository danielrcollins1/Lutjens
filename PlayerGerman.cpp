#include "PlayerGerman.h"
#include "GameDirector.h"
#include "GameStream.h"
#include "Utils.h"
#include <cassert>

// Constructor
PlayerGerman::PlayerGerman() {
	Ship bb("Bismarck", Ship::Type::BB, 29, 10, 13);
	bb.setPosition("F20");
	initWaypoints(bb);
	shipList.push_back(bb);
	bismarck = &shipList[0];
}

// Get Bismarck for read-only
const Ship& PlayerGerman::getBismarck() const {
	assert(bismarck != nullptr);
	return *bismarck;
}

// Initialize a German ship's path waypoints
void PlayerGerman::initWaypoints(Ship& ship) {
	auto board = SearchBoard::instance();
	ship.clearWaypoints();
	
	// Breakout path
	switch(rollDie(6)) {
		case 1: // near path
			ship.addWaypoint(board->randSeaWithinOne("E17"));
			ship.addWaypoint(GridCoordinate("G16"));
			break;
		case 2: case 3: // middle path
			ship.addWaypoint(board->randSeaWithinOne("C14"));
			ship.addWaypoint(board->randSeaWithinOne("E13"));
			break;
		default: // far path
			ship.addWaypoint(board->randSeaWithinOne("B11"));
			ship.addWaypoint(GridCoordinate("B7"));
			ship.addWaypoint(board->randSeaWithinOne("C6"));
			break;	
	}
	
	// Initial convoy route target
	pickConvoyTarget(ship, 4);
}

// Pick a random convoy target
//   Argument is chance in 6 to pick Atlantic route
void PlayerGerman::pickConvoyTarget(Ship& ship, int chanceAtlantic) {
	GridCoordinate target;
	do {
		target = rollDie(6) <= chanceAtlantic ? 
			convoyTargetAtlantic() : convoyTargetAfrican();	
	} while (target == ship.getPosition());
	ship.addWaypoint(target);
	clog << ship.getName() << " picks target: " << target << endl;
}

// Get a convoy target on the Atlantic line
//   On row H, biased to western edge
GridCoordinate PlayerGerman::convoyTargetAtlantic() const {
	char row = 'H';
	int col = min(rollDie(10), rollDie(10));
	GridCoordinate goal(row, col);
	assert(SearchBoard::instance()->isConvoyRoute(goal));
	return goal;
}

// Get a convoy target on the African line
//   Row R to Z, centered on V18
GridCoordinate PlayerGerman::convoyTargetAfrican() const {
	int roll = rollDice(2, 5);
	char row = 'P' + roll;
	int col = 16 + (roll - 1) / 2;
	GridCoordinate goal(row, col);
	assert(SearchBoard::instance()->isConvoyRoute(goal));
	return goal;
}

// Do unit availability phase
void PlayerGerman::doAvailabilityPhase() {
	for (auto& ship: shipList) {
		ship.doAvailability();
	}
}

// Do shadow phase
void PlayerGerman::doShadowPhase() {
	for (auto& ship: shipList) {
		if (ship.wasExposed(1)) {
			GameDirector::instance()
				->checkShadow(ship, ship.getPosition(), false);
		}
	}
}

// Do sea movement phase
void PlayerGerman::doSeaMovementPhase() {
	for (auto& ship: shipList) {
		if (!ship.isSunk()) {
			ship.doMovement();
			clog << ship.getLongDesc() << endl;
		}
	}
}

// Check for search by British player
void PlayerGerman::checkSearch(const GridCoordinate& zone) {
	for (auto& ship: shipList) {
 		if (ship.getPosition() == zone) {
			cgame << ship.getTypeName() 
				<< " found in " << zone << endl;
			ship.setExposed();
		}
		else if (ship.movedThrough(zone)) {
			cgame << ship.getTypeAndEvasion()
				<< " seen moving through " << zone << endl;
			GameDirector::instance()->checkShadow(ship, zone, true);
		}
	}
}

// Do air attack phase
void PlayerGerman::doAirAttackPhase() {
	for (auto& ship: shipList) {
		if (ship.wasExposed(0)) {
			GameDirector::instance()->checkAttack(ship, false);
		}
	}
}

// Do naval combat phase
void PlayerGerman::doNavalCombatPhase() {
	for (auto& ship: shipList) {
		if (ship.wasExposed(0)) {
			GameDirector::instance()->checkAttack(ship, true);
		}
	}
}

// Call result of British HUFF-DUFF detection
void PlayerGerman::callHuffDuff() {
	cgame << "HUFF-DUFF: German ship near "
		<< SearchBoard::instance()
			->randSeaWithinOne(bismarck->getPosition())
		<< endl;
}

// General search results
//   See Basic Game Tables Card: Chance Table
const int GS_ROWS = 7;
const int GS_COLS = 3;
const int GS_VALUES[GS_ROWS][GS_COLS] = {
	{3, 5, 6},
	{2, 4, 5},
	{0, 1, 2},
	{2, 3, 4},
	{0, 1, 2},
	{1, 2, 3},
	{1, 2, 3}
};

// Check a general search result
//   See Basic Game Tables Card: Chance Table
void PlayerGerman::checkGeneralSearch(int roll) {
	assert(3 <= roll && roll <= 9);
	auto ship = bismarck;
	auto pos = ship->getPosition();
	
	// Check if general search possible
	if (!ship->isInNight()       // Rule 11.13
		&& !ship->isInFog()      // Rule 10.213
		&& pos.getRow() >= 'E'   // Rule 10.211
		&& pos.getCol() >= SearchBoard::instance()
			->getPatrolLimitCol(pos.getRow()))
	{
		// Look up search strength
		char colLetter = generalSearchColumn(pos);
		int gsRowIdx = roll - 3;
		int gsColIdx = colLetter - 'A';
		int searchStrength = GS_VALUES[gsRowIdx][gsColIdx];
		
		// Announce result
		int visibility = GameDirector::instance()->getVisibility();
		if (visibility <= searchStrength) {
			cgame << "General Search: " << ship->getName() 
				<< " found in " << pos << endl;
		}
	}
}

// Find the applicable general search table row
//   As per rule 10.214.
char PlayerGerman::generalSearchColumn(const GridCoordinate& zone) {
	
	// Case 'A': near patrol line limit
	if (zone.getCol() <= SearchBoard::instance()
		->getPatrolLimitCol(zone.getRow()) + 2)
	{
		return 'A';	
	}

	// Case 'C': near coast of Britain/Ireland
	else if (SearchBoard::instance()
		->isNearZoneType(zone, 2, &SearchBoard::isBritishCoast))
	{
		return 'C';		
	}

	// Case 'B': any other location
	else 
	{
		return 'B';	
	}
}

// Resolve a convoy result from the Chance Table
void PlayerGerman::checkConvoyResult(int roll) {
	assert(10 <= roll && roll <= 12);
	auto board = SearchBoard::instance();
	auto ship = bismarck;
	auto pos = ship->getPosition();
	if (!ship->wasExposed(0)     // Rule 10.231
		&& !ship->isInNight())   // Rule 11.13
	{
		switch (roll) {
	
			// On convoy route
			case 10:
				if (board->isConvoyRoute(pos)) {
					destroyConvoy();				
				}
				break;
	
			// On patrol and within two
			case 11:
				if (ship->isOnPatrol()
					&& board->isNearZoneType(pos, 2, 
						&SearchBoard::isConvoyRoute))
				{
					destroyConvoy();				
				}
				break;
				
			// One zone from convoy route
			case 12:
				if (board->isNearZoneType(pos, 1, 
					&SearchBoard::isConvoyRoute))
				{
					destroyConvoy();
				}
				break;
		}
	}
}

// Score destruction of a convoy
//   And re-route to new destination
void PlayerGerman::destroyConvoy() {
	auto ship = bismarck;
	auto location = ship->getPosition();
	cgame << "CONVOY SUNK: In zone " << location 
		<< " by " << ship->getName() << endl;
	GameDirector::instance()->msgSunkConvoy();
	ship->setLoseMoveTurn();   // Rule 10.25
	pickNewRoute();
}

// After destroying a convoy, pick a new target
void PlayerGerman::pickNewRoute() {
	auto ship = bismarck;
	ship->clearWaypoints();
	bool isOnAtlantic = ship->getPosition().getRow() <= 'K';
	int chanceAtlantic = isOnAtlantic ? 1: 5;
	pickConvoyTarget(*ship, chanceAtlantic);
}

// Print all of our ships (e.g., for end game)
void PlayerGerman::printAllShips() const {
	for (auto& ship: shipList) {
		cgame << ship.getLongDesc() << endl;
	}
}
