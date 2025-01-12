/*
	Name: Task Force
	Copyright: 2025
	Author: Daniel R. Collins
	Date: 04-01-25 13:24
	Description: Collections of Ships (per Rule 5.4).
	
		Most NavalUnit functions here pass through to the associated
		Ship function. Accessors usually check the accessor of the
		first ship (the flagship). Mutators set the values for all ships.

		Orders and routing for movement are done only for the flagship
		(via callbacks to its player), while other ships simply copy
		the same movement. See doMovementTurn() for details.
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
		~TaskForce();

		// Accessors
		int getId() const;
		bool isEmpty() const;
		bool includes(Ship* ship) const;
		Ship* getFlagship() const;
		
		// Mutator
		void orderFollowers(Ship::OrderType order);
		
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
		Ship* getCommand() override;
		Ship* getShip(int idx) override;
		void doMovementTurn() override;
		void setLocated() override;
		void setShadowed() override;
		void setCombated() override;
		void setConvoySunk() override;
		void setDetected() override;
		
	private:
		int identifier;
		std::vector<Ship*> shipList;
};

#endif
