#include "Navigator.h"
#include "Utils.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <algorithm>
using namespace std;

// Structure to record path-search information
struct ZonePathRecord {
	bool inOpenSet;	
	int gScore, fScore;	
	GridCoordinate cameFrom;	
};

// Find a sea route via the A* search algorithm
//   See: https://en.wikipedia.org/wiki/A*_search_algorithm
//   Initial code from OpenAI chat suggestion.
//   We add a decimal to keys in the priority queue,
//     so as to randomly shuffle equally-close options.
std::vector<GridCoordinate> Navigator::findSeaRoute(
	const Ship& ship, const GridCoordinate& goal)
{
	// Create data structures
	typedef pair<double, GridCoordinate> rankedZone;
	priority_queue<rankedZone, vector<rankedZone>, greater<>> openSet;
	unordered_map<GridCoordinate, ZonePathRecord, GridCoordinateHash> 
		pathRecords;

	// Initialize with ship start position
	GridCoordinate start = ship.getPosition();
	int distance = start.distanceFrom(goal);
	pathRecords[start] = {true, 0, distance, GridCoordinate::NO_ZONE};
	openSet.emplace(distance + randDecimal(), start);
	
	// While we have an open edge to the search space
	while (!openSet.empty()) {

		// Get the best-guess next step
		GridCoordinate current = openSet.top().second;
		pathRecords[current].inOpenSet = false;
		openSet.pop();

		// If we've found our goal, compile route & return
		if (current == goal) {
			vector<GridCoordinate> path;
			while (current != start) {
				path.push_back(current);
				current = pathRecords[current].cameFrom;
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
			int tentative_gScore = pathRecords[current].gScore + 1;

			// Record if this is the shortest path found to neighbor
			if (pathRecords.find(neighbor) == pathRecords.end()
				|| tentative_gScore < pathRecords[neighbor].gScore) 
			{
				pathRecords[neighbor].cameFrom = current;
				pathRecords[neighbor].gScore = tentative_gScore;
				pathRecords[neighbor].fScore = tentative_gScore 
					+ neighbor.distanceFrom(goal);

				// Add new zone to open set
				if (!pathRecords[neighbor].inOpenSet) {
					openSet.emplace(pathRecords[neighbor].fScore 
						+ randDecimal(), neighbor);
					pathRecords[neighbor].inOpenSet = true;
				}
			}
		}
	}

	// Return an empty path if no path is found
	return {};
}
