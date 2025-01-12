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
#include <vector>
#include <list>
#include <set>

class GermanPlayer
{
	public:
		GermanPlayer();
		~GermanPlayer();
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
		int getStartNumShips() const;
		const Ship& getBismarck() const;

	private:
		// Enumeration
		enum MapRegion {NORTH_SEA, EAST_NORWEGIAN, WEST_NORWEGIAN, 
			DENMARK_STRAIT, WEST_ATLANTIC, EAST_ATLANTIC, 
			AZORES, BAY_OF_BISCAY, OFF_MAP};

		// Data
		int startNumShips;
		Ship* theBismarck;
		std::list<Ship> shipList;
		std::list<TaskForce> taskForceList;
		std::list<NavalUnit*> navalUnitList;
		std::set<GridCoordinate> foundShipZones;
		
		// Functions
		void checkGeneralSearch(NavalUnit* unit, int roll);
		void checkConvoyResult(NavalUnit* unit, int roll);
		void callHuffDuff(NavalUnit* unit);
		void destroyConvoy(NavalUnit* unit);
		char getGeneralSearchColumn(const GridCoordinate& zone);

		// Plotting functions
		void orderUnitsForTurn();
		void formTaskForces();
		void cleanTaskForces();
		void cleanTaskForce(TaskForce& taffy);
		void orderNewGoal(Ship& ship);
		void handleFuelEmpty(Ship& ship);
		int getNextTaskForceId();
		TaskForce* getTaskForceById(int id);
		MapRegion getRegion(const GridCoordinate& zone) const;
		std::set<GridCoordinate> getShipZones() const;

		// Plot-targeting functions
		GridCoordinate randLoiterZone(const Ship& ship) const;
		GridCoordinate randAzoresZone() const;
		GridCoordinate randCloseRowZ(const Ship& ship) const;
		GridCoordinate findNearestPort(const Ship& ship) const;
		GridCoordinate randDenmarkStraitToAfricaTransit(
			const Ship& ship) const;

		// Convoy-targeting functions
		GridCoordinate randConvoyTarget(int pctAtlantic) const;
		GridCoordinate randAtlanticConvoyTarget() const;
		GridCoordinate randAfricanConvoyTarget() const;
		GridCoordinate randConvoyTargetWeightNearby(const Ship& ship) const;
		int randWeightedConvoyDistance() const;
		
		// Test regressions
		void testStoppedUnits();
		void testMoveAfterConvoySunk();
};

#endif
