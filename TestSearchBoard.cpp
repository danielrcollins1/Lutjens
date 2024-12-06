/*
	Name: TestSearchBoard
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 29-11-24 11:49
	Description: Test the SearchBoard object.
		Create object, read from data files, print to screen.
*/
#include <iostream>
#include "SearchBoard.h"

// Min test driver
int main(int argc, char** argv) {
	SearchBoard::instance()->print();
	return 0;
}