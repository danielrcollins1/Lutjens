/*
	Name: BritishPlayerComputer
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 08-12-24 00:29
	Description: Computer player for the British side.
*/
#ifndef BRITISHPLAYERCOMPUTER_H
#define BRITISHPLAYERCOMPUTER_H
#include "BritishPlayerInterface.h"

class BritishPlayerComputer: public BritishPlayerInterface
{
	public:

		// Start and end game confirmation
		virtual bool okStartGame() override { return true; }
		virtual void okEndGame() override {}
		
		// Request intentions		
		bool trySearch() override;
		bool tryShadow(const Ship& target, 
			const GridCoordinate& knownPos, bool inSearchPhase) override
			{ return false; }
		bool tryAttack(const Ship& target, bool inSeaPhase) override 
			{ return false; }
		
		// Resolve attempts
		void resolveSearch() override;
		void resolveShadow(const Ship& target, bool& heldContact) override {}
		void resolveAttack(int& midshipsLost, int& evasionLost) override {}
		
	private:
		void doFreeCoastalSearch();
		bool isFreeSearchable(const GridCoordinate& zone);
};

#endif
