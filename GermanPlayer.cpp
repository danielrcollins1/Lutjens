#include "GermanPlayer.h"
#include "GameDirector.h"
#include "GameStream.h"
#include "Utils.h"
#include <cassert>
using namespace std;

// Constructor
GermanPlayer::GermanPlayer() {
	Ship bb("Bismarck", Ship::Type::BB, 29, 10, 13);
	bb.setPosition("F20");
	initWaypoints(bb);
	shipList.push_back(bb);
	flagship = &shipList[0];
}

// Get flagship for read-only
const Ship& GermanPlayer::getFlagship() const {
	assert(flagship != nullptr);
	return *flagship;
}

// Initialize a German ship's path waypoints
void GermanPlayer::initWaypoints(Ship& ship) {
	auto board = SearchBoard::instance();
	ship.clearWaypoints();

	// First-turn location random in box A-F, 15-18
	//   (Note mini-game LRS solutions suggested row ratios 3:3:4,
	//   and columns 1:4:4:4; but testing here shows otherwise.)
	char row = rollDie(100) <= 90 ?
		'A' + rollDie(4) - 1 : 'E' + rollDie(2) - 1;
	int col = 14 + rollDie(4);
	GridCoordinate firstMove(row, col);
	if (firstMove == GridCoordinate("F15")) { // Avoid Faeroe
		firstMove = GridCoordinate("G16");
	}
	ship.addWaypoint(firstMove);
	bool denmarkStrait = false;
	
	// Northerly escape then breaks out near Iceland
	if (firstMove.getRow() <= 'D') {
	
		// Get past air picket on column 15
		ship.addWaypoint(
			GridCoordinate(firstMove.getRow(), 15 - rollDie(3)));

		// North of Iceland (Denmark Straight)
		if (rollDie(100) <= 85) {
			ship.addWaypoint("B7");
			denmarkStrait = true;
		}

		// South of Iceland
		else {
			switch (rollDie(6)) {
				case 1: ship.addWaypoint("F14"); break;
				case 2: case 3: ship.addWaypoint("E13"); break;
				default: ship.addWaypoint("E12"); break;
			}
		}
	}
	
	// Row 'E' breaks out north of Faeroe
	else if (firstMove.getRow() == 'E') {
		ship.addWaypoint("E14");
		ship.addWaypoint("F14");
	}
	
	// Row 'F' breaks out south of Faeroe
	else {
		ship.addWaypoint(board->randSeaWithinOne("G16"));
	}
	
	// Initial convoy route target
	denmarkStrait ? 
		targetAtlanticConvoy(ship) : targetAnyConvoyBreakout(ship);
	//ship.printWaypoints();
}

// Target a convoy on the Atlantic line
//   Around row H, on western edge past patrol line
void GermanPlayer::targetAtlanticConvoy(Ship& ship) {
	int col;
	char row = 'F' + rollDie(3);
	switch (row) {
		case 'G': col = rollDie(4); break;
		case 'H': col = rollDie(5); break;
		case 'I': col = rollDie(5) + 1; break;
		default: cerr << "Error in Atlantic convoy column selection.\n";				
	}
	GridCoordinate target(row, col);
	ship.addWaypoint(target);
	clog << ship.getName() << " targets Atlantic convoy @ " << target << endl;
}

// Target a convoy on the African line
//   Row P to Z, directly on convoy route
void GermanPlayer::targetAfricanConvoy(Ship& ship) {
	int roll = rollDie(11);
	char row = 'O' + roll;
	int col = 15 + roll / 2;
	GridCoordinate target(row, col);
	ship.addWaypoint(target);
	clog << ship.getName() << " targets African convoy @ " << target << endl;
}

// Target any convoy right after initial breakout
void GermanPlayer::targetAnyConvoyBreakout(Ship& ship) {
	int rollRoute = rollDie(100);
	
	// Target Atlantic convoy
	if (rollRoute <= 50) {
		targetAtlanticConvoy(ship);		
	}
	
	// Target African convoy
	//   First, get past patrol line ASAP
	else {
		int roll = rollDie(5); // L11 to P15, past patrol
		ship.addWaypoint(GridCoordinate('K' + roll, 10 + roll));
		targetAfricanConvoy(ship);	
	}
}

// Do unit availability phase
void GermanPlayer::doAvailabilityPhase() {
	foundShipZones.clear();
	for (auto& ship: shipList) {
		ship.doAvailability();
	}
}

// Do shadow phase
void GermanPlayer::doShadowPhase() {
	for (auto& ship: shipList) {
		if (ship.wasLocated(1)) {
			GameDirector::instance()
				->checkShadow(ship, ship.getPosition(), false);
		}
	}
}

// Do sea movement phase
void GermanPlayer::doSeaMovementPhase() {
	for (auto& ship: shipList) {
		if (!ship.isSunk()) {
			ship.doMovement();
			clog << ship << endl;
		}
	}
}

// Check for search by British player
bool GermanPlayer::checkSearch(const GridCoordinate& zone) {
	bool anyFound = false;
	auto director = GameDirector::instance();
	for (auto& ship: shipList) {
 		if (ship.getPosition() == zone) 
		{
			cgame << ship.getTypeName() 
				<< " found in " << zone << endl;
			ship.setLocated();
			anyFound = true;
		}
		else if (ship.movedThrough(zone)
			&& director->isPassThroughSearchOn())
		{
			cgame << ship.getTypeAndEvasion()
				<< " seen moving through " << zone << endl;
			director->checkShadow(ship, zone, true);
			anyFound = true;
		}
	}
	return anyFound;
}

// Do air attack phase
void GermanPlayer::doAirAttackPhase() {
	for (auto& ship: shipList) {
		if (ship.wasLocated(0)) {
			GameDirector::instance()->checkAttack(ship, false);
		}
	}
}

// Do naval combat phase
void GermanPlayer::doNavalCombatPhase() {
	for (auto& ship: shipList) {
		if (ship.wasLocated(0)) {
			GameDirector::instance()->checkAttack(ship, true);
		}
	}
}

// Do chance phase
//   See Basic Game Tables Card: Chance Table
//   While RAW says British player makes this roll (Rule 10.1),
//   it makes more sense for us with knowledge of ships on board.
void GermanPlayer::doChancePhase() {
	for (auto& ship: shipList) {
		int roll = rollDice(2, 6);
		
		// Huff-duff result
		if (roll == 2) {
			callHuffDuff(ship);
		}
	
		// General Search results
		else if (roll <= 9) {
			checkGeneralSearch(ship, roll);
		}
		
		// Convoy results
		else if (roll <= 12) {
			auto game = GameDirector::instance();
			if (!game->wasConvoySunk(0)         // Rule 10.26
				&& game->getVisibility() < 9)   // Errata in General 16/2
			{			
				checkConvoyResult(ship, roll);
			}
		}
		
		// Error check
		else {
			cerr << "Unhandled roll in Chance Phase.\n";
		}
	}
}

// Call result of British HUFF-DUFF detection
void GermanPlayer::callHuffDuff(Ship& ship) {
	ship.noteDetected();
	cgame << "HUFF-DUFF: German ship near "
		<< SearchBoard::instance()
			->randSeaWithinOne(ship.getPosition())
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
void GermanPlayer::checkGeneralSearch(Ship& ship, int roll) {
	assert(3 <= roll && roll <= 9);
	auto pos = ship.getPosition();
	
	// Check if general search possible
	if (!ship.isInNight()       // Rule 11.13
		&& !ship.isInFog()      // Rule 10.213
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
			ship.noteDetected();
			cgame << "General Search: " << ship.getName() 
				<< " found in " << pos << endl;
		}
	}
}

// Find the applicable general search table row
//   As per rule 10.214.
char GermanPlayer::generalSearchColumn(const GridCoordinate& zone) {
	
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
void GermanPlayer::checkConvoyResult(Ship& ship, int roll) {
	assert(10 <= roll && roll <= 12);
	auto board = SearchBoard::instance();
	auto pos = ship.getPosition();
	if (!ship.wasLocated(0)     // Rule 10.231
		&& !ship.isInNight())   // Rule 11.13
	{
		switch (roll) {
	
			// On convoy route
			case 10:
				if (board->isConvoyRoute(pos)) {
					destroyConvoy(ship);				
				}
				break;
	
			// On patrol and within two
			case 11:
				if (ship.isOnPatrol()
					&& board->isNearZoneType(pos, 2, 
						&SearchBoard::isConvoyRoute))
				{
					destroyConvoy(ship);				
				}
				break;
				
			// One zone from convoy route
			case 12:
				if (board->isNearZoneType(pos, 1, 
					&SearchBoard::isConvoyRoute))
				{
					destroyConvoy(ship);
				}
				break;
		}
	}
}

// Score destruction of a convoy
//   And re-route to new destination
void GermanPlayer::destroyConvoy(Ship& ship) {
	auto location = ship.getPosition();
	cgame << "CONVOY SUNK: In zone " << location 
		<< " by " << ship.getName() << endl;
	GameDirector::instance()->msgSunkConvoy();
	ship.setLoseMoveTurn();   // Rule 10.25
	pickNewRoute();
}

// After destroying a convoy, pick a new target
void GermanPlayer::pickNewRoute() {
	auto ship = flagship;
	ship->clearWaypoints();
	bool isOnAtlantic = ship->getPosition().getRow() <= 'K';
	int chanceAtlantic = isOnAtlantic ? 1: 5;
	
	// Make sure to not pick current position
	while (!ship->hasWaypoints()) {
		rollDie(6) <= chanceAtlantic ?
			targetAtlanticConvoy(*ship) : targetAfricanConvoy(*ship);		
	}
}

// Print all of our ships (e.g., for end game)
void GermanPlayer::printAllShips() const {
	for (auto& ship: shipList) {
		cgame << ship << endl;
	}
}

// How many times was our flagship detected?
int GermanPlayer::getTimesFlagshipDetected() const {
	return flagship->getTimesDetected();
}

// Do we want to search now?
bool GermanPlayer::trySearch() {
	for (auto& ship: shipList) {
		if (ship.wasShadowed(0)) {
			return true;
		}
	}
	return false;
}

// Resolve any search attempts
void GermanPlayer::resolveSearch() {
	for (auto& ship: shipList) {
		if (ship.wasShadowed(0)) {
			auto game = GameDirector::instance();

			// Get search strength
			int searchStrength = 1;
			if (ship.isOnPatrol()) {
				searchStrength = ship.isInDay() ? 4 : 3;
			}
			
			// Do the search
			if (searchStrength >= game->getVisibility()) {
				auto zone = ship.getPosition();
				if (game->searchBritishShips(zone)) {
					foundShipZones.push_back(zone);
					clog << "British ships found in " << zone << "\n";
				}
			}
		}
	}
}
