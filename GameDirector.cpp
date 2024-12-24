#include "GameDirector.h"
#include "BritishPlayerComputer.h"
#include "BritishPlayerHuman.h"
#include "GameStream.h"
#include "CmdArgs.h"
#include "Utils.h"
#include <chrono>
#include <cassert>
using namespace std;

// Singleton instance
GameDirector* GameDirector::theInstance = nullptr;

// Singleton instance accessor
GameDirector* GameDirector::instance() {
	if (!theInstance) {
		theInstance = new GameDirector;
	}
	return theInstance;
}

// Set or reset singleton for a new game
void GameDirector::initGame() {
	if (theInstance) {
		delete theInstance;	
	}
	theInstance = new GameDirector;
}

// Constructor
GameDirector::GameDirector() {
	logStartTime();
	germanPlayer = new GermanPlayer;
	auto args = CmdArgs::instance();
	britishPlayer = args->isAutomatedBritish() ?
		(BritishPlayerInterface*) new BritishPlayerComputer :
		(BritishPlayerInterface*) new BritishPlayerHuman;
	if (args->getLastTurn() > 0) {
		finishTurn = args->getLastTurn();	
	}
}

// Destructor
GameDirector::~GameDirector() {
	delete germanPlayer;
	delete britishPlayer;	
}

// Ask player(s) to start the game
bool GameDirector::okPlayerStart() {
	return britishPlayer->okStartGame();
}
		
// Ask player(s) to end the game
void GameDirector::okPlayerEnd() {
	britishPlayer->okEndGame();	
}

// Log the start time
void GameDirector::logStartTime() {
	auto clock = std::chrono::system_clock::now();
	auto timeNow = std::chrono::system_clock::to_time_t(clock);
	cgame << "Game started " << std::ctime(&timeNow);
}

// Is the game over? (Rule 12.1)
bool GameDirector::isGameOver() const {
	auto flagship = germanPlayer->getFlagship();
	return flagship.isSunk()        // Rule 12.11
		|| flagship.enteredPort()   // Rule 12.12
		|| turn > finishTurn;       // Rule 12.14
}

// Do the game loop
void GameDirector::doGameLoop() {
	while (!isGameOver()) {
		cgame << "\nTURN " << turn << endl;
		reportNightState();
		checkNewDay();
		doAvailabilityPhase();
		doVisibilityPhase();
		doShadowPhase();
		doSeaMovementPhase();
		doSearchPhase();
		doAirAttackPhase();
		doNavalCombatPhase();
		doChancePhase();
		turn++;
	}
}

// Get current turn
int GameDirector::getTurn() const {
	return turn;	
}

// Get current visibility
int GameDirector::getVisibility() const {
	return visibility;	
}

// Get the current night state
GameDirector::NightState GameDirector::getNightState() const {
	switch (turn % 6) {
		case 0: return NIGHT_SOUTH;
		case 1: return NIGHT_ALL;
		default: return DAY;		
	}
}

// Report on the night state
void GameDirector::reportNightState() {
	switch (getNightState()) {
		case NIGHT_SOUTH:
			cgame << "Night in southern latitudes.\n";
			break;
		case NIGHT_ALL:
			cgame << "Night at all latitudes.\n";
			break;
		default: // do nothing
			break;
	}
}

// Is this zone currently in night time?
bool GameDirector::isInNight(const GridCoordinate& zone) const {
	switch (getNightState()) {
		case NIGHT_SOUTH: return zone.getRow() >= 'L';
		case NIGHT_ALL: return true;
		default: return false;
	}
}

// Is this zone currently in fog?
bool GameDirector::isInFog(const GridCoordinate& zone) const {
	return foggy && SearchBoard::instance()->isFogZone(zone);
}

// Handle start of a new calendar day
void GameDirector::checkNewDay() {
	if (turn % 6 == 1) { // new day
		convoySunkToday = false;
	}
}

// Do unit availability phase
void GameDirector::doAvailabilityPhase() {
	germanPlayer->doAvailabilityPhase();
}

// Do visibility phase
void GameDirector::doVisibilityPhase() {
	if (turn > START_TURN) {
		rollVisibility();
	}
	cgame << "Visibility: " 
		<< (visibility < 9 ? to_string(visibility) : "X")
		<< (foggy ? ", with fog" : "") << endl;
}

// Do shadow phase
void GameDirector::doShadowPhase() {
	if (turn > START_TURN) {
		germanPlayer->doShadowPhase();
	}
}

// Do sea movement phase
void GameDirector::doSeaMovementPhase() {
	if (turn >= START_TURN) {
		germanPlayer->doSeaMovementPhase();
	}
}

// Do search phase
void GameDirector::doSearchPhase() {

	// Ask players for search attempts
	if (turn >= START_TURN
		&& visibility < 9)
	{
		if (britishPlayer->trySearch()) {
			britishPlayer->resolveSearch();		
		}
		if (germanPlayer->trySearch()) {
			germanPlayer->resolveSearch();		
		}
	}
}

// Is pass-through search permitted?
//   It's allowed after first turn (Rule 7.23)
bool GameDirector::isPassThroughSearchOn() const {
	return turn > START_TURN;
}

// Check British attempt to search a zone
void GameDirector::checkSearch(const GridCoordinate& zone) {
	germanPlayer->checkSearch(zone);
}

// Do chance phase
//   See Basic Game Tables Card: Chance Table
void GameDirector::doChancePhase() {
	if (turn >= START_TURN
		&& !isGameOver())
	{
		int roll = rollDice(2, 6);
		
		// Huff-duff result
		if (roll == 2) {
			germanPlayer->callHuffDuff();
		}
	
		// General Search results
		else if (roll <= 9) {
			germanPlayer->checkGeneralSearch(roll);
		}
		
		// Convoy results
		else if (roll <= 12) {
			if (!convoySunkToday     // Rule 10.26
				&& visibility < 9)   // Errata in General 16/2
			{			
				germanPlayer->checkConvoyResult(roll);
			}
		}
	}
}

// Roll for visibility
//   See Basic Player Aid Card: Visibility Track and Change
void GameDirector::rollVisibility() {
	assert(1 <= visibility && visibility <= 9);
	int roll = rollDice(2, 6);
	
	// Modify roll for current visibility
	//   NOTE: Modifier signs flipped by errata in General 16/2
	//   And more realistic per our real-data weather research
	switch (visibility) {
		case 1: roll += 1; break;
		case 7: roll -= 1; break;
		case 8: case 9: roll -= 2; break;	
	}
	
	// Determine new visibility
	visibility += roll - 7;
	visibility = max(1, visibility);
	visibility = min(9, visibility);
	
	// Determine fog
	switch (roll) {
		case 5: case 6: case 8: case 10: case 12: case 13: case 14:
			foggy = true; break;
		default:
			foggy = false; break;
	}
}

// Get notice that a convoy was sunk
void GameDirector::msgSunkConvoy() {
	convoysSunk++;
	convoySunkToday = true;	
	clog << "(sunk convoy #" << convoysSunk << ")\n";
}

// Get number of conoys sunk
int GameDirector::getConvoysSunk() const {
	return convoysSunk;	
}

// Do end-game reporting
void GameDirector::doEndGame() {
	cgame << "\nEND GAME\n";
	cgame << "Convoys Sunk: " << convoysSunk << endl;
	germanPlayer->printAllShips();
}

// Check for British shadowing a ship
void GameDirector::checkShadow(Ship& target,
	const GridCoordinate& knownPos, bool inSearchPhase) 
{
	// Ask if trying to shadow
	if (britishPlayer->tryShadow(target, knownPos, inSearchPhase)) {

		// Move target ship in shadow phase
		if (!inSearchPhase) {
			target.doMovement();
		}

		// Ask for resolution
		bool holdContact;
		britishPlayer->resolveShadow(target, holdContact);

		// Player holds contact
		if (holdContact) {
			cgame << target.getTypeName() << " shadowed to zone " 
				<< target.getPosition() << endl;
			target.setShadowed();
		}
	}
}

// Do air attack phase
void GameDirector::doAirAttackPhase() {
	if (!isGameOver()) {
		germanPlayer->doAirAttackPhase();
	}
}

// Do naval combat phase
void GameDirector::doNavalCombatPhase() {
	if (!isGameOver()) {
		germanPlayer->doNavalCombatPhase();
	}
}

// Check if the British player attacks this ship
void GameDirector::checkAttack(Ship& target, bool inSeaPhase) 
{
	// Ask if trying to attack
	if (britishPlayer->tryAttack(target, inSeaPhase)) {
		cgame << "Attack by " << (inSeaPhase ? "sea" : "air")
			<< " on " << target.getShortDesc() << "\n";
		int midshipsLost, evasionLost;
		britishPlayer->resolveAttack(midshipsLost, evasionLost);

		// Process damage
		//   (Note on tables, evasion loss only happens
		//   in conjunction with midships loss.)
		if (midshipsLost > 0) {
			cgame << target.getName() << " takes "
				<< midshipsLost << " midships and " 
				<< evasionLost << " evasion damage.\n";
			target.loseMidships(midshipsLost);
			target.loseEvasion(evasionLost);
			if (target.isSunk()) {
				cgame << target.getName() << " is sunk!\n";	
			}
		}
	}
}

// Get the number of times the German flagship was detected
int GameDirector::getTimesFlagshipDetected() const {
	return germanPlayer->getTimesFlagshipDetected();	
}
