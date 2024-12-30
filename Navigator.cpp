#include "Navigator.h"
#include "Utils.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
using namespace std;

// Find a sea route via the A* search algorithm
//   See: https://en.wikipedia.org/wiki/A*_search_algorithm
//   Initial code from OpenAI chat suggestion.
//   We add a decimal to keys in the priority queue,
//     so as to randomly shuffle equally-close options.
std::vector<GridCoordinate> Navigator::findSeaRoute(
	const Ship& ship, const GridCoordinate& goal)
{
	// Create data structures
	priority_queue<
		pair<double, GridCoordinate>,
	    vector<pair<double, GridCoordinate>>,
	    greater<>> openSet;
	unordered_map<GridCoordinate, bool, GridCoordinateHash> inOpenSet;
	unordered_map<GridCoordinate, int, GridCoordinateHash> gScore;
	unordered_map<GridCoordinate, int, GridCoordinateHash> fScore;
	unordered_map<GridCoordinate, GridCoordinate, GridCoordinateHash> 
		cameFrom;

	// Initialize with ship start position
	GridCoordinate start = ship.getPosition();
	gScore[start] = 0;
	fScore[start] = start.distanceFrom(goal);
	openSet.emplace(fScore[start] + randDecimal(), start);
	inOpenSet[start] = true;

	// While we have an open edge to search space
	while (!openSet.empty()) {

		// Get the best-guess next step
		GridCoordinate current = openSet.top().second;
		openSet.pop();
		inOpenSet[current] = false;		

		// If we've found our goal, compile route & return
		if (current == goal) {
			vector<GridCoordinate> path;
			while (current != start) {
				path.push_back(current);
				current = cameFrom[current];
			}
			reverse(path.begin(), path.end());
			return path;
		}

		// Search zones adjacent to current guess
		auto adjacentList = current.getAdjacent();
		for (const auto& neighbor: adjacentList) {
			if (!ship.isAccessible(neighbor)) {
				continue;
			}

			// Compute distance from start to this neighbor
			int tentative_gScore = gScore[current] + 1;

			// Record if this is the shortest path to neighbor
			if (gScore.find(neighbor) == gScore.end() 
				|| tentative_gScore < gScore[neighbor]) 
			{
				cameFrom[neighbor] = current;
				gScore[neighbor] = tentative_gScore;
				fScore[neighbor] = gScore[neighbor] 
					+ neighbor.distanceFrom(goal);
				if (!inOpenSet[neighbor]) {
					openSet.emplace(
						fScore[neighbor] + randDecimal(), neighbor);
					inOpenSet[neighbor] = true;
				}
			}
		}
	}

	// Return an empty path if no path is found
	return {};
}
