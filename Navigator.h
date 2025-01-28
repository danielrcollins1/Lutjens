/*
	Name: Navigator
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 29-12-24 22:19
	Description: Provide variant A* pathfinding algorithm.
		Returns route in reverse order (goal is first element).
*/
#ifndef NAVIGATOR_H
#define NAVIGATOR_H
#include "Ship.h"
#include <vector>

class Navigator
{
	public:
		static std::vector<GridCoordinate> findSeaRoute(
			const Ship& ship, const GridCoordinate& goal);
};

#endif
