#include "CmdArgs.h"
#include <iostream>
using namespace std;

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
		<< "\t-f finish on turn number\n"
		<< "\t-l large series of games\n"
		<< "\t-n number of games to run\n"
		<< "\n";
	std::cout << "Optional (intermediate) rules:\n"
		<< "\t-ofe fuel expenditure (rule 16.0)\n"
		<< "\t-ofd fuel damage (rule 21.0)\n"
		<< "\t-oas Admiral Scheer (rules 43.0, 44.0)\n"
		<< "\t-osg Scharnhorst & Gneisenau (rule 45.0)\n"
		<< "\t-ott Tirpitz (rule 48.0)\n";
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
				case 'o': parseOptionalRule(arg); break;
				default: setExitAfterArgs(); break;
			}
		}
		else {
			setExitAfterArgs();
		}
	}
}

// Set to exit program after argument parsing error
void CmdArgs::setExitAfterArgs() {
	exitAfterArgs = true;
}

// Parse an argument as an integer
//   Format as -a=###
int CmdArgs::parseArgAsInt(char *s) {
	if (strlen(s) > 3 && s[2] == '=') {
		return atoi(s + 3);
	}
	else {
		setExitAfterArgs();
		return -1;
	}
}

// Parse switch for optional (intermediate) rule
void CmdArgs::parseOptionalRule(char *s) {
	string arg(s);
	string optCode = arg.substr(2, 2);
	if (optCode == "fd") { optFuelDamage = true; }
	else if (optCode == "fe") { optFuelExpenditure = true; }
	else if (optCode == "as") { optScheer = true; }
	else if (optCode == "sg") { optScharnhorsts = true; }
	else if (optCode == "tt") { optTirpitz = true; }
	else { 
		setExitAfterArgs();
	}
}
