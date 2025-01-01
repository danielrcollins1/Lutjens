/*
	Name: GameDirector
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 27-11-24 00:04
	Description: Director for Bismarck game mechanics.
		Singleton class pattern.
*/
#ifndef GAMEDIRECTOR_H
#define GAMEDIRECTOR_H
#include "GridCoordinate.h"
#include "Ship.h"
#include <vector>

// Forward to player interfaces
class BritishPlayerInterface;
class GermanPlayer;

// Class for game control
class GameDirector
{
	public:
		// Constants
		static const int START_TURN = 4;
		static const int FINISH_TURN = 34;

		// Enumeration
		enum Phase {AVAILABILITY, VISIBILITY, 
			SHADOW, AIR_MOVEMENT, SHIP_MOVEMENT, 
			SEARCH, AIR_ATTACK, NAVAL_COMBAT, CHANCE};
		
		// Functions
		static GameDirector* instance();
		static void initGame();
		bool okPlayerStart();
		void okPlayerEnd();
		void doGameLoop();
		void doEndGame();
		int getTurn() const;
		int getTurnsElapsed() const;
		int getVisibility() const;
		bool isVisibilityX() const;
		bool isConvoyTurn() const;
		bool isInDay(const GridCoordinate& zone) const;
		bool isInNight(const GridCoordinate& zone) const;
		bool isInFog(const GridCoordinate& zone) const;
		bool isSearchable(const GridCoordinate& zone, int strength) const;
		bool searchGermanShips(const GridCoordinate& zone);
		bool searchBritishShips(const GridCoordinate& zone);
		void checkShadow(Ship& target, 
			const GridCoordinate& knownPos, Phase phase);
		void checkAttackBy(Ship& attacker);
		void checkAttackOn(Ship& target, Phase phase);
		void resolveCombat(Ship& ship);
		void msgSunkConvoy();
		bool isPassThroughSearchOn() const;
		bool wasConvoySunk(unsigned daysAgo) const;
		int getConvoysSunk() const;
		int getTimesFlagshipDetected() const;

	private:
		// Constant
		static const int VISIBILITY_X = 9;

		// Data
		static GameDirector* theInstance;
		GermanPlayer* germanPlayer = nullptr;
		BritishPlayerInterface* britishPlayer = nullptr;
		std::vector<bool> dailyConvoySunk;
		int turn = START_TURN;
		int finishTurn = FINISH_TURN;
		int visibility = 4;
		bool foggy = true;
		
		// Functions
		GameDirector();
		~GameDirector();
		void logStartTime();
		void checkNewDay();
		void rollVisibility();
		bool isGameOver() const;
		void reportNightTime();
		
		// Turn phase handlers (Rule 4.0)
		void doAvailabilityPhase();
		void doVisibilityPhase();
		void doShadowPhase();
		void doSeaMovementPhase();
		void doSearchPhase();
		void doAirAttackPhase();
		void doNavalCombatPhase();
		void doChancePhase();
};

#endif
