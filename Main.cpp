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
	seedRandom();
	auto argsObj = CmdArgs::instance();
	argsObj->parseArgs(argc, argv);
	if (argsObj->isExitAfterArgs()) {
		argsObj->printOptions();
	}
	else if (argsObj->isRunLargeSeries()) {
		runLargeSeries();
	}
	else {
		auto game = GameDirector::instance();
		if (game->okPlayerStart()) {
			game->doGameLoop();
			game->doEndGame();
			game->okPlayerEnd();
		}
	}
	return EXIT_SUCCESS;
}

// Number of games in large series
const int DEFAULT_NUM_GAMES = 1000;

// Run series of game & report stats
void runLargeSeries() {
	
	// Get number of games
	int numGames = CmdArgs::instance()->getNumTrials();
	if (numGames <= 0) {
		numGames = DEFAULT_NUM_GAMES;	
	}
	cgame << "Running series of " << numGames << " games...\n";

	// Turn off normal game logging
	cgame.turnOff();
	std::ofstream nullstream;
	std::clog.rdbuf(nullstream.rdbuf());
	
	// Initialize series
	int gamesDetected = 0;
	int gamesConvoySunk = 0;
	int totalDetections = 0;
	int totalConvoysSunk = 0;
	const int NUM_SUNK_BINS = 7;
	int convoysSunkBin[NUM_SUNK_BINS] = {0};

	// Run series
	for (int i = 0; i < numGames; i++) {
		//cout << "\n# New Game Starts\n";
		GameDirector::initGame();
		auto game = GameDirector::instance();
		game->doGameLoop();

		// Record times detected
		int timesDetected = game->getBismarck().getTimesDetected();
		if (timesDetected > 0) {
			totalDetections += timesDetected;
			gamesDetected++;
		}

		// Record convoys sunk
		int convoysSunk = game->getConvoysSunk();
		if (convoysSunk > 0) {
			totalConvoysSunk += convoysSunk;
			gamesConvoySunk++;
		}	
		if (convoysSunk < NUM_SUNK_BINS) {
			convoysSunkBin[game->getConvoysSunk()]++;
		}
	}

	// Report statistics
	cout << fixed << showpoint << setprecision(2);
	cout << "Games Bismarck detected: " 
		<< (float) gamesDetected / numGames << "\n";
	cout << "Games convoy sunk: " 
		<< (float) gamesConvoySunk / numGames << "\n";
	cout << "Mean Bismarck detections: "
		<< (float) totalDetections / numGames << "\n";
	cout << "Mean convoys sunk: " 
		<< (float) totalConvoysSunk / numGames << "\n";
	cout << "Convoys/detection ratio: "
		<< (float) totalConvoysSunk / totalDetections << "\n";
		
	// Report convoys sunk bins
	cout << "Convoys sunk relative frequencies:\n  ";
	for (int i = 0; i < NUM_SUNK_BINS; i++) {
		int percent = (int)((float) convoysSunkBin[i] / numGames * 100);
		if (percent > 0) {
			cout << (i ? ", " : "") << i << ":" << percent << "%";
		}
	}
	cout << "\n";
}
