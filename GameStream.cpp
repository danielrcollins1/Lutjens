#include "GameStream.h"

// Global object
GameStream cgame;

// Constructor
GameStream::GameStream(): std::ostream(this) {
	logfile.open(LOG_FILENAME);
	std::clog.rdbuf(logfile.rdbuf());
}

// Destructor
GameStream::~GameStream() {
	logfile.close();
	std::clog.rdbuf(nullptr);	
}

// Overflow override
int GameStream::overflow(int c) {
	if (active) {
		std::cout.put(c);
		std::clog.put(c);
	}
	return 0;
}

// Turn on game messages
void GameStream::turnOn() {
	active = true;	
}
		
// Turn off game messages		
void GameStream::turnOff() {
	active = false;	
}
