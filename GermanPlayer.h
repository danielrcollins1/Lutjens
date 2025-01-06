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
#include "TaskForce.h"

class GermanPlayer
{
	public:
		GermanPlayer();
		void doAvailabilityPhase();
		void doVisibilityPhase();
		void doShadowPhase();
		void doShipMovementPhase();
		void doAirAttackPhase();
		void doNavalCombatPhase();
		void doChancePhase();
		bool checkSearch(const GridCoordinate& zone);
		bool trySearch();
		void resolveSearch();
		void printAllShips() const;
		void getOrders(Ship& ship);
		int getTimesFlagshipDetected() const;
		const Ship& getApexShip() const;

	private:
		// Enumeration
		enum MapRegion {NORTH_SEA, EAST_NORWEGIAN, WEST_NORWEGIAN, 
			DENMARK_STRAIT, WEST_ATLANTIC, EAST_ATLANTIC, AZORES, OFF_MAP};

		// Data
		Ship* apexShip;
		std::vector<Ship> shipList;
		std::vector<TaskForce> taskForceList;
		std::vector<NavalUnit*> navalUnitList;
		std::vector<GridCoordinate> foundShipZones;
		
		// Functions
		void checkGeneralSearch(NavalUnit* unit, int roll);
		void checkConvoyResult(NavalUnit* unit, int roll);
		void callHuffDuff(NavalUnit* unit);
		void destroyConvoy(NavalUnit* unit);
		char getGeneralSearchColumn(const GridCoordinate& zone);

		// Plotting functions
		void orderUnitsForTurn();
		void cleanTaskForce(TaskForce& taffy);
		void orderNewGoal(Ship& ship);
		void handleFuelEmpty(Ship& ship);
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
