/*
	Name: PlayerGerman
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 27-11-24 00:11
	Description: Plater for the German forces.
*/

#ifndef PLAYERGERMAN_H
#define PLAYERGERMAN_H
#include "Ship.h"

class PlayerGerman
{
	public:
		PlayerGerman();
		const Ship& getBismarck() const;
		void doAvailabilityPhase();
		void doShadowPhase();
		void doSeaMovementPhase();
		void checkSearch(const GridCoordinate& zone);
		void doAirAttackPhase();
		void doNavalCombatPhase();
		void callHuffDuff();
		void checkGeneralSearch(int roll);
		void checkConvoyResult(int roll);
		void printAllShips() const;

	private:

		// Data
		Ship* bismarck;
		std::vector<Ship> shipList;
		
		// Functions
		void initWaypoints(Ship& ship);
		GridCoordinate convoyTargetAtlantic() const;
		GridCoordinate convoyTargetAfrican() const;
		void pickConvoyTarget(Ship& ship, int chanceAtlantic);
		char generalSearchColumn(const GridCoordinate& zone);
		void destroyConvoy();
		void pickNewRoute();
};

#endif