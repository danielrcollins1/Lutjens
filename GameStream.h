/*
	Name: GameStream
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 28-11-24 23:18
	Description: An output stream to handle game messages.
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

	private:
		int overflow(int c) override;
		std::ofstream logfile;
};

// GameStream global object
extern GameStream cgame;

#endif