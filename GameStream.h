/*
	Name: GameStream
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 28-11-24 23:18
	Description: An output stream to handle key game messages.
		Data sent to this object is forwarded to both cout and clog.
*/
#ifndef GAMESTREAM_H
#define GAMESTREAM_H
#include <iostream>
#include <fstream>

// GameStream definition
class GameStream: private std::streambuf, public std::ostream
{
	public:
		GameStream();
		~GameStream();
		void turnOn();
		void turnOff();

	private:
		const std::string LOG_FILENAME = "Logfile.txt";
		int overflow(int c) override;
		std::ofstream logfile;
		bool active = true;
};

// GameStream global object
extern GameStream cgame;

#endif