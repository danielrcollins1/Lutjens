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

	// Construct basic ships
	shipList.push_back(
		Ship("Bismarck", Ship::Type::BB, 29, 10, 13, "F20", this));
	shipList.push_back(			
		Ship("Prinz Eugen", Ship::Type::CA, 32, 4, 10, "F20", this));
	
	// Construct optional ships on command
	auto cmd = CmdArgs::instance();
	if (cmd->useOptScheer()) {
		shipList.push_back(
			Ship("Scheer", Ship::Type::PB, 26, 4, 13, "F20", this));
	}
	if (cmd->useOptTirpitz()) {
		shipList.push_back(
			Ship("Tirpitz", Ship::Type::BB, 29, 10, 13, "F20", this));
	}
	if (cmd->useOptScharnhorsts()) {
		shipList.push_back(
			Ship("Scharnhorst", Ship::Type::BC, 32, 7, 13, "P23", this));
		shipList.push_back(
			Ship("Gneisenau", Ship::Type::BC, 32, 7, 13, "P23", this));
	}
	
	// Record key data
	startNumShips = shipList.size();
	theBismarck = &shipList.front();
}

// Destructor
GermanPlayer::~GermanPlayer() {
	shipList.clear();
	taskForceList.clear();
}

// Get the Bismarck for special basic rules
const Ship& GermanPlayer::getBismarck() const {
	assert(theBismarck != nullptr);
	return *theBismarck;
}

// Get the number of ships we started with
int GermanPlayer::getStartNumShips() const {
	return startNumShips;
}

// Do unit availability phase
void GermanPlayer::doAvailabilityPhase() {
	foundShipZones.clear();
	for (auto& ship: shipList) {
		ship.doAvailability();
	}
}

// Do visibility phase
void GermanPlayer::doVisibilityPhase() {
	orderUnitsForTurn();	
}

// Organize task forces & list ordered units for one turn
void GermanPlayer::orderUnitsForTurn() {
	navalUnitList.clear();

	// Task force maintenance
	formTaskForces();
	cleanTaskForces();

	// Add active task forces
	for (auto& taffy: taskForceList) {
		if (!taffy.isEmpty()) {
			navalUnitList.push_back(&taffy);
		}
	}

	// Add solo ships
	for (auto& ship: shipList) {
		if (!ship.isInTaskForce()
			&& ship.isAfloat())
		{
			navalUnitList.push_back(&ship);
		}
	}
}

// Combine ships into task forces
void GermanPlayer::formTaskForces() {

	// Check all solo ships
	for (auto& seedShip: shipList) {
		if (!seedShip.isInTaskForce()
			&& seedShip.isAfloat())
		{
			// Wait if patrolling for convoys
			if (seedShip.isOnPatrol()
				&& !seedShip.wasLocated(1)
				&& !seedShip.wasCombated(1)
				&& !seedShip.wasConvoySunk(1)
				&& !seedShip.wasConvoySunk(2))
			{
				continue;	
			}

			// Gather up ships in zone
			vector<Ship*> shipsToJoin;
			auto zone = seedShip.getPosition();
			for (auto& ship: shipList) {
				if (ship.getPosition() == zone
					&& ship.getMaxSpeedClass() >= 3
					&& !ship.isInTaskForce()
					&& ship.isAfloat())
				{
					shipsToJoin.push_back(&ship);
				}
			}
			
			// Create a new task force
			if (shipsToJoin.size() > 1) {
				int newId = getNextTaskForceId();
				taskForceList.push_back(TaskForce(newId));
				for (auto& ship: shipsToJoin) {
					ship->clearOrders();			
					taskForceList.back().attach(ship);
				}
			}
		}
	}
}

// Clean, dissolve, erase task forces
void GermanPlayer::cleanTaskForces() {

	// Clean up task forces
	for (auto& taffy: taskForceList) {
		cleanTaskForce(taffy);
		if (!taffy.isEmpty()) {
			auto flagship = taffy.getFlagship();
	
			// End solo task force
			if (taffy.getSize() == 1) {
				taffy.dissolve();
			}
	
			// Breakup after breakout
			else if (flagship->hasOrders()
				&& flagship->getFirstOrder() == Ship::PATROL
				&& !taffy.wasConvoySunk(1)
				&& !taffy.wasConvoySunk(2))
			{
				taffy.orderFollowers(Ship::PATROL);
				taffy.dissolve();
			}
		}
	}

	// Delete any empty task forces
	auto it = taskForceList.begin();
	while (it != taskForceList.end()) {
		if (it->isEmpty()) {
			it = taskForceList.erase(it);	
			continue;
		}
		it++;
	}
}

// Clean task force as needed
//   Sweep out sunk, slow, low-fuel ships
void GermanPlayer::cleanTaskForce(TaskForce& taffy) {
	for (int i = 0; i < taffy.getSize(); i++) {
		Ship* ship = taffy.getShip(i);
		if (!ship->isAfloat()
			|| ship->getFuel() < 2
			|| ship->getMaxSpeedClass() < 3)
		{
			taffy.detach(ship);
			i--;
		}
	}
}

// Get the number to use for the next convoy
int GermanPlayer::getNextTaskForceId() {
	int number = 1;
	while (getTaskForceById(number)) {
		number++;	
	}
	return number;
}

// Get a task force by ID number
TaskForce* GermanPlayer::getTaskForceById(int id) {
	for (auto& taffy: taskForceList) {
		if (taffy.getId() == id) {
			return &taffy;
		}
	}
	return nullptr;	
}

// Do shadow phase
void GermanPlayer::doShadowPhase() {
	for (auto& unit: navalUnitList) {
		if (unit->wasLocated(1)) {
			GameDirector::instance()
				->checkShadow(*unit, unit->getPosition(), 
					GameDirector::Phase::SHADOW);
		}
	}
}

// Do ship movement phase
void GermanPlayer::doShipMovementPhase() {

	// Move task forces & solo ships
	for (auto& unit: navalUnitList) {
		if (!unit->wasShadowed(0)) {
			unit->doMovementTurn();	
		}
	}
	
	// Log all ship statuses
	for (auto& ship: shipList) {
		if (ship.isAfloat()) {
			clog << ship << endl;
		}
	}
	
	// Tests for bug regressions
	testStoppedUnits();	
	testMoveAfterConvoySunk();
}

// Test for units losing movement turns
//   (Prior bug: Lose-turn field kept after TF dissolved)
void GermanPlayer::testStoppedUnits() {
	for (auto& unit: navalUnitList) {
		if (unit->isAfloat()
			&& !unit->getSpeedThisTurn()
			&& !unit->isInPort()
			&& !unit->isOnPatrol()
			&& !unit->isReturnToBase()
			&& !unit->wasConvoySunk(1)
			&& !(unit->getFlagship()->hasOrders()
				&& unit->getFlagship()->getFirstOrder() == Ship::STOP))
		{
			cerr << "Error: Stopped unit: " << unit->getFullDesc() << "\n";
		}
	}
}

// Test for units moving after convoy sunk
//   (Prior bug: TF ignored follower ships that sank convoys)
void GermanPlayer::testMoveAfterConvoySunk() {
	for (auto& ship: shipList) {
		if (ship.wasConvoySunk(1)
			&& ship.getSpeedThisTurn() > 0)
		{
			cerr << "Error: Ship moved after convoy sunk: " 
				<< ship.getFullDesc() << "\n";
		}
	}
}

// Check for search by British player
bool GermanPlayer::checkSearch(const GridCoordinate& zone) {
	bool anyFound = false;
	auto game = GameDirector::instance();
	for (auto& unit: navalUnitList) {
 		if (unit->getPosition() == zone) 
		{
			cgame << unit->getTypeDesc() 
				<< " found in " << zone << endl;
			unit->setLocated();
			anyFound = true;
		}
		else if (unit->movedThrough(zone)
			&& !game->isStartTurn())
		{
			cgame << unit->getTypeDesc()
				<< " seen moving through " << zone << endl;
			game->checkShadow(*unit, zone, 
				GameDirector::Phase::SEARCH);
			anyFound = true;
		}
	}
	return anyFound;
}

// Do air attack phase
void GermanPlayer::doAirAttackPhase() {
	auto game = GameDirector::instance();
	for (auto& unit: navalUnitList) {
		if (unit->wasLocated(0)     // Rule 9.11
			&& !unit->isInPort())   // Rule 9.13
		{
			game->checkAttackOn(*unit, 
				GameDirector::Phase::AIR_ATTACK);
		}
	}
}

// Do naval combat phase
void GermanPlayer::doNavalCombatPhase() {
	auto game = GameDirector::instance();

	// Check for attacks by British on our units
	for (auto& unit: navalUnitList) {
		if (unit->wasLocated(0)     // Rule 9.23
			&& !unit->isInPort())   // Rule 12.7
		{		
			game->checkAttackOn(*unit, 
				GameDirector::Phase::NAVAL_COMBAT);
		}
	}
	
	// Check for attacks we can make on British ships
	for (auto& zone: foundShipZones) {
		for (auto& unit: navalUnitList) {
			if (unit->getPosition() == zone
				&& unit->isAfloat()
				&& !unit->wasCombated(0))
			{
				// Only attack with battleships
				if (unit->getFlagship()->getGeneralType() 
					== Ship::BATTLESHIP)
				{
					game->checkAttackBy(*unit);
				}
			}
		}
	}
}

// Do chance phase
//   See Basic Game Tables Card: Chance Table
//   While RAW says British player makes this roll (Rule 10.1),
//   it makes more sense for us with knowledge of ships on board.
void GermanPlayer::doChancePhase() {
	for (auto& unit: navalUnitList) {
		if (unit->isOnBoard()) {
			int roll = diceRoll(2, 6);
			
			// Huff-duff result
			if (roll == 2) {
				callHuffDuff(unit);
			}
		
			// General Search results
			else if (roll <= 9) {
				checkGeneralSearch(unit, roll);
			}
			
			// Convoy results
			else if (roll <= 12) {
				auto game = GameDirector::instance();
				if (!game->wasConvoySunk(0)      // Rule 10.26
					&& !game->isVisibilityX())   // Errata in General 16/2
				{			
					checkConvoyResult(unit, roll);
				}
			}
			
			// Error check
			else {
				cerr << "Error: Unhandled roll in Chance Phase.\n";
			}
		}
	}
}

// Call result of British HUFF-DUFF detection
void GermanPlayer::callHuffDuff(NavalUnit* unit) {
	unit->setDetected();
	cgame << "HUFF-DUFF: German ship near "
		<< SearchBoard::instance()
			->randSeaZone(unit->getPosition(), 1)
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
void GermanPlayer::checkGeneralSearch(NavalUnit* unit, int roll) {
	assert(3 <= roll && roll <= 9);
	auto pos = unit->getPosition();
	auto board = SearchBoard::instance();
	
	// Check if general search possible
	if (board->isInsidePatrolLine(pos)     // Rule 10.211
		&& !unit->isInFog()                // Rule 10.213
		&& !unit->isInNight())             // Rule 11.13
	{
		// Look up search strength
		char colLetter = getGeneralSearchColumn(pos);
		int gsRowIdx = roll - 3;
		int gsColIdx = colLetter - 'A';
		int searchStrength = GS_VALUES[gsRowIdx][gsColIdx];
		
		// Announce result
		int visibility = GameDirector::instance()->getVisibility();
		if (visibility <= searchStrength) {
			unit->setDetected();
			cgame << "General Search found " 
				 << unit->getNameDesc() << " in " << pos << "\n";
		}
	}
}

// Get the applicable general search table row
//   As per rule 10.214.
char GermanPlayer::getGeneralSearchColumn(const GridCoordinate& zone) {
	auto board = SearchBoard::instance();
	
	// Case 'A': western edge, near patrol line limit
	if (board->isNearZoneType(zone, 2, board->isBritishPatrolLine)) {
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
void GermanPlayer::checkConvoyResult(NavalUnit* unit, int roll) {
	assert(10 <= roll && roll <= 12);
	auto board = SearchBoard::instance();
	auto pos = unit->getPosition();
	if (!unit->wasLocated(0)     // Rule 10.231
		&& !unit->isInNight())   // Rule 11.13
	{
		switch (roll) {
	
			// On convoy route
			case 10:
				if (board->isConvoyRoute(pos)) {
					destroyConvoy(unit);				
				}
				break;
	
			// On patrol and within two
			case 11:
				if (unit->isOnPatrol()
					&& board->isNearZoneType(pos, 2, 
						&SearchBoard::isConvoyRoute))
				{
					destroyConvoy(unit);				
				}
				break;
				
			// One zone from convoy route
			case 12:
				if (board->isNearZoneType(pos, 1, 
					&SearchBoard::isConvoyRoute))
				{
					destroyConvoy(unit);
				}
				break;
		}
	}
}

// Score destruction of a convoy
//   And re-route to new destination
void GermanPlayer::destroyConvoy(NavalUnit* unit) {
	cgame << "CONVOY SUNK:"
		<< " In zone " << unit->getPosition()
		<< " by " << unit->getNameDesc() << endl;
	GameDirector::instance()->msgSunkConvoy();
	unit->setConvoySunk();
}

// Print all of our ships (e.g., for end game)
void GermanPlayer::printAllShips() const {
	for (auto& ship: shipList) {
		cgame << ship << endl;
	}
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
//   Only search with ships that were located by enemy
void GermanPlayer::resolveSearch() {
	auto zoneList = getShipZones();
	for (auto& zone: zoneList) {

		// Add up search strength
		int strength = 0;
		for (auto& unit: navalUnitList) {
			if (unit->getPosition() == zone
				&& unit->wasLocated(0)) 
			{
				strength += unit->getSearchStrength();
			}
		}
			
		// Do the search
		auto game = GameDirector::instance();
		if (game->isSearchable(zone, strength)) {
			if (game->searchBritishShips(zone)) {
				foundShipZones.insert(zone);
				cgame << "German player locates ship(s) in " 
					<< zone << "\n";
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

	// Off-map (no-zone coordinate)
	if (zone == GridCoordinate::NO_ZONE) { return OFF_MAP; }
	
	// North Sea (east of Britain; don't expect to go here)
	else if (row >= 'H' && col >= row - 'H' + 18) { return NORTH_SEA; }
	
	// East Norwegian sea (area of first-turn breakout bonus)
	else if (row <= 'G' && col > 15) { return EAST_NORWEGIAN; }

	// West Norwegian sea (area above British northern patrol)
	else if (row <= 'G' && col >= 12 
		&& col > row - 'D' + 12) { return WEST_NORWEGIAN; }
	
	// Denmark Strait (foggy area north of Iceland)	
	else if (row <= 'C') { return DENMARK_STRAIT; }

	// Azores (lower left of map)
	else if (col <= row - 'L' + 3) { return AZORES; }
	
	// West Atlantic (closer to Atlantic Convoy line)
	else if (col <= 'G' + 16 - row) { return WEST_ATLANTIC; }

	// Bay of Biscay (gulf between France & Spain)
	else if (isInInterval('P', row, 'T')
		&& col >= row - 'P' + 19) { return BAY_OF_BISCAY; }

	// East Atlantic (closer to African Convoy line)
	else { return EAST_ATLANTIC; }
}

// Get orders for a ship before its move
void GermanPlayer::getOrders(Ship& ship) {
	bool needsNewGoal = false;
	int lastTurn = GameDirector::instance()->getFinishTurn();

	// Abort if off-board
	if (ship.getPosition() == GridCoordinate::NO_ZONE) {
		ship.orderAction(Ship::STOP);
		return;
	}
	
	// Check rule for return-to-base if out of fuel
	if (!ship.getFuel()) {
		handleFuelEmpty(ship);
		if (ship.isReturnToBase()) {
			return;
		}
	}

	// If we can't reach goal, try row Z (Rule 51.6)
	if (ship.routeETA() > lastTurn
		&& ship.rowZ_ETA() < lastTurn)
	{
		ship.clearOrders();
		ship.orderMove(randCloseRowZ(ship));
		return;	
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
		|| ship.getFirstOrder() == Ship::STOP
		|| ship.wasConvoySunk(1))
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

	// At game start, choose breakout bonus move
	if (game->isStartTurn()) {

		// Start at Bergen
		if (position == GridCoordinate("F20")) {
			
			// Note: rows A-D are above general patrol area (Rule 10.211),
			// while rows E-F in reach are mostly 2 zones from Britain
			// (so, worst chance column for us: 10/36 found start turn)
			// We take a small risk of that to widen search area
			char row = dieRoll(100) <= 85 ? 'A' + rand(4): 'E' + rand(2);
			int col = dieRoll(100) <= 50 ? 15 : 16 + rand(3);
			if (row == 'F' && col == 15) { // Avoid Faeroe
				ship.orderMove("G16");
				ship.orderMove(board->randSeaZone("H15", 1));
			} 
			else {
				ship.orderMove(GridCoordinate(row, col));
			}
		}
		
		// Start at Brest
		else if (position == GridCoordinate("P23")) {

			// Here, our entire breakout span is within patrol area.
			// If we go max speed 5 to row P or below, we get
			// 2 zones from patrol line (so, best chance column for us)
			// But most anything else above row R is 2 zones from Britain.
			// (difference is 0/36 vs. 10/36 to be found 1st turn)
			// However, British player can picket the entire
			// max breakout line on start turn and beyond.
			// So we use more of that area.
			if (dieRoll(100) <= 50) { // Fast-track
				char row = 'N' + rand(8);
				int col = (row <= 'P') ? 18 : row - 'P' + 18;
				ship.orderMove(GridCoordinate(row, col));
				if (row < 'P') {
					ship.orderMove(board->randSeaZone("N15", 1));
				}
				ship.orderMove(randAfricanConvoyTarget());
				ship.orderAction(Ship::PATROL);
			}
			else { // Slow-roll near France
				char row = 'P' + rand(5);
				int col = row - 'P' + 18 + dieRoll(4);
				ship.orderMove(GridCoordinate(row, col));
			}
		}

		// Error handler
		else {
			cerr << "Error: Unknown starting position\n";
			assert(false);			
		}
	}

	// If on row Z, exit from map (Rule 51.6)
	else if (position.getRow() == 'Z') {
		ship.orderMove(GridCoordinate::NO_ZONE);
	}

	// If in North Sea, move to Norway coast
	else if (region == NORTH_SEA) {
		ship.orderMove(board->randSeaZone("D17", 1));
	}

	// Move away from Norway
	else if (region == EAST_NORWEGIAN) {

		// If on highest-risk zone, breakout ASAP
		if (position == GridCoordinate("G16")) {
			ship.orderMove(randConvoyTarget(50));
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
			ship.orderMove(randLoiterZone(ship));
			ship.orderAction(Ship::STOP); // if current zone
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
	}
	
	// Breakout accomplished, now search for convoys
	else if (region == EAST_ATLANTIC || region == WEST_ATLANTIC) {
		bool isInWest = (region == WEST_ATLANTIC);
		
		// Coming out of Denmark Strait to Africa,
		// be sure to transit outside patrol line asap
		if (position.getCol() < 10
			&& (position.getRow() < 'E' 
				|| board->isInsidePatrolLine(position)))
		{
			auto target = randConvoyTarget(66);
			if (getRegion(target) == EAST_ATLANTIC) {
				ship.orderMove(randDenmarkStraitToAfricaTransit(ship));
			}
			ship.orderMove(target);
			ship.orderAction(Ship::PATROL);
		}

		// If inside patrol line, want to get out/prefer closer line
		else if (board->isInsidePatrolLine(position)) {
			ship.orderMove(randConvoyTargetWeightNearby(ship));
			ship.orderAction(Ship::PATROL);
		}

		// Expect we just had combat or convoy sunk.
		// Very small chance we want to stay in area
		else if (dieRoll(6) <= 1) {
			ship.orderMove(randConvoyTarget(isInWest ? 100: 0));
			ship.orderAction(Ship::PATROL);
		}
		
		// Otherwise, 50/50 if we should go to other line or Azores
		else {
			if (dieRoll(6) <= 3) {
				ship.orderMove(randConvoyTarget(isInWest ? 0 : 100));
				ship.orderAction(Ship::PATROL);
			}
			else {
				ship.orderMove(randAzoresZone());
			}
		}
	}
	
	// Use Azores to hide out, e.g., after convoy sinking
	// (We rarely get this far, and never time to get out)
	else if (region == AZORES) {

		// Move somewhere if we're found or combated
		if (ship.wasLocated(1) || ship.wasCombated(1)) {
			if (dieRoll(6) <= 2) {
				ship.orderMove(randAzoresZone());
				ship.orderAction(Ship::STOP); // if current zone
			}
			else {
				ship.orderMove(randConvoyTarget(50));
				ship.orderAction(Ship::PATROL);
			}
		}

		// Small chance to return convoy hunting on our own
		else if (dieRoll(6) <= 1) {
			ship.orderMove(randConvoyTarget(50));
			ship.orderAction(Ship::PATROL);
		}
		
		// Otherwise loiter in current region
		else {
			ship.orderMove(randLoiterZone(ship));
			ship.orderAction(Ship::STOP); // if current zone
		}
	}

	// Bay of Biscay is starting region for Brest-based ships
	// (i.e., intermediate new-ship scenarios)
	else if (region == BAY_OF_BISCAY) {

		// Get out ASAP (don't loiter inside patrol line)
		ship.orderMove(randAfricanConvoyTarget());
		ship.orderAction(Ship::PATROL);
	}
	
	// Error-handler
	else {
		cerr << "Error: Unhandled region in GermanPlayer\n";		
		assert(false);		
	}
}

// Get an adjacent zone for a ship loitering in a region
GridCoordinate GermanPlayer::randLoiterZone(const Ship& ship) const {
	GridCoordinate move = GridCoordinate::NO_ZONE;
	while (getRegion(move) != getRegion(ship.getPosition())) {
		move = ship.randMoveInArea(1);
	};
	return move;
}

// Pick a convoy target from between the two lines
GridCoordinate GermanPlayer::randConvoyTarget(int pctAtlantic) const {
	return dieRoll(100) <= pctAtlantic ?
		randAtlanticConvoyTarget() : randAfricanConvoyTarget();
}

// Randomize a convoy target near the Atlantic line
//   Weight distance as chance to find convoy on patrol (2:3:5:3:2)
//   Around row H, on western edge past patrol line
GridCoordinate GermanPlayer::randAtlanticConvoyTarget() const {
	auto board = SearchBoard::instance();
	GridCoordinate zone = GridCoordinate::NO_ZONE;
	while (!board->isSeaZone(zone)
		|| board->isInsidePatrolLine(zone))
	{
		char row = 'H' + randWeightedConvoyDistance();
		int col = dieRoll(7);
		zone = GridCoordinate(row, col);		
	}
	return zone;
}

// Randomize a convoy target near the African line
//   Weight distance as chance to find convoy on patrol (2:3:5:3:2)
//   Row P to Y, near convoy beyond patrol line
GridCoordinate GermanPlayer::randAfricanConvoyTarget() const {
	auto board = SearchBoard::instance();
	GridCoordinate zone = GridCoordinate::NO_ZONE;
	while (!board->isSeaZone(zone)
		|| board->isInsidePatrolLine(zone))
	{
		int inc = rand(10);
		char row = 'P' + inc;
		int col = 15 + (inc + 1) / 2 + randWeightedConvoyDistance();
		zone = GridCoordinate(row, col);		
	}
	return zone;
}

// Get a desired distance from a convoy route
//   Weighted by chance to find convoy on patrol (2:3:5:3:2)
//   As per Chance Table convoy results (out of 36 options)
int GermanPlayer::randWeightedConvoyDistance() const {
	switch (dieRoll(15)) {
		case 1: case 2: return -2;
		case 3: case 4: case 5: return -1;
		default: return 0;
		case 11: case 12: case 13: return +1;
		case 14: case 15: return +2;
	}
}

// Use optional rule for return-to-base when fuel empty (Rule 16.3)
void GermanPlayer::handleFuelEmpty(Ship& ship) {
	assert(!ship.getFuel());
	if (CmdArgs::instance()->useOptFuelExpenditure()) {

		// First fuel-empty notice
		if (!ship.isReturnToBase()) {
			clog << ship.getName() << " out of fuel (set RTB)\n";
			ship.setReturnToBase();
			ship.clearOrders();
		}
		
		// Order to base
		if (!ship.hasOrders()) {
			if (ship.isInPort()) {
				ship.orderAction(Ship::STOP); // Rule 16.6
			}
			else {
				ship.orderMove(findNearestPort(ship));
			}
		}
	}
}

// Find the nearest friendly port for a given ship
GridCoordinate GermanPlayer::findNearestPort(const Ship& ship) const {
	int minDistance = INT_MAX;
	auto nearestPort = GridCoordinate::NO_ZONE;
	auto portList = SearchBoard::instance()->getAllGermanPorts();
	for (auto& port: portList) {
		int distance = port.distanceFrom(ship.getPosition());
		if (distance < minDistance) {
			nearestPort = port;
			minDistance = distance;
		}
	}
	assert(nearestPort != GridCoordinate::NO_ZONE);
	return nearestPort;	
}

// Get a transit point when moving from Denmark Strait to Africa Convoy line
//   Find closest zone past patrol line, then vary a bit
GridCoordinate GermanPlayer::randDenmarkStraitToAfricaTransit(
	const Ship& ship) const
{
	assert(ship.getPosition().getCol() < 10);
	int startCol = ship.getPosition().getCol();
	GridCoordinate bestZone('C' + startCol, startCol);
	GridCoordinate targetZone = GridCoordinate::NO_ZONE;
	while (!ship.isAccessible(targetZone)) {

		// Vary northwest along patrol up to 3 spaces
		int inc = rand(4);
		char row = bestZone.getRow() - inc;
		int col = bestZone.getCol() - inc;

		// Vary southwest past patrol line up to 5 spaces
		// (This includes where Bismarck shook Sheffield)
		row += rand(6);
		targetZone = GridCoordinate(row, col);
	}
	assert(!SearchBoard::instance()->isInsidePatrolLine(targetZone));
	return targetZone;
}

// Get a random point in the Azores region to hide
GridCoordinate GermanPlayer::randAzoresZone() const {
	auto target = GridCoordinate::NO_ZONE;
	auto board = SearchBoard::instance();
	while (!board->isSeaZone(target)) {
		char row = 'L' + rand(13);
		int col = 3 + row - 'L' - rand(8);
		target = GridCoordinate(row, col);
	}
	assert(getRegion(target) == AZORES);	
	return target;	
}

// Get a random conoy target, prefering the closer line
//   For use getting out of patrol line area asap
GridCoordinate GermanPlayer::randConvoyTargetWeightNearby(
	const Ship& ship) const
{
	auto position = ship.getPosition();
	int distAtlantic = position.distanceFrom("H5");
	int distAfrican = position.distanceFrom("P15");
	int roll = dieRoll(distAtlantic + distAfrican);
	return roll <= distAtlantic ? // small chance to avoid closer line
		randAfricanConvoyTarget() : randAtlanticConvoyTarget();
}

// Get the set of zones where we have ships
set<GridCoordinate> GermanPlayer::getShipZones() const {
	set<GridCoordinate> shipZones;
	for (auto& ship: shipList) {
		auto zone = ship.getPosition();
		if (ship.isAfloat()
			&& zone != GridCoordinate::NO_ZONE)
		{
			shipZones.insert(zone);
		}
	}
	return shipZones;	
}

// Get a minimal-distance zone on row Z
GridCoordinate GermanPlayer::randCloseRowZ(const Ship& ship) const {
	auto pos = ship.getPosition();
	int minCol = max(pos.getCol(), 10);
	int maxCol = min(pos.getCol() + 'Z' - pos.getRow(), 26);
	int width = maxCol - minCol + 1;
	assert(width >= 1);
	return GridCoordinate('Z', minCol + rand(width));
}
