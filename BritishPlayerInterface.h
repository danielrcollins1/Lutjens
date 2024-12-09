/*
	Name: BritishPlayerInterface
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 08-12-24 00:07
	Description: Abstract class to represent a British player type.
*/
#ifndef BRITISHPLAYERINTERFACE_H
#define BRITISHPLAYERINTERFACE_H
#include "GridCoordinate.h"
#include "Ship.h"

class BritishPlayerInterface 
{
	public:
		virtual ~BritishPlayerInterface() {}
		
		// Request intentions
		virtual bool trySearch() = 0;
		virtual bool tryShadow(const Ship& target, 
			const GridCoordinate& knownPos, bool inSearchPhase) = 0;
		virtual bool tryAttack(const Ship& target, bool inSeaPhase) = 0;
		
		// Resolve attempts
		virtual void resolveSearch() = 0;
		virtual void resolveShadow(const Ship& target, bool& heldContact) = 0;
		virtual void resolveAttack(int& midshipsLost, int& evasionLost) = 0;
};

#endif
