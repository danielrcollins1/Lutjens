/*
	Name: GermanPlayer
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 27-11-24 00:11
	Description: Player for the German forces.
*/
#ifndef GERMANPLAYER_H
#define GERMANPLAYER_H
#include "Ship.h"

class GermanPlayer
{
	public:
		GermanPlayer();
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
		int getTimesFlagshipDetected() const;

	private:

		// Data
		Ship* bismarck;
		std::vector<Ship> shipList;
		
		// Functions
		void initWaypoints(Ship& ship);
		void targetAtlanticConvoy(Ship& ship);
		void targetAfricanConvoy(Ship& ship);
		void targetAnyConvoyBreakout(Ship& ship);
		char generalSearchColumn(const GridCoordinate& zone);
		void destroyConvoy();
		void pickNewRoute();
};

#endif
