#include "GameDirector.h"
#include "SearchBoard.h"
#include "BritishPlayerInterface.h"
#include "BritishPlayerComputer.h"
#include "BritishPlayerHuman.h"
#include "GermanPlayer.h"
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
	auto args = CmdArgs::instance();
	germanPlayer = new GermanPlayer;
	britishPlayer = args->isAutomatedBritish() ?
		(BritishPlayerInterface*) new BritishPlayerComputer :
		(BritishPlayerInterface*) new BritishPlayerHuman;
	if (args->getLastTurn() > 0) {
		finishTurn = args->getLastTurn();	
	}
	dailyConvoySunk.push_back(false);
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
	return flagship.isSunk()           // Rule 12.11
		|| flagship.isEnteringPort()   // Rule 12.12
		|| turn > finishTurn;          // Rule 12.14
}

// Do the game loop
void GameDirector::doGameLoop() {
	while (!isGameOver()) {
		cgame << "\nTURN " << turn << endl;
		checkNewDay();
		doAvailabilityPhase();
		doVisibilityPhase();
		doShadowPhase();
		doShipMovementPhase();
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

// Get number of turns completed
int GameDirector::getTurnsElapsed() const {
	return turn - START_TURN;	
}

// Are we in the first turn (or before)?
bool GameDirector::isFirstTurn() const {
	return getTurnsElapsed() <= 0;	
}

// Get current visibility
int GameDirector::getVisibility() const {
	assert(visibility <= VISIBILITY_X);
	return visibility;	
}

// Is visibility at the maximum level?
//   Prevents search, combat, convoys (Rule 7.17 + errara)
bool GameDirector::isVisibilityX() const {
	assert(visibility <= VISIBILITY_X);
	return visibility == VISIBILITY_X;	
}

// Report on night time
void GameDirector::reportNightTime() {
	switch (turn % 6) {
		case 0: cgame << "Night in southern latitudes.\n"; break;
		case 1: cgame << "Night at all latitudes.\n"; break;
		default: break; // Day; print nothing
	}
}

// Is this zone currently in night time? (Rule 11.11)
bool GameDirector::isInNight(const GridCoordinate& zone) const {
	switch (turn % 6) {
		case 0: return zone.getRow() >= 'L';
		case 1: return true;
		default: return false;
	}
}

// Is this zone currently in daylight?
bool GameDirector::isInDay(const GridCoordinate& zone) const {
	return !isInNight(zone);
}

// Is this zone currently in fog?
bool GameDirector::isInFog(const GridCoordinate& zone) const {
	return foggy && SearchBoard::instance()->isFogZone(zone);
}

// Is this zone currently searchable at the given search strength?
bool GameDirector::isSearchable(
	const GridCoordinate& zone, int strength) const
{
	return !isVisibilityX()
		&& !isInFog(zone)
		&& strength >= visibility;
}

// Handle start of a new calendar day
void GameDirector::checkNewDay() {
	if (turn % 6 == 1) { // new day
		dailyConvoySunk.push_back(false);
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
		<< (visibility == VISIBILITY_X ? "X" : to_string(visibility))
		<< (foggy ? ", with fog" : "") << endl;
	reportNightTime();
}

// Do shadow phase
void GameDirector::doShadowPhase() {
	if (turn > START_TURN) {
		germanPlayer->doShadowPhase();
	}
}

// Do ship movement phase
void GameDirector::doShipMovementPhase() {
	if (turn >= START_TURN) {
		germanPlayer->doShipMovementPhase();
	}
}

// Do search phase
void GameDirector::doSearchPhase() {

	// Ask players for search attempts
	if (turn >= START_TURN
		&& visibility < VISIBILITY_X)
	{
		if (britishPlayer->trySearch()) {
			britishPlayer->resolveSearch();		
		}
		if (germanPlayer->trySearch()) {
			germanPlayer->resolveSearch();		
		}
	}
}

// Search a zone for German ships
bool GameDirector::searchGermanShips(const GridCoordinate& zone) {
	return germanPlayer->checkSearch(zone);
}

// Search a zone for British ships
bool GameDirector::searchBritishShips(const GridCoordinate& zone) {
	return britishPlayer->checkSearch(zone);
}

// Do chance phase
//   See Basic Game Tables Card: Chance Table
void GameDirector::doChancePhase() {
	if (turn >= START_TURN && !isGameOver()) {
		germanPlayer->doChancePhase();
	}
}

// Roll for visibility
//   See Basic Player Aid Card: Visibility Track and Change
void GameDirector::rollVisibility() {
	assert(isInInterval(1, visibility, VISIBILITY_X));
	int roll = diceRoll(2, 6);
	
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
	assert(isInInterval(1, visibility, VISIBILITY_X));
	
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
	dailyConvoySunk.back() = true;
}

// Get number of convoys sunk
int GameDirector::getConvoysSunk() const {
	return count(dailyConvoySunk.begin(), dailyConvoySunk.end(), true);
}

// Check if a convoy was sunk on a given day
bool GameDirector::wasConvoySunk(unsigned daysAgo) const {
	return daysAgo < dailyConvoySunk.size() ?
		dailyConvoySunk.rbegin()[daysAgo] : false;
}

// Do end-game reporting
void GameDirector::doEndGame() {
	cgame << "\nEND GAME\n";
	cgame << "Convoys Sunk: " << getConvoysSunk() << endl;
	germanPlayer->printAllShips();
}

// Check for British shadowing a ship (Rule 8.0)
void GameDirector::checkShadow(Ship& target,
	const GridCoordinate& knownPos, Phase phase) 
{
	assert(phase == SHADOW || phase == SEARCH);
	
	// Ask if trying to shadow
	if (britishPlayer->tryShadow(target, knownPos, phase)) {
		target.setShadowed();

		// Move target ship in shadow phase
		if (phase == SHADOW) {
			target.doMovementTurn();
		}

		// Ask for resolution
		bool holdContact;
		britishPlayer->resolveShadow(target, holdContact);

		// Player holds contact
		if (holdContact) {
			target.setLocated();
			cgame << target.getTypeDesc() << " shadowed to zone " 
				<< target.getPosition() << endl;
		}
	}
}

// Do air attack phase
void GameDirector::doAirAttackPhase() {
	if (!isGameOver() && !isVisibilityX()) {
		germanPlayer->doAirAttackPhase();
	}
}

// Do naval combat phase
void GameDirector::doNavalCombatPhase() {
	if (!isGameOver() && !isVisibilityX()) {
		germanPlayer->doNavalCombatPhase();
	}
}

// Check if the British player attacks this ship
void GameDirector::checkAttackOn(Ship& target, Phase phase) 
{
	assert(phase == AIR_ATTACK || phase == NAVAL_COMBAT);
	if (britishPlayer->tryAttack(target, phase)) {
		cgame << "Attack by " << (phase == AIR_ATTACK ? "air" : "sea")
			<< " on " << target.getFullDesc() << "\n";
		if (phase == NAVAL_COMBAT) {
			target.setCombated();
		}
		resolveCombat(target);
	}
}

// Check if this ship attacks a British ship
void GameDirector::checkAttackBy(Ship& attacker) {
	cgame << attacker.getFullDesc() 
		<< " attacks solo cruiser in " << attacker.getPosition() << "\n";
	attacker.setCombated();
	resolveCombat(attacker);
}

// Get combat resolution with this German shop
//   Note evasion loss only happens in conjunction
//   with midships loss, per Battle Board tables
void GameDirector::resolveCombat(Ship& ship) {
	int midshipsLost, evasionLost;
	britishPlayer->resolveAttack(midshipsLost, evasionLost);
	if (midshipsLost > 0) {
		cgame << ship.getName() << " takes "
			<< midshipsLost << " midships and " 
			<< evasionLost << " evasion damage.\n";
		ship.loseMidships(midshipsLost);
		ship.loseEvasion(evasionLost);
		if (ship.isSunk()) {
			cgame << ship.getName() << " is sunk!\n";	
		}
	}
}

// Get the number of times the German flagship was detected
int GameDirector::getTimesFlagshipDetected() const {
	return germanPlayer->getTimesFlagshipDetected();	
}

// Is this a turn in which convoys move?
//   That is: A "C-turn" on Time Record Track (Rule 5.24, etc.)
bool GameDirector::isConvoyTurn() const {
	return !(turn % 2);
}
