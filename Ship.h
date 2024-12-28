/*
	Name: Ship
	Copyright: 2024 
	Author: Daniel R. Collins
	Date: 24-11-24 21:19
	Description: Ship object class.
*/
#ifndef SHIP_H
#define SHIP_H
#include "GridCoordinate.h"
#include "SearchBoard.h"
#include <vector>

// Forward for player
class GermanPlayer;

// Ship class
class Ship
{
	public:
		
		// Enumerations
		enum Type {BB, BC, PB, CA, CL, CV, DD, SS, NUM_TYPES};
		enum OrderType {MOVE, PATROL, LOITER};

		// Functions
		Ship(std::string name, Type type, 
			int evasion, int midships, int fuel, 
			GermanPlayer* player);
		std::string getName() const;
		std::string getTypeName() const;
		std::string getTypeAndEvasion() const;
		std::string getShortDesc() const;
		std::string getLongDesc() const;
		int getEvasion() const;
		int getMidships() const;
		int getFuel() const;
		int getSpeed() const;
		int getTimesDetected() const;
		bool isOnPatrol() const;
		bool isInPort() const;
		bool isSunk() const;
		bool isInDay() const;
		bool isInNight() const;
		bool isInFog() const;
		bool enteredPort() const;
		bool wasLocated(int turnsAgo) const;
		bool wasShadowed(int turnsAgo) const;
		bool wasInCombat(int turnsAgo) const;
		void doAvailability();
		void setLocated();
		void setShadowed();
		void setInCombat();
		void setLoseMoveTurn();
		void loseMidships(int loss);
		void loseEvasion(int loss);
		void checkEvasionRepair();
		void noteDetected();
		void clogDestination();
		
		// Movement functions
		GridCoordinate getPosition() const;
		void doMovement();
		void doBreakoutBonusMove();
		void setPosition(const GridCoordinate& zone);
		void addWaypoint(const GridCoordinate& zone);
		bool isAccessible(const GridCoordinate& zone) const;
		bool movedThrough(const GridCoordinate& zone) const;
		bool hasWaypoints() const;
		void printWaypoints() const;
		void clearWaypoints();

		// Plotting functions
		void giveOrder(OrderType type, 
			const GridCoordinate& zone = GridCoordinate::NO_ZONE);
		
	private:

		// Orders structure
		struct Order {
			OrderType type;
			GridCoordinate zone = GridCoordinate::NO_ZONE;
		};

		// Logging structure
		struct LogTurn {
			std::vector<GridCoordinate> moves;
			bool shadowed = false, located = false, battled = false;
		};

		// Data
		std::string name;
		Type type;
		bool onPatrol;
		bool loseMoveTurn;
		int fuelMax, fuelLost;
		int midshipsMax, midshipsLost;
		int evasionMax, evasionLostTemp, evasionLostPerm;
		int timesDetected;
		GridCoordinate position;
		GermanPlayer* player;
		std::vector<GridCoordinate> waypoints;
		std::vector<Order> orders;
		std::vector<LogTurn> log;

		// Constants
		static const std::string typeAbbr[NUM_TYPES];
		static const std::string typeName[NUM_TYPES];
		
		// Functions
		GridCoordinate getNextZone() const;
		void checkForWaypoint();
		int maxSpeed() const;
		void applyTempEvasionLoss(int midshipsLost);
		LogTurn& logNow();
};

// Stream insertion operator
std::ostream& operator<<(std::ostream& stream, const Ship& ship);

#endif
