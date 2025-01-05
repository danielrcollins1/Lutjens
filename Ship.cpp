#include "Ship.h"
#include "Utils.h"
#include "GameDirector.h"
#include "SearchBoard.h"
#include "GermanPlayer.h"
#include "Navigator.h"
#include "CmdArgs.h"
#include <cassert>
using namespace std;

// Stream insertion operator
std::ostream& operator<<(std::ostream& stream, const Ship& ship) {
	stream << ship.getLongDesc();
	return stream;
}

// Ship type abbreviations
const char* Ship::typeAbbr[]
	= {"BB", "BC", "PB", "CV", "CA", "CL", "DD", "CT", "SS", "UB"};

// Ship type full names
const char* Ship::typeName[]
	= {"Battleship", "Battlecruiser", "Pocket Battleship", 
		"Aircraft Carrier", "Heavy Cruiser", "Light Cruiser",
		"Destroyer", "Contre-Torpilleur", "Submarine", "U-Boat"};

// Ship class type names
const char* Ship::classTypeName[]
	= {"Battleship", "Aircraft Carrier", "Cruiser", 
		"Destroyers", "Submarines"};

// Constructor
//   DriveDefense indicates evasion loss rate (Rule 9.72)
Ship::Ship(std::string name, Type type, 
	int evasion, int midships, int fuel, 
	GridCoordinate position,
	GermanPlayer* player)
{
	this->name = name;
	this->type = type;
	this->evasionMax = evasion;
	this->midshipsMax = midships;
	this->fuelMax = fuel;
	this->player = player;
	this->position = position;
	fuelLost = 0;
	midshipsLost = 0;
	evasionLostTemp = 0;
	evasionLostPerm = 0;
	timesDetected = 0;
	onPatrol = false;
	loseMoveTurn = false;
	returnToBase = false;
	taskForce = nullptr;
	setEvasionLossRate();
}

// Get the name
string Ship::getName() const {
	return name;	
}

// Get the full name of this type
string Ship::getTypeName() const {
	return typeName[type];	
}

// Get the full name of this class type
string Ship::getClassTypeName() const {
	return classTypeName[getClassType()];
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
		+ ", zone " + position.toString()
		+ (onPatrol ? ", patrol" : "")
		+ ")";
}

// Get a descriptor for opponent search results
std::string Ship::getSearchDesc() const {
	return isTaskForceFlagship() ?
		taskForce->getTypeDesc() : getLongDesc();
}

// Get the type of ship
Ship::Type Ship::getType() const {
	return type;	
}

// Get the category of ship
//   For purposes of movement, fueling, withdrawal, etc.
//   E.g., Rules 5.21, 9.72, 9.93, 16.4, 58.5, 67.22
Ship::ClassType Ship::getClassType() const {
	switch (type) {
		default: return BATTLESHIP;
		case CV: return CARRIER;
		case CA: case CL: return CRUISER;
		case DD: case CT: return DESTROYER;
		case SS: case UB: return SUBMARINE;
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

// Take expense to our fuel
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
void Ship::applyTempEvasionLoss(int midshipsLoss) {
	evasionLostTemp += midshipsLoss * evasionLossRate;
}

// Set how much evasion we lose per midships hit
//   This quality is given by ship type & name in the rules
//   So we should parse the name on ship construction
void Ship::setEvasionLossRate() {
	int rate = 0;
	switch (getClassType()) {

		case BATTLESHIP: case CARRIER:
			// Rules 9.724, 48.2
			rate = (name == "Bismarck" || name == "Tirpitz") ? 1 : 2;
			break;

		case CRUISER:
			// Rules 9.725, 9.726, and class inference
			rate = (name == "Prinz Eugen" || name == "Hipper") ? 3 : 5;
			break;
		
		default:
			// Other types don't engage in basic naval combat 
			// (Destroyer rule 23.32, Submarine rule 22.17)
			break;
	}
	evasionLossRate = rate;
}

// Try to repair evasion following movement (Rule 9.728)
void Ship::tryEvasionRepair() {
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

// Set new position
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

// Do ordered movement for turn
void Ship::doMovement() {
	player->getOrders(*this);
	assert(hasOrders());

	// Lose a turn if needed
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
	
	// Finish the move turn
	doPostMoveAccounts();	
}

// Perform post-move accounting (fuel & repairs)
void Ship::doPostMoveAccounts() {
	int speed = logNow().moves.size();
	loseFuel(getFuelExpense(speed));
	checkFuelForWeather(speed);
	tryEvasionRepair();
}

// Perform orders to move on search board
void Ship::doMoveOrder() {
	assert(orders.front().type == MOVE);

	// Plot new route if needed
	if (route.empty()) {
		plotRoute(orders.front().zone);
	}

	// Select speed to move
	int speed = getMaxSpeedThisTurn();
	if (getFuel() == 1) { // avoid emptying
		speed = min(1, speed);	
	}
	
	// Try to perform movement
	for (int step = 0; step < speed; step++) {
		if (!route.empty()) {
			auto next = route.front();
			route.pop();
			position = next;
			logNow().moves.push_back(position);
			updateOrders();
			//cout << *this << endl;
		}
	}
}

// Get the max speed class on the search board
//   See Basic Game Tables Card: Movement on Search Board
//   Note this is effectively in half-zone units per turn
int Ship::getMaxSpeedClass() const {
	int evasion = getEvasion();
	if (evasion <= 6) return 0;
	else if (evasion <= 15) return 1;
	else if (evasion <= 24) return 2;
	else if (evasion <= 34) return 3;
	else {
		// No ship in the published game has speed this high.
		// In this case, search board speed would be full 2 zones/turn.
		// A few destroyers/cruisers in WWII had speeds up to 45 knots.
		cerr << "Error: Ship evasion above game allowances.\n";
		return 4;
	}
}

// How many board spaces can we move this turn?
//   See Basic Game Tables Card: Movement on Search Board
int Ship::getMaxSpeedThisTurn() const {

	// Out-of-fuel handler (Rule 5.23)
	if (!getFuel()) {
		return getEmergencySpeedThisTurn();	
	}
	
	// First-turn breakout bonus (Rule 5.28)
	if (isOnBreakoutBonus()) {
		return 5;		
	}
	
	// Standard cases	
	switch(getMaxSpeedClass()) {
		case 0: return 0;
		case 1: return getEmergencySpeedThisTurn();
		case 2: return 1;
		case 3: return log.rbegin()[1].moves.size() > 1 ? 1 : 2;
		default: return 2;
	}
}

// How far can emergency movement take us this turn? (Rule 5.24)
int Ship::getEmergencySpeedThisTurn() const {
	return GameDirector::instance()->isConvoyTurn() ? 1 : 0;
}

// Are we in a friendly port?
//   Note that we're not considered in port until 
//   the turn after we enter the port zone (Rule 12.7)
bool Ship::isInPort() const {
	return SearchBoard::instance()->isGermanPort(position)
		&& !isEnteringPort();
}

// Did we enter a friendly port zone on our most recent move?
bool Ship::isEnteringPort() const {
	return !log.empty()
		&& !log.back().moves.empty()
		&& SearchBoard::instance()->isGermanPort(
			log.back().moves.back());
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
		assert(route.empty());
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
	clearRoute();
}

// Get a string descriptor for an order
string Ship::Order::toString() const {
	switch (type) {
		case MOVE: return "Move to " + zone.toString();
		case PATROL: return "Patrol";
		case STOP: return "Stop";
		default: return "Unknown";
	}
}

// Get a random nearby space to which we can move
GridCoordinate Ship::randMoveInArea(int radius) const {
	auto board = SearchBoard::instance();
	GridCoordinate move = GridCoordinate::NO_ZONE;
	while (!isAccessible(move) || board->isGermanPort(move)) {
		move = board->randSeaZone(position, radius);
	}
	return move;
}

// Return type of the frontmost order
Ship::OrderType Ship::getFirstOrder() const {
	assert(hasOrders());
	return orders.front().type;
}

// Compute the fuel we expend at a given speed
//   Does not include optional variation for weather (Rule 16.4),
//   which refers back here (would be recursive call)
int Ship::getFuelExpense(int speed) const {
	int expense;
	auto game = GameDirector::instance();

	// First-turn breakout bonus (Rule 5.28)
	if (isOnBreakoutBonus()) {
		assert(speed <= 5);
		switch (speed) {
			case 5: return 2;
			case 4: return 1;
			default: return 0;
		}
	}

	// Handle normal cases
	assert(speed <= 2);
	switch (getClassType()) {
	
		case BATTLESHIP: case CARRIER:
			// Rule 5.21
			expense = (speed < 2) ? 0 : 1;

			// Slow battleship: Rules 5.25 & 5.27
			if (evasionMax <= 24 && speed == 1
				&& !game->isConvoyTurn()) 
			{
				expense = 1;
			}
			return expense;
	
		case CRUISER:
			// Rule 5.21
			expense = 0;
		
			// Optional fuel expenditure: Rule 16.2
			if (CmdArgs::instance()->useOptFuelExpenditure()) {
				expense = (speed < 2) ? 0 : 1;
			}
			return expense;

		case DESTROYER:
			// Rule 23.21
			return speed < 2 ? 1 : 3;

		case SUBMARINE:
			// Rule 22.14
			return 0;
			
		default:
			cerr << "Error: Unhandled ship class type\n";
			assert(false);
			return 0;
	}
}

// Expend extra fuel at bad visibility levels
//   As per optional Rule 16.4 on Fuel Expenditure
void Ship::checkFuelForWeather(int speed) {
	if (CmdArgs::instance()->useOptFuelExpenditure()) {
		int visibility = GameDirector::instance()->getVisibility();
		switch (getClassType()) {

			case BATTLESHIP: 
			case CARRIER:
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

			case SUBMARINE:
				// Doesn't track fuel (Rule 22.14)
				break;
		}
	}
}

// Check for fuel lost from combat damage
//   As per optional Rule 21.0 on Fuel Damage
void Ship::checkFuelDamage(int midshipsLoss) {
	if (CmdArgs::instance()->useOptFuelDamage()) {
		for (int i = 0; i < midshipsLoss; i++) {
			if (dieRoll(6) >= 5) {
				loseFuel(1);
			}
		}
	}
}

// Get a route from Navigator
void Ship::plotRoute(const GridCoordinate& goal) {
	clearRoute();
	auto navRoute = Navigator::findSeaRoute(*this, goal);
	for (auto zone: navRoute) {
		route.push(zone);	
	}
}

// Clear the current route plot
void Ship::clearRoute() {
	queue<GridCoordinate> empty;
	swap(route, empty);
}

// Set the return to base (RTB) marker (Rule 16.3)
void Ship::setReturnToBase() {
	returnToBase = true;	
}

// Is the return to base (RTB) marker set?
bool Ship::isReturnToBase() const {
	return returnToBase;	
}

// Are we using the breakout bonus first-turn move (Rule 5.28)?
bool Ship::isOnBreakoutBonus() const {
	return GameDirector::instance()->isFirstTurn();
}

// Join a task force
void Ship::joinTaskForce(TaskForce* taskForce) {
	this->taskForce = taskForce;
}

// Leave a task force
void Ship::leaveTaskForce() {
	taskForce = nullptr;
}

// Are we in a task force?
bool Ship::isInTaskForce() const { 
	return taskForce != nullptr;
}

// Are we the flagship of a task force?
bool Ship::isTaskForceFlagship() const {
	return isInTaskForce()
		&& taskForce->getFlagship() == this;
}

// Are we a task force escort (non-flagship)?
bool Ship::isTaskForceEscort() const {
	return isInTaskForce()
		&& taskForce->getFlagship() != this;
}

// Get the task force we're in
TaskForce* Ship::getTaskForce() const { 
	return taskForce;
}

// Move as the task force we're in
void Ship::moveWithTaskForce() {
	assert(isInTaskForce());
	auto flagship = taskForce->getFlagship();
	position = flagship->position;
	onPatrol = flagship->onPatrol;
	logNow().moves = flagship->logNow().moves;
	assert((int) logNow().moves.size() <= getMaxSpeedThisTurn());
	doPostMoveAccounts();
}
