#include "BritishPlayerHuman.h"
#include "GameDirector.h"
#include "Utils.h"
using namespace std;

// Ask if we want to start game
bool BritishPlayerHuman::okStartGame() {
	cout << "Start game (y/n)? ";
	return getUserYes();
}

// Confirm to end the game
void BritishPlayerHuman::okEndGame() {
	cout << "Press <Enter> to finish. ";
	cin.ignore();
	cin.get();	
}

// Ask if we want to try searching
bool BritishPlayerHuman::trySearch() {
	cout << "Do you wish to search (y/n)? ";
	return getUserYes();
}

// Ask if we want to try shadowing
bool BritishPlayerHuman::tryShadow(const Ship& target, 
	const GridCoordinate& knownPos, GameDirector::Phase phase)
{
	assert(phase == GameDirector::Phase::SHADOW 
		|| phase == GameDirector::Phase::SEARCH);
	if (phase == GameDirector::Phase::SHADOW) {
		cout << target.getTypeAndEvasion()
			<< " was seen in " << knownPos << "\n";
	}
	cout << "Do you wish to shadow (y/n)? ";
	return getUserYes();
}

// Ask if we want to try attacking
bool BritishPlayerHuman::tryAttack(
	const Ship& target, GameDirector::Phase phase) 
{
	assert(phase == GameDirector::Phase::AIR_ATTACK 
		|| phase == GameDirector::Phase::NAVAL_COMBAT);
	cout << (phase == GameDirector::Phase::NAVAL_COMBAT ? 
		target.getTypeAndEvasion(): target.getGeneralTypeName())
		<< 	" is seen in " << target.getPosition() << "\n";
	cout << "Do you wish to attack by " 
		<< (phase == GameDirector::Phase::AIR_ATTACK ? "air" : "sea") 
		<< " (y/n)? ";
	return getUserYes();
}

// Resolve attempt to search
void BritishPlayerHuman::resolveSearch() {
	const char END_SEARCH = '@';
	cout << "Enter zones to search "
		<< "(" <<  END_SEARCH << " to end):\n";
	while (true) {
		cout << "==> ";
		string input;
		cin >> input;
		if (input[0] == END_SEARCH) {
			break;
		}
		if (!GridCoordinate::isValid(input)) {
			cout << "> Invalid grid coordinate.\n";
			continue;
		}
		auto director = GameDirector::instance();
		GridCoordinate zone(input);
		if (director->isInFog(zone)) {
			cout << "> Cannot search in fog.\n";
			continue;
		}
		director->searchGermanShips(zone);
	}
}

// Resolve attempt to shadow
void BritishPlayerHuman::resolveShadow(const Ship& target, bool& heldContact) 
{
	string targetID = target.getGeneralTypeName();	
	if (target.getSpeed() > 1) {
		cout << targetID << " moves at high speed (apply +1).\n";
	}
	cout << "Resolve attempt on the Shadow Table.\n";
	cout << "Did shadow attempt hold contact (y/n)? ";
	heldContact = getUserYes();
}

// Resolve attempt to attack
void BritishPlayerHuman::resolveAttack(int& midshipsLost, int& evasionLost) 
{
	cout << "Resolve attack on the Battle Board.\n";
	cout << "Enter midships and evasion damage from table: ";
	cin >> midshipsLost >> evasionLost;
}

// Respond to enemy search attempt
bool BritishPlayerHuman::checkSearch(const GridCoordinate& zone) {
	cout << "Opponent searches in " << zone << ".\n";
	cout << "Are ships present there (y/n)? ";
	return getUserYes();
}
