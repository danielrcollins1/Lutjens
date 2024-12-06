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
using namespace std;

class Ship
{
	public:
		enum Type {BB, BC, PB, CA, CL, CV, DD, SS, NUM_TYPES};
		Ship(string name, Type type, int evasion, int midships, int fuel);
		string getName() const;
		string getTypeName() const;
		string getTypeAndEvasion() const;
		string getShortDesc() const;
		string getLongDesc() const;
		int getEvasion() const;
		int getMidships() const;
		int getFuel() const;
		int getSpeed() const;
		bool isOnPatrol() const;
		bool isInPort() const;
		bool isSunk() const;
		bool isInNight() const;
		bool isInFog() const;
		bool enteredPort() const;
		void doAvailability();
		void setExposed();
		bool wasExposed(int turnsAgo) const;
		void setLoseMoveTurn();
		void loseMidships(int loss);
		void loseEvasion(int loss);
		void checkEvasionRepair();
		
		// Movement functions
		void doMovement();
		void setPosition(const GridCoordinate& zone);
		void addWaypoint(const GridCoordinate& zone);
		bool isAccessible(const GridCoordinate& zone) const;
		bool movedThrough(const GridCoordinate& zone) const;
		GridCoordinate getPosition() const;
		void clearWaypoints();
		
	private:

		// Data
		string name;
		Type type;
		bool onPatrol;
		bool tookMoveTurn;
		bool loseMoveTurn;
		int fuelMax, fuelLost;
		int midshipsMax, midshipsLost;
		int evasionMax, evasionLostTemp, evasionLostPerm;
		GridCoordinate position;
		vector<vector<GridCoordinate>> moveHistory;
		vector<GridCoordinate> waypoints;
		vector<bool> exposureHistory;

		// Constants
		static const string typeAbbr[NUM_TYPES];
		static const string typeName[NUM_TYPES];
		
		// Functions
		GridCoordinate getNextZone() const;
		void checkForWaypoint();
		int maxSpeed() const;
		void applyTempEvasionLoss(int midshipsLost);
};

#endif