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
	stream << ship.getFullDesc();
	return stream;
}

// Ship type abbreviations
const char* Ship::typeAbbr[]
	= {"BB", "BC", "PB", "CV", "CA", "CL", "DD", "CT", "SS", "UB"};

// Ship class type names
const char* Ship::generalTypeName[]
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
	this->position = position;
	this->player = player;
	fuelLost = 0;
	midshipsLost = 0;
	evasionLostTemp = 0;
	evasionLostPerm = 0;
	timesDetected = 0;
	onPatrol = false;
	returnToBase = false;
	taskForce = nullptr;
	setEvasionLossRate();
}

// Get our size
int Ship::getSize() const {
	return 1;	
}

// Get our pointer
Ship* Ship::getShip(int idx) {
	assert(idx == 0);
	return this;	
}

// Get commanding ship (this)
const Ship* Ship::getFlagship() const {
	assert(!isInTaskForce());
	return this;
}

// Get commanding ship (non-const)
Ship* Ship::getFlagship() {
	// Per Scott Meyers: stackoverflow.com/a/123995/5407757
	return const_cast<Ship*>
		(static_cast<const Ship&>(*this).getFlagship());
}

// Get the name
string Ship::getName() const {
	return name;	
}

// Get a description with only ship types
string Ship::getTypeDesc() const {
	return generalTypeName[getGeneralType()];
}

// Get a description with any ship names
string Ship::getNameDesc() const {
	return getName();	
}

// Get a description with full information
string Ship::getFullDesc() const {
	return name
		+ " (" + typeAbbr[type]
		+ ", evrtg " + to_string(getEvasion())
		+ ", mships " + to_string(getMidships())
		+ ", fuel " + to_string(getFuel())
		+ ", zone " + position.toString()
		+ (onPatrol ? ", patrol" : "")
		+ (returnToBase ? ", RTB" : "")
		+ ")";
}

// Get the type of ship
Ship::Type Ship::getType() const {
	return type;	
}

// Get the general type of ship
Ship::GeneralType Ship::getGeneralType() const {
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

// Get evasion for launching an attack
int Ship::getAttackEvasion() const {
	return getEvasion();	
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
//   So we parse the name on ship construction
void Ship::setEvasionLossRate() {
	int rate = 0;
	switch (getGeneralType()) {

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

// Try to repair evasion after movement (Rule 9.728)
void Ship::tryEvasionRepair() {
	if (evasionLostTemp && getSpeedThisTurn() <= 1) {
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

// Get our current search strength
//   Note this is inferred for available German ship types
int Ship::getSearchStrength() const {
	switch (type) {
		case BB: case BC: 
		case CA: return onPatrol ? (isInDay() ? 4 : 3) : 1;
		case CL: return onPatrol ? (isInDay() ? 4 : 2) : 1;
		case PB: return onPatrol ? (isInDay() ? 3 : 2) : 1;
		case UB: return onPatrol ? 4 : (isInDay() ? 3 : 2);
		case CV: case CT: return 1; // no patrol (Rules 5.31, 23.17)
		default:
			cerr << "Error: Unhandled ship type in search\n";
			assert(false);
			return 0;
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
void Ship::doMovementTurn() {
	player->getOrders(*this);
	assert(hasOrders());

	// Obey first order
	onPatrol = false;
	switch (orders.front().type) {
		case MOVE: doMoveOrder(); break;
		case PATROL: onPatrol = true; break;
		case STOP: break;
	}
	
	// Finish the move turn
	doPostMoveAccounts();	
}

// Follow the leader of our task force
void Ship::followShip(Ship& flagship) {
	assert(isInTaskForce());
	position = flagship.position;
	onPatrol = flagship.onPatrol;
	logNow().moves = flagship.logNow().moves;
	assert(getSpeedThisTurn() <= getMaxSpeedThisTurn());
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
	if (isInTaskForce()) { // lead task force
		speed = min(speed, taskForce->getMaxSpeedThisTurn());
	}
	
	// Try to perform movement
	for (int step = 0; step < speed; step++) {
		if (!route.empty()) {
			auto next = route.back();
			route.pop_back();
			assert(isAdjacent(next));
			position = next;
			logNow().moves.push_back(position);
			updateOrders();
		}
	}
}

// Get the max speed class on the search board
//   See Basic Game Tables Card: Movement on Search Board
//   This is effectively in half-zone units per turn
int Ship::getMaxSpeedClass() const {
	int evasion = getEvasion();
	if (evasion <= 6) return 0;
	else if (evasion <= 15) return 1;
	else if (evasion <= 24) return 2;
	else if (evasion <= 34) return 3;
	else {
		// No ship in the published game has speed this high.
		// We infer the search board speed would be fully 2 zones/turn.
		// A few destroyers/cruisers in WWII had speeds up to 45 knots.
		cerr << "Error: Ship evasion above game allowances.\n";
		return 4;
	}
}

// How many board spaces can we move this turn?
int Ship::getMaxSpeedThisTurn() const {

	// No move after convoy sunk (Rule 10.25)
	if (wasConvoySunk(1)) {
		return 0;
	}

	// Out-of-fuel handler (Rule 5.23)
	if (!getFuel()) {
		return getEmergencySpeedThisTurn();	
	}
	
	// First-turn breakout bonus (Rule 5.28)
	if (isOnBreakoutBonus()) {
		return 5;		
	}
	
	// Standard cases (Basic Game Tables Card)
	switch(getMaxSpeedClass()) {
		case 0: return 0;
		case 1: return getEmergencySpeedThisTurn();
		case 2: return 1;
		case 3: return log.rbegin()[1].moves.size() > 1 ? 1 : 2;
		default: cerr << "Error: Unhandled speed class\n";
			return 0;
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

// Are we afloat?
bool Ship::isAfloat() const {
	return getMidships() > 0;	
}

// Are we on the search board?
bool Ship::isOnBoard() const {
	return isAfloat()
		&& getPosition() != GridCoordinate::OFFBOARD;
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

// Note that we have been detected by any means:
//   Search, shadow, general search, or HUFF-DUFF,
//   but not reveal for convoy attack.
//   Used for statistical models (not part of game).
void Ship::setDetected() {
	timesDetected++;
}

// Check how many times we were detected by any means
int Ship::getTimesDetected() const {
	return timesDetected;	
}

// Note that we have been located by search/shadow
void Ship::setLocated() {
	logNow().located = true;
	setDetected();
}

// Note that the enemy tried to shadow us
//   And so we have moved earlier in the turn (Rule 8.11)
//   Compare to Shadow marker usage (Rule 2.53)
void Ship::setShadowed() {
	logNow().shadowed = true;
}

// Note that we have entered naval combat
void Ship::setCombated() {
	logNow().combated = true;
}

// Note that we (helped) sank a convoy
void Ship::setConvoySunk() {
	logNow().convoySunk = true;	
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

// Check if we (helped) sank a convoy on a given turn
bool Ship::wasConvoySunk(unsigned turnsAgo) const {
	return turnsAgo < log.size() ?
		log.rbegin()[turnsAgo].convoySunk : false;
}

// How far did we move on the search board this turn?
int Ship::getSpeedThisTurn() const {
	return log.back().moves.size();
}

// Return the log record for the current turn
Ship::LogTurn& Ship::logNow() {
	return log.back();	
}

// Receive a non-move order
void Ship::orderAction(OrderType type) {
	assert(type != MOVE);
	Order order = {type, GridCoordinate::OFFBOARD};
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
	route.clear();
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
	GridCoordinate move = GridCoordinate::OFFBOARD;
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
	switch (getGeneralType()) {
	
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
		switch (getGeneralType()) {

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

// Get a route from the Navigator
void Ship::plotRoute(const GridCoordinate& goal) {
	route.clear();
	if (goal == GridCoordinate::OFFBOARD) {
		assert(position.getRow() == 'Z');
		route.push_back(goal);
	}
	else {
		route = Navigator::findSeaRoute(*this, goal);
	}
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
	return GameDirector::instance()->isStartTurn();
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

// How many search board spaces can we move per turn?
float Ship::getMaxSpeedAvg() const {
	float speed = (float) getMaxSpeedClass() / 2;
	switch (getFuel()) {
		case 0: speed = min(speed, 0.5f); break;
		case 1: speed = min(speed, 1.0f); break;	
	}
	return speed;
}

// What turn is the earliest we could reach a convoy route zone?
int Ship::convoyETA() const {
	int minDist = INT_MAX;
	auto convoyZones = SearchBoard::instance()->getAllConvoyRoutes();	
	for (auto zone: convoyZones) {
		int distance = position.distanceFrom(zone);
		if (distance < minDist) {
			minDist = distance;	
		}
	}
	int turnsToGo = (int) (minDist / getMaxSpeedAvg());
	return GameDirector::instance()->getTurn() + turnsToGo;
}

// What turn should we arrive at end of our plotted route?
int Ship::routeETA() const {
	int turnsToGo = (int) (route.size() / getMaxSpeedAvg());
	return GameDirector::instance()->getTurn() + turnsToGo;
}

// What turn could we get to row Z? (note Rule 51.6)
int Ship::rowZ_ETA() const {
	int distance = 'Z' - position.getRow();
	int turnsToGo = (int) (distance / getMaxSpeedAvg());
	return GameDirector::instance()->getTurn() + turnsToGo;
}

// Are we adjacent to this zone?
bool Ship::isAdjacent(const GridCoordinate& zone) const {
	return zone == GridCoordinate::OFFBOARD ?
		position.getRow() == 'Z': 
		position.distanceFrom(zone) == 1;
}
