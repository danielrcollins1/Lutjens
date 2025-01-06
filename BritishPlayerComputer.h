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
#include <vector>

class BritishPlayerComputer: public BritishPlayerInterface
{
	public:
		BritishPlayerComputer();

		// Start and end game confirmation
		bool okStartGame() override { return true; }
		void okEndGame() override {}
		
		// Present prompts		
		void promptMovement() override {}
		void promptAttack() override {}
		
		// Request intentions		
		bool trySearch() override;
		bool tryShadow(const NavalUnit& target, 
			const GridCoordinate& knownPos, 
			GameDirector::Phase phase) override
			{ return false; }
		bool tryAttack(const NavalUnit& target, 
			GameDirector::Phase phase) override
			{ return false; }
		bool tryDefend(const NavalUnit& target) override
			{ return false; }
		
		// Resolve attempts
		void resolveSearch() override;
		void resolveShadow(const NavalUnit& target, 
			bool& heldContact) override {}
		void resolveAttack(Ship& ship, 
			int& midshipsLost, int& evasionLost) override {}
		
		// Response to enemy request
		bool checkSearch(const GridCoordinate& zone) override
			{ return false; }
		
	private:
		std::vector<int> initialAirPatrols;
		std::vector<GridCoordinate> coastalFreeSearchList;
		void searchZones(const std::vector<GridCoordinate>& zones,
			int dayStrength, int nightStrength);
		std::vector<GridCoordinate> getCoastalFreeSearchZones();
		std::vector<GridCoordinate> getShipPatrolZones();
		std::vector<GridCoordinate> getAirPatrolZones();		
		std::vector<int> planInitialAirPatrols();
		GridCoordinate pickAirPatrolZone();
};

#endif
