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
#include <vector>
#include <queue>

// Forwards
class GermanPlayer;
class TaskForce;

// Ship class
class Ship
{
	public:
		
		// Enumerations
		enum Type {BB, BC, PB, CV, CA, CL, DD, CT, SS, UB};
		enum ClassType {BATTLESHIP, CRUISER, DESTROYER, SUBMARINE};
		enum OrderType {MOVE, PATROL, STOP};

		// Constructor
		Ship(std::string name, Type type, 
			int evasion, int midships, int fuel, 
			GridCoordinate position = GridCoordinate::NO_ZONE,
			GermanPlayer* player = nullptr);

		// Descriptors
		std::string getName() const;
		std::string getTypeName() const;
		std::string getTypeAndEvasion() const;
		std::string getShortDesc() const;
		std::string getLongDesc() const;

		// Accessors
		Type getType() const;
		ClassType getClassType() const;
		int getFuel() const;
		int getEvasion() const;
		int getMidships() const;
		int getSpeed() const;
		int getTimesDetected() const;
		GridCoordinate getPosition() const;

		// Status checks		
		bool isSunk() const;
		bool isOnPatrol() const;
		bool isInPort() const;
		bool isInDay() const;
		bool isInNight() const;
		bool isInFog() const;
		bool isReturnToBase() const;
		bool isEnteringPort() const;
		bool wasLocated(unsigned turnsAgo) const;
		bool wasShadowed(unsigned turnsAgo) const;
		bool wasCombated(unsigned turnsAgo) const;

		// Mutator functions
		void doAvailability();
		void setLocated();
		void setShadowed();
		void setInCombat();
		void setLoseMoveTurn();
		void setReturnToBase();
		void loseFuel(int loss);
		void loseEvasion(int loss);
		void loseMidships(int loss);
		void tryEvasionRepair();
		void noteDetected();
		
		// Movement functions
		void doMovement();
		void setPosition(const GridCoordinate& zone);
		bool isAccessible(const GridCoordinate& zone) const;
		bool movedThrough(const GridCoordinate& zone) const;
		GridCoordinate randMoveInArea(int radius) const;
		int getMaxSpeedClass() const;

		// Plotting functions
		void orderAction(OrderType type);
		void orderMove(const GridCoordinate& dest);
		bool hasOrders() const;
		OrderType getFirstOrder() const;
		void clearOrders();
		
		// Task force membership
		void joinTaskForce(TaskForce* taskForce);
		void leaveTaskForce();
		bool isInTaskForce() const;
		bool isTaskForceFlagship() const;
		bool isTaskForceEscort() const;
		TaskForce* getTaskForce() const;
		void moveWithTaskForce();
		
	private:

		// Type labels
		static const char* typeAbbr[];
		static const char* typeName[];

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

		// Data
		std::string name;
		Type type;
		int fuelMax, fuelLost;
		int midshipsMax, midshipsLost;
		int evasionMax, evasionLostTemp, evasionLostPerm;
		int evasionLossRate;
		int timesDetected;
		bool onPatrol;
		bool loseMoveTurn;
		bool returnToBase;
		GridCoordinate position;
		GermanPlayer* player;
		TaskForce* taskForce;
		std::queue<Order> orders;
		std::queue<GridCoordinate> route;
		std::vector<LogTurn> log;

		// Functions
		LogTurn& logNow();
		int getMaxSpeedThisTurn() const;
		int getEmergencySpeedThisTurn() const;
		int getFuelExpense(int speed) const;
		bool isOnBreakoutBonus() const;
		void updateOrders();
		void doMoveOrder();
		void doPostMoveAccounts();
		void pushOrder(Order order);
		void setEvasionLossRate();
		void applyTempEvasionLoss(int midshipsLoss);
		void checkFuelDamage(int midshipsLoss);
		void checkFuelForWeather(int speed);
		void plotRoute(const GridCoordinate& goal);
		void clearRoute();
};

// Stream insertion operator
std::ostream& operator<<(std::ostream& stream, const Ship& ship);

#endif
