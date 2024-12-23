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

class Ship
{
	public:
		enum Type {BB, BC, PB, CA, CL, CV, DD, SS, NUM_TYPES};
		Ship(std::string name, Type type, 
			int evasion, int midships, int fuel);
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
		void doAvailability();
		void setLocated();
		void setShadowed();
		void setLoseMoveTurn();
		void loseMidships(int loss);
		void loseEvasion(int loss);
		void checkEvasionRepair();
		void noteDetected();
		
		// Movement functions
		GridCoordinate getPosition() const;
		void doMovement();
		void setPosition(const GridCoordinate& zone);
		void addWaypoint(const GridCoordinate& zone);
		bool isAccessible(const GridCoordinate& zone) const;
		bool movedThrough(const GridCoordinate& zone) const;
		bool hasWaypoints() const;
		void printWaypoints() const;
		void clearWaypoints();
		
	private:

		// Data
		std::string name;
		Type type;
		bool onPatrol;
		bool tookMoveTurn;
		bool loseMoveTurn;
		int fuelMax, fuelLost;
		int midshipsMax, midshipsLost;
		int evasionMax, evasionLostTemp, evasionLostPerm;
		int timesDetected;
		GridCoordinate position;
		std::vector<std::vector<GridCoordinate>> moveHistory;
		std::vector<GridCoordinate> waypoints;
		std::vector<bool> shadowedHistory;
		std::vector<bool> locatedHistory;

		// Constants
		static const std::string typeAbbr[NUM_TYPES];
		static const std::string typeName[NUM_TYPES];
		
		// Functions
		GridCoordinate getNextZone() const;
		void checkForWaypoint();
		int maxSpeed() const;
		int startGameSpeedCap();
		void applyTempEvasionLoss(int midshipsLost);
		void doBreakoutBonusMove();
};

// Stream insertion operator
std::ostream& operator<<(std::ostream& stream, const Ship& ship);

#endif
