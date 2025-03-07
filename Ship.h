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
			GridCoordinate position = GridCoordinate::OFFBOARD,
			GermanPlayer* player = nullptr);

		// Accessors
		Type getType() const;
		GeneralType getGeneralType() const;
		std::string getGeneralTypeName() const;
		int getFuel() const;
		int getMidships() const;
		int getTimesDetected() const;

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
		void clearOrders();
		bool hasOrders() const;
		OrderType getFirstOrder() const;
		float getMaxSpeedAvg() const;
		int convoyETA() const;
		int routeETA() const;
		int rowZ_ETA() const;
		
		// Task force membership
		void joinTaskForce(TaskForce* taskForce);
		void leaveTaskForce();
		bool isInTaskForce() const;
		void followShip(Ship& ship);

		//
		// NavalUnit overrides
		//
		
		// Descriptors
		std::string getName() const override;
		std::string getTypeDesc() const override;
		std::string getNameDesc() const override;
		std::string getFullDesc() const override;
		
		// Accessors
		virtual Ship* getShip(int idx) override;
		virtual Ship* getFlagship() override;
		virtual const Ship* getFlagship() const override;
		GridCoordinate getPosition() const override;
		int getSize() const override;
		int getMaxSpeedClass() const override;
		int getMaxSpeedThisTurn() const override;
		int getSpeedThisTurn() const override;
		int getEvasion() const override;
		int getAttackEvasion() const override;
		int getSearchStrength() const override;

		// Status checks
		bool isAfloat() const override;
		bool isOnBoard() const override;
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
		bool wasConvoySunk(unsigned turnsAgo) const override;
		bool movedThrough(const GridCoordinate& zone) const override;

		// Mutators
		void doMovementTurn() override;
		void setLocated() override;
		void setShadowed() override;
		void setCombated() override;
		void setConvoySunk() override;
		void setDetected() override;
		
	private:

		// Type labels
		static const char* typeAbbr[];
		static const char* generalTypeName[];

		// Orders structure
		struct Order {
			OrderType type = OrderType::STOP;
			GridCoordinate zone = GridCoordinate::OFFBOARD;
			std::string toString() const;
		};

		// Logging structure
		struct LogTurn {
			std::vector<GridCoordinate> moves;
			bool shadowed = false, located = false, 
				combated = false, convoySunk = false;
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
		bool returnToBase;
		GridCoordinate position;
		GermanPlayer* player;
		TaskForce* taskForce;
		std::queue<Order> orders;
		std::vector<GridCoordinate> route;
		std::vector<LogTurn> log;

		// Functions
		LogTurn& logNow();
		int getEmergencySpeedThisTurn() const;
		int getFuelExpense(int speed) const;
		bool isOnBreakoutBonus() const;
		bool isAdjacent(const GridCoordinate& zone) const;
		void updateOrders();
		void doMoveOrder();
		void doPostMoveAccounts();
		void pushOrder(Order order);
		void setEvasionLossRate();
		void applyTempEvasionLoss(int midshipsLoss);
		void checkFuelDamage(int midshipsLoss);
		void checkFuelForWeather(int speed);
		void plotRoute(const GridCoordinate& goal);
};

// Stream insertion operator
std::ostream& operator<<(std::ostream& stream, const Ship& ship);

#endif
