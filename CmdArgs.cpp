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
		<< std::endl;
}

// Parse the command-line arguments
void CmdArgs::parseArgs(int argc, char** argv) {
	for (int count = 1; count < argc; count++) {
		char *arg = argv[count];
		if (arg[0] == '-') {
			switch (arg[1]) {
				case 'a': automateBritish = true; break;
				default: exitAfterArgs = true; break;
			}
		}
		else {
			exitAfterArgs = true;
		}
	}
}
