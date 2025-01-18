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
		int getStartTurn() const;
		int getFinishTurn() const;
		int getTurnsElapsed() const;
		int getVisibility() const;
		bool isVisibilityX() const;
		bool isStartTurn() const;
		bool isConvoyTurn() const;
		bool isInDay(const GridCoordinate& zone) const;
		bool isInNight(const GridCoordinate& zone) const;
		bool isInFog(const GridCoordinate& zone) const;
		bool isSearchable(const GridCoordinate& zone, int strength) const;
		bool searchGermanShips(const GridCoordinate& zone);
		bool searchBritishShips(const GridCoordinate& zone);
		void checkShadow(NavalUnit& target, 
			const GridCoordinate& knownPos, Phase phase);
		void checkAttackBy(NavalUnit& attacker);
		void checkAttackOn(NavalUnit& target, Phase phase);
		void resolveCombat(NavalUnit& unit);
		void msgSunkConvoy();
		bool wasConvoySunk(unsigned daysAgo) const;
		int getConvoysSunk() const;
		const Ship& getBismarck() const;

	private:
		// Constants
		static const int BASIC_START_TURN = 4;
		static const int BASIC_FINISH_TURN = 34;
		static const int VISIBILITY_X = 9;

		// Data
		static GameDirector* theInstance;
		GermanPlayer* germanPlayer = nullptr;
		BritishPlayerInterface* britishPlayer = nullptr;
		std::vector<bool> dailyConvoySunk;
		int turn = BASIC_START_TURN;
		int finishTurn = BASIC_FINISH_TURN;
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
		void doShipMovementPhase();
		void doSearchPhase();
		void doAirAttackPhase();
		void doNavalCombatPhase();
		void doChancePhase();
};

#endif
