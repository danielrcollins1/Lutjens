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
