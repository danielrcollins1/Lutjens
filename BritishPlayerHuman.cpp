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
bool BritishPlayerHuman::tryShadow(const NavalUnit& target, 
	const GridCoordinate& knownPos, GameDirector::Phase phase)
{
	assert(phase == GameDirector::Phase::SHADOW 
		|| phase == GameDirector::Phase::SEARCH);
	if (phase == GameDirector::Phase::SHADOW) {
		cout << target.getTypeDesc() << " was seen in " << knownPos << "\n";
		cout << "Evasion level is " << target.getEvasion() << "\n";
	}
	cout << "Do you wish to shadow (y/n)? ";
	return getUserYes();
}

// Ask if we want to try attacking
bool BritishPlayerHuman::tryAttack(
	const NavalUnit& target, GameDirector::Phase phase) 
{
	assert(phase == GameDirector::Phase::AIR_ATTACK 
		|| phase == GameDirector::Phase::NAVAL_COMBAT);
	cout << target.getTypeDesc() << " is seen in " 
		<< target.getPosition() << "\n";
	if (phase == GameDirector::Phase::NAVAL_COMBAT) {
		cout << "Evasion rating is " << target.getEvasion() << "\n";		
	}
	cout << "Do you wish to attack by " 
		<< (phase == GameDirector::Phase::AIR_ATTACK ? "air" : "sea") 
		<< " (y/n)? ";
	return getUserYes();
}

// Ask if there is a desired target for Germans to attack
bool BritishPlayerHuman::tryDefend(const NavalUnit& target) {
	cout << "In " << target.getPosition() 
		<< " is there a solo cruiser with evasion no more than " 
		<< target.getAttackEvasion()
		<< " (y/n)? ";
	return getUserYes();
}

// Prompt for an attak resolution
void BritishPlayerHuman::promptAttack() {
	cout << "Resolve attack on the Battle Board.\n";
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
void BritishPlayerHuman::resolveShadow(
	const NavalUnit& target, bool& heldContact) 
{
	string targetID = target.getTypeDesc();	
	if (target.getSpeedThisTurn() > 1) {
		cout << targetID << " moves at high speed (apply +1).\n";
	}
	cout << "Resolve attempt on the Shadow Table.\n";
	cout << "Did shadow attempt hold contact (y/n)? ";
	heldContact = getUserYes();
}

// Resolve attempt to attack on a ship
void BritishPlayerHuman::resolveAttack(Ship& ship, 
	int& midshipsLost, int& evasionLost)
{
	cout << "Enter " << ship.getName() << " midships and evasion damage: ";
	cin >> midshipsLost >> evasionLost;
}

// Respond to enemy search attempt
bool BritishPlayerHuman::checkSearch(const GridCoordinate& zone) {
	cout << "Opponent searches in " << zone << ".\n";
	cout << "Are ships present there (y/n)? ";
	return getUserYes();
}
