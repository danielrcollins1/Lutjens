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
		static GameDirector* instance();
		int getTurn() const;
		int getVisibility() const;
		void doGameLoop();
		void doEndGame();
		void msgSunkConvoy();
		bool isInNight(const GridCoordinate& zone) const;
		bool isInFog(const GridCoordinate& zone) const;
		void checkSearch(const GridCoordinate& zone);
		void checkShadow(Ship& target,
			const GridCoordinate& knownPos, bool inSearchPhase);
		void checkAttack(Ship& target, bool inAirPhase);
		bool isPassThroughSearchOn() const;

	private:

		// Data
		static GameDirector* theInstance;
		BritishPlayerInterface* britishPlayer;
		GermanPlayer* germanPlayer;
		int turn;
		int visibility;
		bool foggy;
		int convoysSunk;
		bool convoySunkToday;
		
		// Constants
		static const int FIRST_TURN = 1;
		static const int GERMAN_START_TURN = 2;
		static const int BRITISH_START_TURN = 4;
		static const int FINISH_TURN = 34;
		static const int START_VISIBILITY = 4;
		static const bool START_FOG = true;

		// Enumeration
		enum NightState {NIGHT_SOUTH, NIGHT_ALL, DAY};
		
		// Functions
		GameDirector();
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
