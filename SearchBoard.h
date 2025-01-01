/*
	Name: SearchBoard
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 23-11-24 21:25
	Description: Class to represent the Bismarck Search Board.
		Singleton class pattern.
*/
#ifndef SEARCHBOARD_H
#define SEARCHBOARD_H
#include "GridCoordinate.h"
#include "SearchBoardLayer.h"

class SearchBoard
{
	public:
		static SearchBoard* instance();
		bool isSeaZone(const GridCoordinate& zone) const;
		bool isBritishCoast(const GridCoordinate& zone) const;
		bool isBritishPort(const GridCoordinate& zone) const;
		bool isGermanPort(const GridCoordinate& zone) const;
		bool isFogZone(const GridCoordinate& zone) const;
		bool isIrishSea(const GridCoordinate& zone) const;
		bool isBritishPatrol(const GridCoordinate& zone) const;
		bool isConvoyRoute(const GridCoordinate& zone) const;
		bool isNearZoneType(const GridCoordinate& zone, 
			int distance, bool (SearchBoard::*zoneType)
				(const GridCoordinate& zone) const) const;
		GridCoordinate randSeaZone(const GridCoordinate& center, 
			int radius) const;
		std::vector<GridCoordinate> getAllGermanPorts() const;
		int getPatrolLimitForRow(char row) const;
		void print() const;

	private:
		SearchBoard();
		static SearchBoard* theInstance;
		enum Layers {SeaZones, BritishCoast, BritishPorts, GermanPorts,
			FogZones, IrishSea, BritishPatrol, ConvoyRoutes, NUM_LAYERS};
		SearchBoardLayer layers[NUM_LAYERS] = {
			SearchBoardLayer("SearchBoard-SeaZones.csv"),
			SearchBoardLayer("SearchBoard-BritishCoast.csv"),
			SearchBoardLayer("SearchBoard-BritishPorts.csv"),
			SearchBoardLayer("SearchBoard-GermanPorts.csv"),
			SearchBoardLayer("SearchBoard-FogZones.csv"),
			SearchBoardLayer("SearchBoard-IrishSea.csv"),
			SearchBoardLayer("SearchBoard-BritishPatrol.csv"),
			SearchBoardLayer("SearchBoard-ConvoyRoutes.csv")
		};
};

#endif