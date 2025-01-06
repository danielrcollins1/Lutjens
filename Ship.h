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
#include "NavalUnit.h"
#include <vector>
#include <queue>

// Forwards
class GermanPlayer;
class TaskForce;

// Ship class
class Ship: public NavalUnit
{
	public:
		
		// Enumerations
		enum Type {BB, BC, PB, CV, CA, CL, DD, CT, SS, UB};
		enum GeneralType {BATTLESHIP, CARRIER, CRUISER, DESTROYER, SUBMARINE};
		enum OrderType {MOVE, PATROL, STOP};

		// Constructor
		Ship(std::string name, Type type, 
			int evasion, int midships, int fuel, 
			GridCoordinate position = GridCoordinate::NO_ZONE,
			GermanPlayer* player = nullptr);

		// Accessors
		Type getType() const;
		GeneralType getGeneralType() const;
		std::string getGeneralTypeName() const;
		int getFuel() const;
		int getMidships() const;
		int getSpeed() const;
		int getTimesDetected() const;
		bool isSunk() const;

		// Mutator functions
		void doAvailability();
		void setReturnToBase();
		void loseFuel(int loss);
		void loseEvasion(int loss);
		void loseMidships(int loss);
		void tryEvasionRepair();
		
		// Movement functions
		void setPosition(const GridCoordinate& zone);
		bool isAccessible(const GridCoordinate& zone) const;
		GridCoordinate randMoveInArea(int radius) const;

		// Plotting functions
		void orderAction(OrderType type);
		void orderMove(const GridCoordinate& dest);
		bool hasOrders() const;
		OrderType getFirstOrder() const;
		
		// Task force membership
		void joinTaskForce(TaskForce* taskForce);
		void leaveTaskForce();
		bool isInTaskForce() const;
		TaskForce* getTaskForce() const;
		void moveWithShip(Ship& ship);

		//
		// NavalUnit overrides
		//
		
		// Descriptors
		std::string getName() const override;
		std::string getTypeDesc() const override;
		std::string getNameDesc() const override;
		std::string getFullDesc() const override;
		
		// Accessors
		GridCoordinate getPosition() const override;
		int getMaxSpeedClass() const override;
		int getEvasion() const override;
		int getAttackEvasion() const override;

		// Status checks
		bool isInDay() const override;
		bool isInNight() const override;
		bool isInFog() const override;
		bool isInPort() const override;
		bool isEnteringPort() const override;
		bool isReturnToBase() const override;
		bool isOnPatrol() const override;
		bool wasLocated(unsigned turnsAgo) const override;
		bool wasShadowed(unsigned turnsAgo) const override;
		bool wasCombated(unsigned turnsAgo) const override;
		bool movedThrough(const GridCoordinate& zone) const override;

		// Mutators		
		void doMovementTurn() override;
		void setLocated() override;
		void setShadowed() override;
		void setCombated() override;
		void setLoseMoveTurn() override;
		void noteDetected() override;
		void clearOrders() override;
		
	private:

		// Type labels
		static const char* typeAbbr[];
		static const char* generalTypeName[];

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
