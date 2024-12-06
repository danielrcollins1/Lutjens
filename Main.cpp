/*
	Name: Lutjens
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 23-11-24 18:03
	Description: A program to run the German ships
		in the 1979 Avalon Hill game Bismarck
*/
#include <iostream>
#include "GameDirector.h"
#include "GameStream.h"
#include "Utils.h"

// Main driver
int main(int argc, char** argv) {
	cgame << "LUTJENS: German player and game director\n"
		<< "for the 1979 Avalon Hill game Bismarck\n\n";
	cout << "Start game (y/n)? ";
	if (getUserYes()) {
		auto game = GameDirector::instance();
		game->doGameLoop();
		game->doEndGame();
		cout << "Press Enter to end program.\n";
		cin.ignore();
		cin.get();
	}
	return 0;
}
