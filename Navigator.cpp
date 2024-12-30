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
//   Our version adds random variation among equal-distance options.
std::vector<GridCoordinate> Navigator::findSeaRoute(
	const Ship& ship, const GridCoordinate& goal)
{
	// Create data structures
	priority_queue<
		pair<double, GridCoordinate>,
	    vector<pair<double, GridCoordinate>>,
	    greater<>> openSet;
	unordered_map<GridCoordinate, bool, GridCoordinateHash> inOpenSet;
	unordered_map<GridCoordinate, double, GridCoordinateHash> gScore;
	unordered_map<GridCoordinate, double, GridCoordinateHash> fScore;
	unordered_map<GridCoordinate, GridCoordinate, GridCoordinateHash> 
		cameFrom;

	// Initialize with ship start position
	GridCoordinate start = ship.getPosition();
	gScore[start] = 0;
	fScore[start] = start.distanceFrom(goal);
	openSet.emplace(fScore[start], start);
	inOpenSet[start] = true;

	// While we have an open edge to search space
	while (!openSet.empty()) {

		// Get all the best-guess zones on edge of search space
		vector<GridCoordinate> bestGuessEdge;
		int fScoreBest = openSet.top().first;
		while (!openSet.empty() 
			&& openSet.top().first == fScoreBest) 
		{
			bestGuessEdge.push_back(openSet.top().second);
			openSet.pop();			
		}
		
		// Randomly pick one of those zones, put the rest back
		GridCoordinate current = randomElem(bestGuessEdge);
		inOpenSet[current] = false;
		for (auto zone: bestGuessEdge) {
			if (zone != current) {
				openSet.emplace(fScoreBest, zone);
			}
		}

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
			double tentative_gScore = gScore[current] + 1;

			// Record if this is the shortest path to neighbor
			if (gScore.find(neighbor) == gScore.end() 
				|| tentative_gScore < gScore[neighbor]) 
			{
				cameFrom[neighbor] = current;
				gScore[neighbor] = tentative_gScore;
				fScore[neighbor] = gScore[neighbor] 
					+ neighbor.distanceFrom(goal);
				if (!inOpenSet[neighbor]) {
					openSet.emplace(fScore[neighbor], neighbor);
					inOpenSet[neighbor] = true;
				}
			}
		}
	}

	// Return an empty path if no path is found
	return {};
}
