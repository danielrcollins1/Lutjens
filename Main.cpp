/*
	Name: Lutjens
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 23-11-24 18:03
	Description: A program to run the German ships
		in the 1979 Avalon Hill game Bismarck
*/
#include <iostream>
#include <iomanip>
#include "GameDirector.h"
#include "GameStream.h"
#include "CmdArgs.h"
#include "Utils.h"
using namespace std;

// Prototypes
void runLargeSeries();

// Main driver
int main(int argc, char** argv) {
	cgame << "LUTJENS: German player and game director\n"
		<< "for the 1979 Avalon Hill game Bismarck\n\n";
	auto argsObj = CmdArgs::instance();
	argsObj->parseArgs(argc, argv);
	if (argsObj->isExitAfterArgs()) {
		argsObj->printOptions();
	}
	else if (argsObj->isRunLargeSeries()) {
		runLargeSeries();
	}
	else {
		seedRandom();
		auto game = GameDirector::instance();
		if (game->okPlayerStart()) {
			game->doGameLoop();
			game->doEndGame();
			game->okPlayerEnd();
		}
	}
	return 0;
}

// Number of games in large series
const int NUM_GAMES = 1000;

// Run series of game & report stats
void runLargeSeries() {
	cout << "Running series of " << NUM_GAMES << " games...\n";
	
	// Initialize series
	seedRandom();
	cgame.turnOff();
	int numGermansFound = 0;
	int numSomeConvoySunk = 0;
	
	// Run series
	for (int i = 0; i < NUM_GAMES; i++) {
		GameDirector::initGame();
		auto game = GameDirector::instance();
		game->doGameLoop();
		if (game->wasAnyShipDetected()) {
			numGermansFound++;			
		}
		if (game->getConvoysSunk() > 0) {
			numSomeConvoySunk++;	
		}
	}
	
	// Report statistics
	cout << fixed << showpoint << setprecision(2);
	cout << "Ratio Germans found: " 
		<< (float) numGermansFound / NUM_GAMES << "\n";
	cout << "Ratio some convoy sunk: " 
		<< (float) numSomeConvoySunk / NUM_GAMES << "\n";
}
