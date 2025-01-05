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
#include <vector>

class TaskForce
{
	public:
		TaskForce ();
		int getId() const;
		void attach(Ship* ship);
		void detach(Ship* ship);
		void dissolve();
		bool includes(Ship* ship) const;
		bool isEmpty() const;
		int getEvasion() const;
		int getAttackEvasion() const;
		int getMaxSpeedClass() const;
		
	private:
		static int numMade;
		int identifier;
		std::vector<Ship*> shipList;
};

#endif
