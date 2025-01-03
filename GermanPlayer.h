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
		void getOrders(Ship& ship);
		int getTimesFlagshipDetected() const;

	private:
		// Enumeration
		enum MapRegion {NORTH_SEA, EAST_NORWEGIAN, WEST_NORWEGIAN, 
			DENMARK_STRAIT, WEST_ATLANTIC, EAST_ATLANTIC, AZORES, OFF_MAP};

		// Data
		Ship* flagship;
		std::vector<Ship> shipList;
		std::vector<GridCoordinate> foundShipZones;
		
		// Functions
		void checkGeneralSearch(Ship& ship, int roll);
		void checkConvoyResult(Ship& ship, int roll);
		void callHuffDuff(Ship& ship);
		void destroyConvoy(Ship& ship);
		char getGeneralSearchColumn(const GridCoordinate& zone);
		void handleFuelEmpty(Ship& ship);

		// Plotting functions
		void orderNewGoal(Ship& ship);
		MapRegion getRegion(const GridCoordinate& zone) const;
		int randWeightedConvoyDistance() const;
		GridCoordinate randConvoyTarget(int pctAtlantic) const;
		GridCoordinate randAtlanticConvoyTarget() const;
		GridCoordinate randAfricanConvoyTarget() const;
		GridCoordinate randLoiterZone(const Ship& ship) const;
		GridCoordinate randDenmarkStraitToAfricaTransit(
			const Ship& ship) const;
		GridCoordinate randConvoyTargetWeightNearby(const Ship& ship) const;
		GridCoordinate randAzoresZone() const;
		GridCoordinate findNearestPort(const Ship& ship) const;
};

#endif
