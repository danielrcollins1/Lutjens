#include "GameStream.h"

// Global object
GameStream cgame;

// Constructor
GameStream::GameStream(): std::ostream(this) {
	logfile.open("Logfile.txt");
	std::clog.rdbuf(logfile.rdbuf());
}

// Destructor
GameStream::~GameStream() {
	logfile.close();
	std::clog.rdbuf(nullptr);	
}

// Overflow override
int GameStream::overflow(int c) {
	std::cout.put(c);
	std::clog.put(c);
	return 0;
}
