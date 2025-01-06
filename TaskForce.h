/*
	Name: Task Force
	Copyright: 2025
	Author: Daniel R. Collins
	Date: 04-01-25 13:24
	Description: Organize collections of ships (Rule 5.4)
*/
#ifndef TASKFORCE_H
#define TASKFORCE_H
#include "Ship.h"
#include "NavalUnit.h"
#include <vector>

class TaskForce: public NavalUnit
{
	public:
		// Construction
		TaskForce(int id);
		void attach(Ship* ship);
		void detach(Ship* ship);
		void dissolve();

		// Accessors
		int getId() const;
		bool isEmpty() const;
		bool includes(Ship* ship) const;
		Ship* getFlagship() const;
		
		// Operators
		bool operator==(const TaskForce& other) const;

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
		int getSize() const override;
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
		bool wasConvoySunk(unsigned turnsAgo) const override;
		bool movedThrough(const GridCoordinate& zone) const override;

		// Mutators		
		Ship* getShip(int idx) override;
		void doMovementTurn() override;
		void setLocated() override;
		void setShadowed() override;
		void setCombated() override;
		void setConvoySunk() override;
		void setLoseMoveTurn() override;
		void setDetected() override;
		
	private:
		int identifier;
		std::vector<Ship*> shipList;
};

#endif
