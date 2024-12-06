#include "GameDirector.h"
#include "PlayerGerman.h"
#include "GameStream.h"
#include "Utils.h"
#include <cstdlib>
#include <ctime>
#include <chrono>

// Switch to turn on human UI
const bool DO_HUMAN_UI = true;

// Singleton instance
GameDirector* GameDirector::theInstance = nullptr;

// Singlton instance accessor
GameDirector* GameDirector::instance() {
	if (!theInstance) {
		theInstance = new GameDirector;
	}
	return theInstance;
}

// Constructor
GameDirector::GameDirector()
{
	logStartTime();
	srand(time(0));
	playerGerman = new PlayerGerman;
	turn = FIRST_TURN;
	visibility = START_VISIBILITY;
	foggy = START_FOG;
	convoysSunk = 0;
	convoySunkToday = false;
}

// Log the start time
void GameDirector::logStartTime() {
	auto clock = std::chrono::system_clock::now();
	auto timeNow = std::chrono::system_clock::to_time_t(clock);
	cgame << "Game started " << std::ctime(&timeNow);
}

// Is the game over? (Rule 12.1)
bool GameDirector::isGameOver() const {
	auto bismarck = playerGerman->getBismarck();
	return bismarck.isSunk()        // Rule 12.11
		|| bismarck.enteredPort()   // Rule 12.12
		|| turn > FINISH_TURN;      // Rule 12.14
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
	playerGerman->doAvailabilityPhase();
}

// Do visibility phase
void GameDirector::doVisibilityPhase() {
	if (turn > BRITISH_START_TURN) {
		rollVisibility();
	}
	cgame << "Visibility: " 
		<< (visibility < 9 ? to_string(visibility) : "X")
		<< (foggy ? ", with fog" : "") << endl;
}

// Do shadow phase
void GameDirector::doShadowPhase() {
	if (turn > BRITISH_START_TURN) {
		playerGerman->doShadowPhase();
	}
}

// Do sea movement phase
void GameDirector::doSeaMovementPhase() {
	if (turn >= GERMAN_START_TURN) {
		playerGerman->doSeaMovementPhase();
	}
}

// Do search phase
void GameDirector::doSearchPhase() {
	
	// Ask human British player for search requests
	if (DO_HUMAN_UI 
		&& turn >= BRITISH_START_TURN
		&& visibility < 9)
	{
		const char END_SEARCH = '@';
		cout << "Enter zones to search "
			<< "(" <<  END_SEARCH << " to end):\n";
		string input;
		do {
			cout << "==> ";
			cin >> input;
			if (input[0] == END_SEARCH) {
				break;
			}
			if (!GridCoordinate::isValid(input)) {
				cout << "> Invalid grid coordinate.\n";
				continue;
			}
			GridCoordinate zone(input);
			if (isInFog(zone)) {
				cout << "> Cannot search in fog.\n";
				continue;
			}
			playerGerman->checkSearch(zone);
		} while (true);
	}
}

// Do chance phase
//   See Basic Game Tables Card: Chance Table
void GameDirector::doChancePhase() {
	if (turn >= BRITISH_START_TURN
		&& !isGameOver())
	{
		int roll = rollDice(2, 6);
		
		// Huff-duff result
		if (roll == 2) {
			playerGerman->callHuffDuff();
		}
	
		// General Search results
		else if (roll <= 9) {
			playerGerman->checkGeneralSearch(roll);
		}
		
		// Convoy results
		else if (roll <= 12) {
			if (!convoySunkToday     // Rule 10.26
				&& visibility < 9)   // Errata in General 16/2
			{			
				playerGerman->checkConvoyResult(roll);
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

// Do end-game reporting
void GameDirector::doEndGame() {
	cgame << "\nEND GAME\n";
	cgame << "Convoys Sunk: " << convoysSunk << endl;
	playerGerman->printAllShips();
}

// Check if the human player wants to shadow a ship
//   Argument 'inSearchPhase' implies high-speed shadow (Rule 8.2)
void GameDirector::checkShadow(Ship& target, bool inSearchPhase, 
	const GridCoordinate& knownPos) 
{
	// Ask for shadow attempt
	if (!inSearchPhase) {
		cout << target.getTypeAndEvasion()
			<< " was seen in " << knownPos << "\n";
	}
	cout << "Do you wish to shadow (y/n)? ";

	// Player tries to shadow
	if (getUserYes()) {
		string targetID = target.getTypeName();

		// Move target ship in shadow phase
		if (!inSearchPhase) {
			target.doMovement();
		}
		
		// Ask for shadow resolution
		if (target.getSpeed() > 1) {
			cout << targetID << " moves at high speed (apply +1).\n";
		}
		cout << "Resolve attempt on the Shadow Table.\n";
		cout << "Did shadow attempt hold contact (y/n)? ";

		// Player holds contact
		if (getUserYes()) {
			cgame << targetID << " shadowed to zone " 
				<< target.getPosition() << endl;
			target.setExposed();
		}
	}
}

// Do air attack phase
void GameDirector::doAirAttackPhase() {
	if (!isGameOver()) {
		playerGerman->doAirAttackPhase();
	}
}

// Do naval combat phase
void GameDirector::doNavalCombatPhase() {
	if (!isGameOver()) {
		playerGerman->doNavalCombatPhase();
	}
}

// Check if the human player attacks this ship
void GameDirector::checkAttack(Ship& target, bool inSeaPhase) 
{
	// Prompt
	cout << (inSeaPhase ? 
		target.getTypeAndEvasion(): target.getTypeName())
		<< 	" is seen in " << target.getPosition() << "\n";
	string atkType = inSeaPhase ? "sea" : "air";
	cout << "Do you wish to attack by " << atkType << " (y/n)? ";

	// Get input
	if (getUserYes()) {
		cgame << "Attack by " << atkType 
			<< " on " << target.getShortDesc() << "\n";
		resolveAttack(target);
	}
}

// Resolve a player attack on this ship
void GameDirector::resolveAttack(Ship& target) {

	// Ask for damage results
	cout << "Resolve attack on the Battle Board.\n";
	cout << "Enter midships and evasion damage from table: ";
	int midshipsLost, evasionLost;
	cin >> midshipsLost >> evasionLost;

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
