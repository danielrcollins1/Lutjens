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
		const Ship& getFlagship() const;
		void doAvailabilityPhase();
		void doShadowPhase();
		void doSeaMovementPhase();
		void checkSearch(const GridCoordinate& zone);
		void doAirAttackPhase();
		void doNavalCombatPhase();
		void doChancePhase();
		bool trySearch();
		void resolveSearch();
		void printAllShips() const;
		int getTimesFlagshipDetected() const;

	private:

		// Data
		Ship* flagship;
		std::vector<Ship> shipList;
		
		// Functions
		void initWaypoints(Ship& ship);
		void targetAtlanticConvoy(Ship& ship);
		void targetAfricanConvoy(Ship& ship);
		void targetAnyConvoyBreakout(Ship& ship);
		char generalSearchColumn(const GridCoordinate& zone);
		void checkGeneralSearch(Ship& ship, int roll);
		void checkConvoyResult(Ship& ship, int roll);
		void callHuffDuff(Ship& ship);
		void destroyConvoy(Ship& ship);
		void pickNewRoute();
};

#endif
