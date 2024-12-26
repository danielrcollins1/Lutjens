#include "Ship.h"
#include "Utils.h"
#include "GameDirector.h"
#include <cassert>
#include <algorithm>
using namespace std;

// Ship type abbreviations
const string Ship::typeAbbr[NUM_TYPES]
	= {"BB", "BC", "PB", "CA", "CL", "CV", "DD", "SS"};

// Ship type full names
const string Ship::typeName[NUM_TYPES]
	= {"Battleship", "Battlecruiser", "Pocket Battleship", 
		"Heavy Cruiser", "Light Cruiser", "Aircraft Carrier", 
		"Destroyer", "Submarine"};

// Constructor
Ship::Ship(string name, Type type, 
	int evasion, int midships, int fuel) 
{
	this->name = name;
	this->type = type;
	this->evasionMax = evasion;
	this->midshipsMax = midships;
	this->fuelMax = fuel;
	position = GridCoordinate::NO_ZONE;
	onPatrol = false;
	tookMoveTurn = false;
	loseMoveTurn = false;
	fuelLost = 0;
	midshipsLost = 0;
	evasionLostTemp = 0;
	evasionLostPerm = 0;
	timesDetected = 0;
}

// Get the name
string Ship::getName() const {
	return name;	
}

// Get the full name of this type
string Ship::getTypeName() const {
	return typeName[type];	
}

// Get a descriptor of type & evasion (e.g., shadow prompt)
string Ship::getTypeAndEvasion() const {
	return getTypeName() 
		+ " (evrtg " + to_string(getEvasion()) + ")";
}

// Get a short descriptor (e.g., naval combat prompt)
string Ship::getShortDesc() const {
	return name
		+ " (" + typeAbbr[type]
		+ ", evrtg " + to_string(getEvasion())
		+ ", mships " + to_string(getMidships())
		+ ")";
}

// Get a long descriptor (e.g., logging purposes)
string Ship::getLongDesc() const {
	return name
		+ " (" + typeAbbr[type]
		+ ", evrtg " + to_string(getEvasion())
		+ ", mships " + to_string(getMidships())
		+ ", fuel " + to_string(getFuel())
		+ ", pos " + position.toString()
		+ (onPatrol ? ", patrol" : "")
		+ ")";
}

// Get the current evasion
int Ship::getEvasion() const {
	return max(0, evasionMax - evasionLostTemp - evasionLostPerm);
}

// Get the current midships
int Ship::getMidships() const {
	return max(0, midshipsMax - midshipsLost);
}

// Get the current fuel
int Ship::getFuel() const {
	return max(0, fuelMax - fuelLost);	
}

// Set starting position
void Ship::setPosition(const GridCoordinate& zone) {
	assert(moveHistory.empty());
	position = zone;
}

// Get current position
GridCoordinate Ship::getPosition() const {
	return position;	
}

// Are we on patrol?
bool Ship::isOnPatrol() const {
	return onPatrol;	
}

// Do setup in first phase of turn
void Ship::doAvailability() {
	tookMoveTurn = false;
	shadowedHistory.push_back(false);
	locatedHistory.push_back(false);
	combatHistory.push_back(false);
}

// Clear all waypoints
void Ship::clearWaypoints() {
	waypoints.clear();	
}

// Add a waypoint
//   But reject if we're already there
void Ship::addWaypoint(const GridCoordinate& coord) {
	if (coord != position) {
		waypoints.push_back(coord);
	}
}

// Do we have any waypoints?
bool Ship::hasWaypoints() const {
	return !waypoints.empty();
}

// How many board spaces can we move this turn?
//   See Basic Game Tables Card: Movement on Search Board
int Ship::maxSpeed() const {
	int evasion = getEvasion();
	if (evasion <= 6) {
		return 0;
	}
	else if (evasion <= 15 || getFuel() <= 0) {
		int turn = GameDirector::instance()->getTurn();
		return turn % 2 ? 0 : 1; // speed 0.5
	}
	else if (evasion <= 24) {
		return 1;		
	}
	else if (evasion <= 34) {
		int lastTurnSpeed = moveHistory.empty() ? 
			0 : moveHistory.back().size();
		return lastTurnSpeed > 1 ? 1 : 2; // speed 1.5
	}
	else {
		// No ship in published game has a speed this high.
		// At this point, search board speed would be full 2/turn.
		// A few destroyers/cruisers in WWII had speeds up to 45 knots.
		cerr << "Error: Ship evasion above game allowances.\n";
		return 2;		
	}
}

// Cap speed in early phase of game when needed.
int Ship::startGameSpeedCap() {
	auto game = GameDirector::instance();
	int turn = game->getTurn();
	int visibility = game->getVisibility();

	// Avoid standard air picket when visible
	if (position.getRow() < 'F'
		&& position.getCol() > 15
		&& 4 < turn && turn < 9
		&& rollDie(100) <= 93)
	{
		if ((isInDay() && visibility <= 6)
			|| visibility <= 3)
		{
			// Make a random move east of picket
			GridCoordinate randMove;
			do {
				randMove = SearchBoard::instance()
					->randSeaWithinOne(position);
			} while (!isInInterval(16, randMove.getCol(), 18));
			if (randMove == position) {
				return 0;	
			}
			else {
				waypoints.insert(waypoints.begin(), randMove);
				return 1;
			}
		}
	}
	return 2;	
}

// Check if we reached a waypoint
void Ship::checkForWaypoint() {
	while (!waypoints.empty() 
		&& position == waypoints[0])
	{
		waypoints.erase(waypoints.begin());
	}
}

// Get one step towards destination
GridCoordinate Ship::getNextZone() const {
	if (!waypoints.empty()) {
		GridCoordinate dest = waypoints[0];
		int dist = position.distanceFrom(dest);
		vector<GridCoordinate> adjacent = position.getAdjacent();
		vector<GridCoordinate> options;
		for (auto zone: adjacent) {
			if (isAccessible(zone) 
				&& zone.distanceFrom(dest) < dist) 
			{
				options.push_back(zone);
			}
		}
		return randomElem(options);
	}
	return position;
}

// Do movement for turn
void Ship::doMovement() {
	auto board = SearchBoard::instance();
	vector<GridCoordinate> moves;

	// Do movement only once per turn (phase 3 or 5)
	if (tookMoveTurn) {
		return;	
	}
	tookMoveTurn = true;

	// Do breakout bonus on first turn
	if (GameDirector::instance()->getTurn() 
		== GameDirector::START_TURN)
	{
		doBreakoutBonusMove();
		//cout << *this << endl;
		return;		
	}

	// Set to patrol if no waypoints
	onPatrol = waypoints.empty()
		&& !board->isGermanPort(position);

	// Select speed to move
	int speed = maxSpeed();
	if (getFuel() == 1) {
		speed = min(1, speed);	
	}
	if (onPatrol || loseMoveTurn) {
		speed = 0;
		loseMoveTurn = false;
	}
	speed = min(speed, startGameSpeedCap());
	assert(speed <= 2);
	
	// Try to perform movement
	for (int step = 0; step < speed; step++) {
		auto next = getNextZone();
		if (next != position) {
			position = next;
			moves.push_back(position);
			checkForWaypoint();
			if (isInPort()) {
				clog << name << " entered port at " 
					<< position << "\n";
			}
			//cout << *this << endl;
		}
	}

	// Account for fuel, history
	if (moves.size() == 2) {
		fuelLost++;	
	}
	moveHistory.push_back(moves);
	checkEvasionRepair();
}

// Do German ship first-turn breakout bonus move (Rule 5.28)
//   We use the first waypoint as our location at end of move
//   Waive most other rules restrictions here
void Ship::doBreakoutBonusMove() {

	// Check first turn only
	assert(GameDirector::instance()->getTurn() 
		== GameDirector::START_TURN);
	assert(!waypoints.empty());
	auto goal = waypoints[0];
	int distance = position.distanceFrom(goal);

	// Set position & cycle waypoint
	assert(distance <= 5);
	position = goal;
	checkForWaypoint();

	// Add to move history
	vector<GridCoordinate> move;
	move.push_back(position);
	moveHistory.push_back(move);

	// Charge for fuel
	switch (distance) {
		case 5: fuelLost += 2; break;
		case 4: fuelLost += 1; break;
		default: break; // no loss
	}
}

// Are we in port?
bool Ship::isInPort() const {
	auto board = SearchBoard::instance();
	return board->isGermanPort(position);
}

// Did we enter port last turn?
bool Ship::enteredPort() const {
	return isInPort()
		&& !moveHistory.empty()
		&& !moveHistory.back().empty();
}

// Is this zone accessible to German ships?
//   Note coastal zones allowed by errata (vs. Terrain Effects Chart)
bool Ship::isAccessible(const GridCoordinate& zone) const {
	auto board = SearchBoard::instance();
	return board->isSeaZone(zone)           // Rule 5.17
		&& !board->isIrishSea(zone)         // Rule 5.18
		&& !board->isBritishPort(zone);     // Rule 5.18
}

// Did we move into/through a given zone this turn?
bool Ship::movedThrough(const GridCoordinate& zone) const {
	assert(tookMoveTurn);
	return hasElem(moveHistory.back(), zone);
}

// Are we sunk?
bool Ship::isSunk() const {
	return getMidships() <= 0;	
}

// Are we in the day?
bool Ship::isInDay() const {
	return !isInNight();	
}

// Are we in the night?
bool Ship::isInNight() const {
	return GameDirector::instance()->isInNight(position);	
}

// Are we in fog?
bool Ship::isInFog() const {
	return GameDirector::instance()->isInFog(position);
}

// Set if we must lose a move turn
void Ship::setLoseMoveTurn() {
	loseMoveTurn = true;
}

// Note that we have been detected by any means:
//   Search, shadow, general search, or HUFF-DUFF,
//   but not reveal for convoy attack.
//   Used for statistical models (not part of game).
void Ship::noteDetected() {
	timesDetected++;
}

// Check how many times we were detected by any means
int Ship::getTimesDetected() const {
	return timesDetected;	
}

// Note that we have been located by search/shadow
void Ship::setLocated() {
	locatedHistory.back() = true;
	noteDetected();
}

// Note that we have been shadowed
void Ship::setShadowed() {
	shadowedHistory.back() = true;
	setLocated();
}

// Note that we have entered naval combat
void Ship::setInCombat() {
	combatHistory.back() = true;	
}

// Check if we were located by search/shadow on a given turn
bool Ship::wasLocated(int turnsAgo) const {
	assert(0 <= turnsAgo 
		&& turnsAgo < (int) locatedHistory.size());
	return locatedHistory.rbegin()[turnsAgo];
}

// Check if we were shadowed on a given turn
bool Ship::wasShadowed(int turnsAgo) const {
	assert(0 <= turnsAgo 
		&& turnsAgo < (int) shadowedHistory.size());
	return shadowedHistory.rbegin()[turnsAgo];
}

// Check if we were in naval combat on a given turn
bool Ship::wasInCombat(int turnsAgo) const {
	assert(0 <= turnsAgo 
		&& turnsAgo < (int) combatHistory.size());
	return combatHistory.rbegin()[turnsAgo];
}

// How far did we move on the search board this turn?
int Ship::getSpeed() const {
	assert(tookMoveTurn);
	return 	moveHistory.back().size();
}

// Take damage to our midships
void Ship::loseMidships(int loss) {
	midshipsLost += loss;
	applyTempEvasionLoss(loss);
}
		
// Take damage to our evasion rating (permanently)
void Ship::loseEvasion(int loss) {
	evasionLostPerm += loss;
}

// Apply temporary evasion loss from midships hit (Rule 9.72)
//   For simplicity, we use the Bismarck & Prinz Eugen reductions
//   for all ships here (presumably only German ships)
void Ship::applyTempEvasionLoss(int midshipsLost) {
	int lossPerMidships;
	switch (type) {
		case BB: case BC: case PB:
			lossPerMidships = 1;   // Rule 9.724
			break;
		case CA: case CL:
			lossPerMidships = 3;   // Rule 9.725
			break;		
		default:
			lossPerMidships = 0;
			cerr << "Error: Unhandled ship type "
				<< " for temp evasion loss.\n";
	}
	evasionLostTemp += lossPerMidships * midshipsLost;
}

// Check for evasion repair following movement (Rule 9.728)
void Ship::checkEvasionRepair() {
	assert(tookMoveTurn);
	if (evasionLostTemp && getSpeed() <= 1) {
		int repair = rollDie(6) * 2 - 4;
		repair = max(0, repair);
		repair = min(repair, evasionLostTemp);
		if (repair) {
			clog << name << " repairs " 
				<< repair << " evasion factor(s).\n";
			evasionLostTemp -= repair;
			assert(evasionLostTemp >= 0);			
		}
	}
}

// Print our waypoints (for testing)
void Ship::printWaypoints() const {
	cout << name << " waypoints: ";
	printVec(waypoints);	
}

// Log our final waypoint destination
void Ship::logDestination() {
	if (waypoints.empty()) {
		clog << name << " has no goal\n";
	}
	else {
		clog << name << " directed @ " 
			<< waypoints.back() << "\n";
	}
}

// Stream insertion operator
std::ostream& operator<<(std::ostream& stream, const Ship& ship) {
	stream << ship.getLongDesc();
	return stream;
}
