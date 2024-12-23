#include "CmdArgs.h"
#include <iostream>

// Pointer to the singleton object.
CmdArgs* CmdArgs::theInstance = nullptr;

// Accessor to the singleton object.
CmdArgs* CmdArgs::instance() {
	if (!theInstance) {
		theInstance = new CmdArgs;
	}
	return theInstance;
}

// Constructor
CmdArgs::CmdArgs() {
}

// Print available options
void CmdArgs::printOptions() const {
	std::cout << "Command line options available:\n"
		<< "\t-a automate British player\n"
		<< "\t-l large series of games\n"
		<< "\t-f finish on turn number\n"
		<< "\t-n number of games to run\n";
}

// Parse the command-line arguments
void CmdArgs::parseArgs(int argc, char** argv) {
	for (int count = 1; count < argc; count++) {
		char *arg = argv[count];
		if (arg[0] == '-') {
			switch (arg[1]) {
				case 'l': runLargeSeries = true; // & fall through
				case 'a': automateBritish = true; break;
				case 'f': lastTurn = parseArgAsInt(arg); break;
				case 'n': numTrials = parseArgAsInt(arg); break;
				default: exitAfterArgs = true; break;
			}
		}
		else {
			exitAfterArgs = true;
		}
	}
}

// Parse an argument as an integer
//   Format as -a=###
int CmdArgs::parseArgAsInt(char *s) {
	if (strlen(s) > 3 && s[2] == '=') {
		return atoi(s + 3);
	}
	return -1;
}
