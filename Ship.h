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
#include <queue>

// Forward for player
class GermanPlayer;

// Ship class
class Ship
{
	public:
		
		// Enumerations
		enum Type {BB, BC, PB, CA, CL, CV, DD, SS, NUM_TYPES};
		enum ClassType {BATTLESHIP, CRUISER, DESTROYER, OTHER};
		enum OrderType {MOVE, PATROL, STOP};

		// Functions
		Ship(std::string name, Type type, 
			int evasion, int midships, int fuel, 
			GermanPlayer* player);
		std::string getName() const;
		std::string getTypeName() const;
		std::string getTypeAndEvasion() const;
		std::string getShortDesc() const;
		std::string getLongDesc() const;
		ClassType getClassType() const;
		int getFuel() const;
		int getEvasion() const;
		int getMidships() const;
		int getSpeed() const;
		int getTimesDetected() const;
		bool isOnPatrol() const;
		bool isInPort() const;
		bool isSunk() const;
		bool isInDay() const;
		bool isInNight() const;
		bool isInFog() const;
		bool enteredPort() const;
		bool wasLocated(unsigned turnsAgo) const;
		bool wasShadowed(unsigned turnsAgo) const;
		bool wasCombated(unsigned turnsAgo) const;
		void doAvailability();
		void setLocated();
		void setShadowed();
		void setInCombat();
		void setLoseMoveTurn();
		void setRobustEvasion();
		void loseFuel(int loss);
		void loseEvasion(int loss);
		void loseMidships(int loss);
		void checkEvasionRepair();
		void noteDetected();
		
		// Movement functions
		GridCoordinate getPosition() const;
		void doMovement();
		void doBreakoutBonusMove();
		void setPosition(const GridCoordinate& zone);
		bool isAccessible(const GridCoordinate& zone) const;
		bool movedThrough(const GridCoordinate& zone) const;
		GridCoordinate randAdjacentMove() const;

		// Plotting functions
		void orderAction(OrderType type);
		void orderMove(const GridCoordinate& dest);
		bool hasOrders() const;
		OrderType getFirstOrder() const;
		void clearOrders();
		
	private:

		// Orders structure
		struct Order {
			OrderType type;
			GridCoordinate zone = GridCoordinate::NO_ZONE;
			std::string toString() const;
		};

		// Logging structure
		struct LogTurn {
			std::vector<GridCoordinate> moves;
			bool shadowed = false, located = false, combated = false;
		};

		// Constants
		static const std::string typeAbbr[NUM_TYPES];
		static const std::string typeName[NUM_TYPES];
		
		// Data
		std::string name;
		Type type;
		bool onPatrol;
		bool loseMoveTurn;
		bool robustEvasion;
		int fuelMax, fuelLost;
		int midshipsMax, midshipsLost;
		int evasionMax, evasionLostTemp, evasionLostPerm;
		int timesDetected;
		GridCoordinate position;
		GermanPlayer* player;
		std::queue<Order> orders;
		std::queue<GridCoordinate> route;
		std::vector<LogTurn> log;

		// Functions
		LogTurn& logNow();
		int maxSpeed() const;
		int getFuelExpense(int speed) const;
		void updateOrders();
		void doMoveOrder();
		void pushOrder(Order order);
		void applyTempEvasionLoss(int midshipsLoss);
		void checkFuelDamage(int midshipsLoss);
		void checkFuelForWeather(int speed);
		void plotRoute(const GridCoordinate& goal);
		void clearRoute();
};

// Stream insertion operator
std::ostream& operator<<(std::ostream& stream, const Ship& ship);

#endif
