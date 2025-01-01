#include "GermanPlayer.h"
#include "GameDirector.h"
#include "SearchBoard.h"
#include "GameStream.h"
#include "CmdArgs.h"
#include "Utils.h"
#include <cassert>
using namespace std;

// Constructor
GermanPlayer::GermanPlayer() {
	Ship bb("Bismarck", Ship::Type::BB, 29, 10, 13, "F20", this);
	shipList.push_back(bb);
	flagship = &shipList[0];
}

// Get flagship for read-only
const Ship& GermanPlayer::getFlagship() const {
	assert(flagship != nullptr);
	return *flagship;
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
				->checkShadow(ship, ship.getPosition(), 
					GameDirector::Phase::SHADOW);
		}
	}
}

// Do sea movement phase
void GermanPlayer::doSeaMovementPhase() {
	for (auto& ship: shipList) {
		if (!ship.isSunk()
			&& !ship.wasShadowed(0))
		{
			if (GameDirector::instance()->getTurnsElapsed() == 0) {
				ship.doBreakoutBonusMove();
			}
			else {
				ship.doMovement();
			}
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
			director->checkShadow(ship, zone, 
				GameDirector::Phase::SEARCH);
			anyFound = true;
		}
	}
	return anyFound;
}

// Do air attack phase
void GermanPlayer::doAirAttackPhase() {
	auto game = GameDirector::instance();
	for (auto& ship: shipList) {
		if (ship.wasLocated(0)) {
			game->checkAttackOn(ship, 
				GameDirector::Phase::AIR_ATTACK);
		}
	}
}

// Do naval combat phase
void GermanPlayer::doNavalCombatPhase() {
	auto game = GameDirector::instance();

	// Check for attacks by British on our ships
	for (auto& ship: shipList) {
		if (ship.wasLocated(0)) {
			game->checkAttackOn(ship, 
				GameDirector::Phase::NAVAL_COMBAT);
		}
	}
	
	// Check for attacks we can make on British ships
	for (auto zone: foundShipZones) {
		for (auto& ship: shipList) {
			if (ship.getPosition() == zone
				&& !ship.wasCombated(0)
				&& !ship.isSunk())
			{
				game->checkAttackBy(ship);
			}
		}
	}
}

// Do chance phase
//   See Basic Game Tables Card: Chance Table
//   While RAW says British player makes this roll (Rule 10.1),
//   it makes more sense for us with knowledge of ships on board.
void GermanPlayer::doChancePhase() {
	for (auto& ship: shipList) {
		int roll = diceRoll(2, 6);
		
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
			if (!game->wasConvoySunk(0)      // Rule 10.26
				&& !game->isVisibilityX())   // Errata in General 16/2
			{			
				checkConvoyResult(ship, roll);
			}
		}
		
		// Error check
		else {
			cerr << "Error: Unhandled roll in Chance Phase.\n";
		}
	}
}

// Call result of British HUFF-DUFF detection
void GermanPlayer::callHuffDuff(Ship& ship) {
	ship.noteDetected();
	cgame << "HUFF-DUFF: German ship near "
		<< SearchBoard::instance()
			->randSeaZone(ship.getPosition(), 1)
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
	if (!ship.isInNight()        // Rule 11.13
		&& !ship.isInFog()       // Rule 10.213
		&& pos.getRow() >= 'E'   // Rule 10.211
		&& pos.getCol() >= SearchBoard::instance()
			->getPatrolLimitForRow(pos.getRow()))
	{
		// Look up search strength
		char colLetter = getGeneralSearchColumn(pos);
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

// Get the applicable general search table row
//   As per rule 10.214.
char GermanPlayer::getGeneralSearchColumn(const GridCoordinate& zone) {
	auto board = SearchBoard::instance();
	
	// Case 'A': western edge, near patrol line limit
	if (zone.getCol() <= board->getPatrolLimitForRow(zone.getRow()) + 2) {
		return 'A';	
	}

	// Case 'C': eastern edge, near coast of Britain/Ireland
	else if (board->isNearZoneType(zone, 2, board->isBritishCoast)) {
		return 'C';		
	}

	// Case 'B': in-between, any other location
	else {
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
	cgame << "CONVOY SUNK:"
		<< " In zone " << ship.getPosition()
		<< " by " << ship.getName() << endl;
	GameDirector::instance()->msgSunkConvoy();
	ship.setLoseMoveTurn();   // Rule 10.25
	if (!ship.isReturnToBase()) {
		ship.clearOrders();
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
		if (ship.wasLocated(0)) {
			return true;
		}
	}
	return false;
}

// Resolve any search attempts
void GermanPlayer::resolveSearch() {
	for (auto& ship: shipList) {
		if (ship.wasLocated(0)) {
			auto game = GameDirector::instance();

			// Get search strength
			int strength = 1;
			if (ship.isOnPatrol()) {
				strength = ship.isInDay() ? 4 : 3;
			}
			
			// Do the search
			auto zone = ship.getPosition();
			if (game->isSearchable(zone, strength)) {
				if (game->searchBritishShips(zone)) {
					foundShipZones.push_back(zone);
					cgame << "German player locates ship(s) in " 
						<< zone << "\n";
				}
			}
		}
	}
}

// Convert a map zone to a strategic region
//   Splits map in 6 sections, mostly radiating from zone G15
GermanPlayer::MapRegion GermanPlayer::getRegion(
	const GridCoordinate& zone) const
{
	// Get zone components
	char row = zone.getRow();
	int col = zone.getCol();
	
	// North Sea (east of Britain; don't expect to go here)
	if (row >= 'H' && col >= row - 'H' + 18) { return NORTH_SEA; }
	
	// East Norwegian sea (area of first-turn breakout bonus)
	else if (row <= 'G' && col > 15) { return EAST_NORWEGIAN; }

	// West Norwegian sea (area above British northern patrol)
	else if (row <= 'G' && col >= 12 
		&& col > row - 'D' + 12) { return WEST_NORWEGIAN; }
	
	// Denmark Strait (foggy area north of Iceland)	
	else if (row <= 'C') { return DENMARK_STRAIT; }
	
	// West Atlantic (closer to Atlantic Convoy line)
	else if (col <= 'G' + 16 - row) { return WEST_ATLANTIC; }

	// East Atlantic (closer to African Convoy line)
	else { return EAST_ATLANTIC; }
}

// Get orders for a ship before its move
void GermanPlayer::getOrders(Ship& ship) {
	bool needsNewGoal = false;
	
	// Check rule for return-to-base if out of fuel
	if (!ship.getFuel()) {
		handleFuelEmpty(ship);
		if (ship.isReturnToBase()) {
			return;	
		}
	}

	// Redirect if we saw combat or were found patroling
	if (ship.wasCombated(1)
		|| (ship.wasLocated(1) && ship.isOnPatrol()))
	{
		ship.clearOrders();
		ship.orderMove(ship.randMoveInArea(3));
		needsNewGoal = true;
	}

	// Check other reasons for new goal
	if (!ship.hasOrders() 
		|| ship.getFirstOrder() == Ship::STOP)
	{
		ship.clearOrders();
		needsNewGoal = true;
	}

	// Set new strategic goal if needed
	if (needsNewGoal) {
		orderNewGoal(ship);
	}
}

// Set a new strategic goal for a ship
void GermanPlayer::orderNewGoal(Ship& ship) {

	// Gather data
	GameDirector* game = GameDirector::instance();
	SearchBoard* board = SearchBoard::instance();
	GridCoordinate position = ship.getPosition();
	MapRegion region = getRegion(position);
	int visibility = game->getVisibility();
	if (ship.isInNight()) {
		visibility += 2; // approximation
	}
	
	// On first turn, choose breakout bonus move
	if (game->getTurnsElapsed() == 0) {
		char row = dieRoll(100) <= 85 ? 
			'A' + rand(4): 'E' + rand(2);
		int col = dieRoll(100) <= 50 ?
			15 : 16 + rand(3);
		if (row == 'F' && col == 15) { // Avoid Faeroe
			ship.orderMove("G16");
			ship.orderMove(board->randSeaZone("H15", 1));
		} 
		else {
			ship.orderMove(GridCoordinate(row, col));
			ship.orderAction(Ship::STOP);
		}
	}

	// If in North Sea, move to Norway coast
	else if (region == NORTH_SEA) {

		// Safety valve for region we shouldn't ever go
		ship.orderMove(board->randSeaZone("D17", 1));
	}

	// Move away from Norway
	else if (region == EAST_NORWEGIAN) {

		// If on highest-risk zone, breakout ASAP
		if (position == GridCoordinate("G16")) {
			ship.orderMove(randAnyConvoyTarget(ship));
			ship.orderAction(Ship::PATROL);
		}

		// If located, move out ASAP
		else if (ship.wasLocated(1)) {
			ship.orderMove(board->randSeaZone("B13", 1));
		}

		// Break out in bad weather or late game
		else if (visibility > 6
			|| dieRoll(6) < game->getTurn() - 6)
		{
			ship.orderMove(GridCoordinate(position.getRow(), 14));
		}

		// Loiter near Norway
		else {
			ship.orderMove(getLoiterZone(ship));
			ship.orderAction(Ship::STOP);
		}
	}
	
	// Move through west Norwegian sea
	else if (region == WEST_NORWEGIAN) {
		
		// Maybe break south if conditions right
		if (visibility > 4
			&& position.getRow() >= 'C'
			&& dieRoll(6) <= 3)
		{
			ship.orderMove(board->randSeaZone("F13", 1));
		}
		
		// Otherwise go for Denmark Strait
		else {
			ship.orderMove(dieRoll(2) == 1 ? "A10" : "B11");
		}
	}
	
	// Move through Denmark Strait
	else if (region == DENMARK_STRAIT) {
		ship.orderMove("B7");
		int colOnRowC = 5 + rand(3);
		int colOnRowE = colOnRowC + rand(3);
		ship.orderMove(GridCoordinate('C', colOnRowC));
		ship.orderMove(GridCoordinate('E', colOnRowE));

		// Get past general search asap
		if (dieRoll(100) <= 50) {
			ship.orderMove(randAtlanticConvoyTarget());
		}
		else {
			ship.orderMove(board->randSeaZone("K6", 2));
			ship.orderMove(randAfricanConvoyTarget());
		}
		ship.orderAction(Ship::PATROL);
	}
	
	// Breakout accomplished, now search for convoys
	else if (region == EAST_ATLANTIC || region == WEST_ATLANTIC) {
		auto target = randAnyConvoyTarget(ship);
		ship.orderMove(target);
		ship.orderAction(Ship::PATROL);
	}
	
	// Error-handler
	else {
		cerr << "Error: Unhandled region in GermanPlayer\n";		
		assert(false);		
	}
}

// Get an adjacent zone for a ship loitering in a region
GridCoordinate GermanPlayer::getLoiterZone(const Ship& ship) const {
	GridCoordinate move;
	do {
		move = ship.randMoveInArea(1);
	} while (getRegion(move) != getRegion(ship.getPosition()));
	return move;
}

// Pick an appropriate convoy target after breakout
GridCoordinate GermanPlayer::randAnyConvoyTarget(Ship& ship) const {
	
	// On analysis, we usually want to make this evenly distributed, e.g.:
	// Initial breakout b/w Iceland and Britain (either route in line)
	// After being found or in combat (maximize difficulty guessing)
	// After a convoy sinking (same)
	// But note it takes 1-2 days to switch routes outside patrol line
	int pctAtlantic = 50;
	return dieRoll(100) <= pctAtlantic ?
		randAtlanticConvoyTarget() : randAfricanConvoyTarget();
}

// Randomize a convoy target near the Atlantic line
//   Around row H, on western edge past patrol line
GridCoordinate GermanPlayer::randAtlanticConvoyTarget() const {
	int col;
	char row = 'G' + rand(3);
	switch (row) {
		case 'G': col = dieRoll(4); break;
		case 'H': col = dieRoll(5); break;
		case 'I': col = dieRoll(5) + 1; break;
		default: cerr << "Error: Unhandled Atlantic convoy column.\n";
	}
	return GridCoordinate(row, col);
}

// Randomize a convoy target near the African line
//   Row P to Z, directly on convoy route
GridCoordinate GermanPlayer::randAfricanConvoyTarget() const {
	int inc = rand(11);
	char row = 'P' + inc;
	int col = 15 + (inc + 1) / 2;
	return GridCoordinate(row, col);
}

// Randomize a mid-Atlantic breakout target
//   When breaking out east of Iceland:
//   Target a zone mid-Atlantic beyond the general patrol line.
//   (1) Reduces distance beyond general search,
//   (2) Pulls path away from British coast
//   (3) Maneuvers near African convoy line
//   Zones L11 to Q16
GridCoordinate GermanPlayer::randMidAtlanticTarget() const {
	int inc = rand(6);
	int row = 'L' + inc;
	int col = 11 + inc;
	return GridCoordinate(row, col);
}

// Use optional rule for return-to-base when fuel empty (Rule 16.3)
void GermanPlayer::handleFuelEmpty(Ship& ship) {
	assert(!ship.getFuel());
	if (CmdArgs::instance()->useFuelExpenditure()) {
		if (!ship.isReturnToBase()) {
			clog << ship.getName() << " out of fuel (set RTB)\n";
			ship.setReturnToBase();
			ship.clearOrders();
			ship.orderMove(findNearestPort(ship));
			ship.orderAction(Ship::STOP); // Rule 16.6
		}
	}
}

// Find the nearest friendly port for a given ship
GridCoordinate GermanPlayer::findNearestPort(const Ship& ship) const {
	int minDistance = INT_MAX;
	auto nearestPort = GridCoordinate::NO_ZONE;
	auto portList = SearchBoard::instance()->getAllGermanPorts();
	for (auto port: portList) {
		int distance = port.distanceFrom(ship.getPosition());
		if (distance < minDistance) {
			nearestPort = port;
			minDistance = distance;
		}
	}
	assert(nearestPort != GridCoordinate::NO_ZONE);
	return nearestPort;	
}
