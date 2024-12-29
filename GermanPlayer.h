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
		void doAirAttackPhase();
		void doNavalCombatPhase();
		void doChancePhase();
		bool checkSearch(const GridCoordinate& zone);
		bool trySearch();
		void resolveSearch();
		void printAllShips() const;
		void getDirection(Ship& ship);
		int getTimesFlagshipDetected() const;

	private:
		// Enumeration
		enum MapRegion {NORTH_SEA, EAST_NORWEGIAN, WEST_NORWEGIAN, 
			DENMARK_STRAIT, WEST_ATLANTIC, EAST_ATLANTIC};

		// Data
		Ship* flagship;
		std::vector<Ship> shipList;
		std::vector<GridCoordinate> foundShipZones;
		
		// Functions
		GridCoordinate randAtlanticConvoyTarget() const;
		GridCoordinate randAfricanConvoyTarget() const;
		GridCoordinate randMidAtlanticTarget() const;
		GridCoordinate randAnyConvoyTarget(Ship& ship) const;
		void checkGeneralSearch(Ship& ship, int roll);
		void checkConvoyResult(Ship& ship, int roll);
		void callHuffDuff(Ship& ship);
		void destroyConvoy(Ship& ship);
		char getGeneralSearchColumn(const GridCoordinate& zone);

		// Plotting functions
		MapRegion getRegion(const GridCoordinate& zone) const;
		GridCoordinate loiterTarget(const Ship& ship) const;
};

#endif
