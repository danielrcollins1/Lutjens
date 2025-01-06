/*
	Name: Naval Unit
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 05-01-25 13:41
	Description: Parent class for sea units on the Search Board; 
		that is, Ships and TaskForces
*/
#ifndef NAVALUNIT_H
#define NAVALUNIT_H
#include "GridCoordinate.h"

// Forwards
class Ship;

// Class for naval units
class NavalUnit
{
	public:
		// Descriptors
		virtual std::string getName() const = 0;
		virtual std::string getTypeDesc() const = 0;
		virtual std::string getNameDesc() const = 0;
		virtual std::string getFullDesc() const = 0;
		
		// Accessors
		virtual GridCoordinate getPosition() const = 0;
		virtual int getSize() const = 0;
		virtual int getMaxSpeedClass() const = 0;
		virtual int getEvasion() const = 0;
		virtual int getAttackEvasion() const = 0;

		// Status checks
		virtual bool isInDay() const = 0;
		virtual bool isInNight() const = 0;
		virtual bool isInFog() const = 0;
		virtual bool isInPort() const = 0;
		virtual bool isEnteringPort() const = 0;
		virtual bool isReturnToBase() const = 0;
		virtual bool isOnPatrol() const = 0;
		virtual bool wasLocated(unsigned turnsAgo) const = 0;
		virtual bool wasShadowed(unsigned turnsAgo) const = 0;
		virtual bool wasCombated(unsigned turnsAgo) const = 0;
		virtual bool wasConvoySunk(unsigned turnsAgo) const = 0;
		virtual bool movedThrough(const GridCoordinate& zone) const = 0;

		// Mutators
		virtual Ship* getShip(int idx) = 0;
		virtual void doMovementTurn() = 0;
		virtual void setLocated() = 0;
		virtual void setShadowed() = 0;
		virtual void setCombated() = 0;
		virtual void setConvoySunk() = 0;
		virtual void setLoseMoveTurn() = 0;
		virtual void setDetected() = 0;
};

#endif
