/*
	Name: BritishPlayerHuman
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 08-12-24 01:00
	Description: Human player UI for the British side.
*/
#ifndef BRITISHPLAYERHUMAN_H
#define BRITISHPLAYERHUMAN_H
#include "BritishPlayerInterface.h"

class BritishPlayerHuman: public BritishPlayerInterface
{
	public:

		// Start and end game confirmation
		bool okStartGame() override;
		void okEndGame() override;

		// Present prompts		
		void promptMovement() override;
		void promptAttack() override;

		// Request intentions		
		bool trySearch() override;
		bool tryShadow(const NavalUnit& target, 
			const GridCoordinate& knownPos, 
			GameDirector::Phase phase) override;
		bool tryAttack(const NavalUnit& target, 
			GameDirector::Phase phase) override;
		bool tryDefend(const NavalUnit& target) override;
		
		// Resolve attempts
		void resolveSearch() override;
		void resolveShadow(const NavalUnit& target, 
			bool& heldContact) override;
		void resolveAttack(Ship& ship, 
			int& midshipsLost, int& evasionLost) override;
		
		// Response to enemy request
		bool checkSearch(const GridCoordinate& zone);
};

#endif
