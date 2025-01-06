/*
	Name: BritishPlayerInterface
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 08-12-24 00:07
	Description: Abstract class to represent a British player type.
*/
#ifndef BRITISHPLAYERINTERFACE_H
#define BRITISHPLAYERINTERFACE_H
#include "GameDirector.h"
#include "GridCoordinate.h"
#include "NavalUnit.h"

// Abstract base class for British players
class BritishPlayerInterface 
{
	public:
		virtual ~BritishPlayerInterface() {}
		
		// Start and end game confirmation
		virtual bool okStartGame() = 0;
		virtual void okEndGame() = 0;
		
		// Request intentions
		virtual bool trySearch() = 0;
		virtual bool tryShadow(const NavalUnit& target, 
			const GridCoordinate& knownPos, 
			GameDirector::Phase phase) = 0;
		virtual bool tryAttack(const Ship& target, 
			GameDirector::Phase phase) = 0;
		
		// Resolve attempts
		virtual void resolveSearch() = 0;
		virtual void resolveShadow(const NavalUnit& target, bool& heldContact) 
			= 0;
		virtual void resolveAttack(int& midshipsLost, int& evasionLost) = 0;

		// Response to enemy request
		virtual bool checkSearch(const GridCoordinate& zone) = 0;
};

#endif
