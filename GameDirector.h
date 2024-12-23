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
#include "BritishPlayerInterface.h"
#include "GermanPlayer.h"

class GameDirector
{
	public:
		// Constants
		static const int START_TURN = 4;
		static const int FINISH_TURN = 34;

		// Functions
		static GameDirector* instance();
		static void initGame();
		bool okPlayerStart();
		void okPlayerEnd();
		void doGameLoop();
		void doEndGame();
		int getTurn() const;
		int getVisibility() const;
		bool isInNight(const GridCoordinate& zone) const;
		bool isInFog(const GridCoordinate& zone) const;
		void checkSearch(const GridCoordinate& zone);
		void checkShadow(Ship& target,
			const GridCoordinate& knownPos, bool inSearchPhase);
		void checkAttack(Ship& target, bool inAirPhase);
		bool isPassThroughSearchOn() const;
		void msgSunkConvoy();
		int getConvoysSunk() const;
		int getTimesFlagshipDetected() const;

	private:
		// Enumeration
		enum NightState {NIGHT_SOUTH, NIGHT_ALL, DAY};

		// Data
		static GameDirector* theInstance;
		GermanPlayer* germanPlayer = nullptr;
		BritishPlayerInterface* britishPlayer = nullptr;
		int turn = START_TURN;
		int finishTurn = FINISH_TURN;
		int visibility = 4;
		bool foggy = true;
		int convoysSunk = 0;
		bool convoySunkToday = false;
		
		// Functions
		GameDirector();
		~GameDirector();
		void logStartTime();
		void checkNewDay();
		void rollVisibility();
		bool isGameOver() const;
		void reportNightState();
		NightState getNightState() const;
		
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
