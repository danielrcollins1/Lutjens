#include "Ship.h"
#include "Utils.h"
#include "GameDirector.h"
#include "GermanPlayer.h"
#include "CmdArgs.h"
#include <cassert>
using namespace std;

// Stream insertion operator
std::ostream& operator<<(std::ostream& stream, const Ship& ship) {
	stream << ship.getLongDesc();
	return stream;
}

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
	int evasion, int midships, int fuel,
	GermanPlayer* player) 
{
	this->name = name;
	this->type = type;
	this->evasionMax = evasion;
	this->midshipsMax = midships;
	this->fuelMax = fuel;
	this->player = player;
	position = GridCoordinate::NO_ZONE;
	onPatrol = false;
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

// Get the category of ship
//   For purposes of movement, fueling, withdrawal, etc.
//   E.g., Rules 5.21, 9.72, 9.93, 16.4, 58.5, 67.22
Ship::ClassType Ship::getClassType() const {
	switch (type) {
		case BB: case BC: case PB: case CV: return BATTLESHIP;
		case CA: case CL: return CRUISER;
		case DD: return DESTROYER;
		default: cerr << "Error: Unhandled ship class type\n";
			assert(false);
			return OTHER;
	}
}

// Get the current fuel
int Ship::getFuel() const {
	return max(0, fuelMax - fuelLost);
}

// Get the current midships
int Ship::getMidships() const {
	return max(0, midshipsMax - midshipsLost);
}

// Get the current evasion
int Ship::getEvasion() const {
	return max(0, evasionMax - evasionLostTemp - evasionLostPerm);
}

// Take exspense to our fuel
void Ship::loseFuel(int loss) {
	fuelLost += loss;	
}

// Take damage to our evasion rating (permanently)
void Ship::loseEvasion(int loss) {
	evasionLostPerm += loss;
}

// Take damage to our midships
void Ship::loseMidships(int loss) {
	midshipsLost += loss;
	applyTempEvasionLoss(loss);
	checkFuelDamage(loss);
}
		
// Apply temporary evasion loss from midships hit (Rule 9.72)
//   For simplicity, we use the Bismarck & Prinz Eugen reductions
//   for all ships here (presumably only German ships)
void Ship::applyTempEvasionLoss(int midshipsLoss) {
	int lossPerMidships = 0;
	switch (getClassType()) {
		case BATTLESHIP: lossPerMidships = 1; break;
		case CRUISER: lossPerMidships = 3; break;
		default: 
			cerr << "Error: Unhandled class type "
				<< " for temporary evasion loss.\n";
			assert(false);
	}
	evasionLostTemp += lossPerMidships * midshipsLoss;
}

// Check for evasion repair following movement (Rule 9.728)
void Ship::checkEvasionRepair() {
	if (evasionLostTemp && getSpeed() <= 1) {
		int repair = dieRoll(6) * 2 - 4;
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

// Set starting position
void Ship::setPosition(const GridCoordinate& zone) {
	assert(log.empty());
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
	log.push_back(LogTurn());
}

// Do German ship first-turn breakout bonus move (Rule 5.28)
//   We use the first waypoint as our location at end of move
//   Waive most other rules restrictions here
void Ship::doBreakoutBonusMove() {
	assert(GameDirector::instance()->getTurnsElapsed() == 0);
	player->getOrders(*this);
	assert(hasOrders());

	// Get ordered position
	assert(orders.front().type == MOVE);
	auto goal = orders.front().zone;
	int distance = position.distanceFrom(goal);

	// Move to that position
	assert(distance <= 5);
	position = goal;
	logNow().moves.push_back(position);
	updateOrders();

	// Spend fuel
	switch (distance) {
		case 5: loseFuel(2); break;
		case 4: loseFuel(1); break;
		default: break; // no loss
	}
}

// Do normal movement for turn
void Ship::doMovement() {
	player->getOrders(*this);
	assert(hasOrders());

	// Lose a turn if required
	if (loseMoveTurn) {
		loseMoveTurn = false;
	}

	// Follow the next order
	else {
		onPatrol = false;
		switch (orders.front().type) {
			case MOVE: doMoveOrder(); break;
			case PATROL: onPatrol = true; break;
			case STOP: break;
		}
	}
	
	// Spend fuel
	int speed = logNow().moves.size();
	loseFuel(getFuelExpense(speed));
	checkFuelForWeather(speed);

	// Repair evasion
	checkEvasionRepair();
}

// Perform orders to move on search board
void Ship::doMoveOrder() {
	assert(orders.front().type == MOVE);

	// Select speed to move
	int speed = maxSpeed();
	if (getFuel() == 1) {
		speed = min(1, speed);	
	}
	
	// Try to perform movement
	for (int step = 0; step < speed; step++) {
		auto next = getNextZone();
		if (next != GridCoordinate::NO_ZONE) {
			position = next;
			logNow().moves.push_back(position);
			updateOrders();
		}
	}
}

// How many board spaces can we move this turn?
//   See Basic Game Tables Card: Movement on Search Board
int Ship::maxSpeed() const {
	int evasion = getEvasion();
	if (evasion <= 6) {
		return 0;
	}
	else if (evasion <= 15 || getFuel() < 1) {
		bool convoyTurn = GameDirector::instance()->isConvoyTurn();
		return convoyTurn ? 0 : 1; // speed 0.5
	}
	else if (evasion <= 24) {
		return 1;		
	}
	else if (evasion <= 34) {
		int lastTurnSpeed = log.rbegin()[1].moves.size();
		return lastTurnSpeed > 1 ? 1 : 2; // speed 1.5
	}
	else {
		// No ship in published game has speed this high.
		// In this case, search board speed would be full 2/turn.
		// A few destroyers/cruisers in WWII had speeds up to 45 knots.
		cerr << "Error: Ship evasion above game allowances.\n";
		return 2;		
	}
}

// Get one step towards destination
GridCoordinate Ship::getNextZone() const {
	if (!orders.empty()
		&& orders.front().type == MOVE)
	{		
		GridCoordinate dest = orders.front().zone;
		int dist = position.distanceFrom(dest);
		vector<GridCoordinate> adjacent = position.getAdjacent();
		vector<GridCoordinate> options;
		for (auto zone: adjacent) {
			if (isAccessible(zone) && zone.distanceFrom(dest) < dist) {
				options.push_back(zone);
			}
		}

		// If options list is empty, ship is stuck
		// Avoid map concavities or get a better pathing algorithm (A*)
		// (And if so done, fix Irish sea layer data)
		if (options.empty()) {
			cerr << "Error: Pathfinding failed, ship is stuck @ " 
				<< position << "\n";
			assert(false);
		}
		return randomElem(options);
	}
	return GridCoordinate::NO_ZONE;
}

// Are we in port?
bool Ship::isInPort() const {
	return SearchBoard::instance()->isGermanPort(position);
}

// Have we entered a port?
//   Assumes we moved out of port on first turn
bool Ship::enteredPort() const {
	return isInPort()
		&& GameDirector::instance()->getTurnsElapsed() > 0;
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
	return hasElem(log.back().moves, zone);
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
	logNow().located = true;
	noteDetected();
}

// Note that the enemy tried to shadow us
//   And so we have moved earlier in the turn (Rule 8.11)
//   Compare to Shadow marker usage (Rule 2.53)
void Ship::setShadowed() {
	logNow().shadowed = true;
}

// Note that we have entered naval combat
void Ship::setInCombat() {
	logNow().combated = true;
}

// Check if we were located by search/shadow on a given turn
bool Ship::wasLocated(unsigned turnsAgo) const {
	return turnsAgo < log.size() ?
		log.rbegin()[turnsAgo].located : false;
}

// Check if we were shadowed on a given turn
bool Ship::wasShadowed(unsigned turnsAgo) const {
	return turnsAgo < log.size() ?
		log.rbegin()[turnsAgo].shadowed : false;
}

// Check if we were in naval combat on a given turn
bool Ship::wasCombated(unsigned turnsAgo) const {
	return turnsAgo < log.size() ?
		log.rbegin()[turnsAgo].combated : false;
}

// How far did we move on the search board this turn?
int Ship::getSpeed() const {
	return log.back().moves.size();
}

// Return the log record for the current turn
Ship::LogTurn& Ship::logNow() {
	return log.back();	
}

// Receive a non-move order
void Ship::orderAction(OrderType type) {
	assert(type != MOVE);
	Order order = {type, GridCoordinate::NO_ZONE};
	pushOrder(order);
}

// Receive a movement order
//   Ignored if we're already there
void Ship::orderMove(const GridCoordinate& dest) {
	if (position != dest) {
		Order order = {MOVE, dest};
		pushOrder(order);
	}
}

// Push a new order onto our queue
void Ship::pushOrder(Order order) {
	orders.push(order);
	clog << name << " ordered to " << order.toString() << endl;
}

// Check if orders need updating
void Ship::updateOrders() {

	// If we achieved move destination, cycle to next
	while (!orders.empty()
		&& orders.front().type == MOVE
		&& orders.front().zone == position)
	{
		orders.pop();	
	}
	
	// Note: Do NOT ask for new orders at this point (if empty)
	// We want to see state/visibility next turn for best choice
}

// Do we have any pending orders?
bool Ship::hasOrders() const {
	return !orders.empty();	
}

// Clear the pending orders list
void Ship::clearOrders() {
	queue<Order> empty;
	swap(orders, empty);
}

// Get a string descriptor for an order
string Ship::Order::toString() const {
	switch (type) {
		case MOVE: return "Move-" + zone.toString();
		case PATROL: return "Patrol";
		case STOP: return "Stop";
		default: cerr << "Error: Unknown order type\n";
			return "Unknown";
	}
}

// Get a random adjacent space to which we can move
GridCoordinate Ship::randAdjacentMove() const {
	GridCoordinate move;
	auto board = SearchBoard::instance();
	do {
		move = board->randSeaWithinOne(position);
	} while (!isAccessible(move)
		|| board->isGermanPort(move));
	return move;
}

// Return type of the frontmost order
Ship::OrderType Ship::getFirstOrder() const {
	assert(hasOrders());
	return orders.front().type;
}

// Compute the fuel we expend at a given speed
//   See Rules 5.2, 16.2, 22.14, 23.21
//   Does not include optional variation for weather (Rule 16.4),
//   which refers back here (would be recursive call)
int Ship::getFuelExpense(int speed) const {
	assert(speed <= 2);
	int expense;
	auto game = GameDirector::instance();
	switch (getClassType()) {
	
		case BATTLESHIP: // Rule 5.21
			expense = (speed < 2) ? 0 : 1;

			// Slow battleship: Rules 5.25 & 5.27
			if (evasionMax <= 24 && speed == 1
				&& !game->isConvoyTurn()) 
			{
				expense = 1;
			}
			break;
	
		case CRUISER: // Rule 5.21
			expense = 0;
		
			// Optional fuel expenditure: Rule 16.2
			if (CmdArgs::instance()->useFuelExpenditure()) {
				expense = (speed < 2) ? 0 : 1;
			}
			break;

		case DESTROYER: // Rule 23.21
			expense = (speed < 2) ? 1 : 3;
			break;
			
		default: 
			cerr << "Error: Unhandled ship class\n";
			assert(false);
			expense = 0;
	}
	return expense;
}

// Expend extra fuel at bad visibility levels
//   As per optional Rule 16.4 on Fuel Expenditure
void Ship::checkFuelForWeather(int speed) {
	if (CmdArgs::instance()->useFuelExpenditure()) {
		int visibility = GameDirector::instance()->getVisibility();
		switch (getClassType()) {
			case BATTLESHIP:
				if (visibility >= 8 && speed > 0) {
					loseFuel(1);
				}
				break;
			case CRUISER: 
				if (visibility >= 7 && speed > 0) {
					loseFuel(1);
				}
				break;
			case DESTROYER:
				if (visibility >= 5) {
					loseFuel(getFuelExpense(speed));
				}
				break;
			default: 
				cerr << "Error: Unhandled ship class type\n";
				assert(false);
		}
	}
}

// Check for fuel lost from combat damage
//   As per optional Rule 21.0 on Fuel Damage
void Ship::checkFuelDamage(int midshipsLoss) {
	if (CmdArgs::instance()->useFuelDamage()) {
		for (int i = 0; i < midshipsLoss; i++) {
			if (dieRoll(6) >= 5) {
				loseFuel(1);
			}
		}
	}
}
