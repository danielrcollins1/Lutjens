/*
	Name: CmdArgs
	Copyright: 2024
	Author: Daniel R. Collins
	Date: 07-12-24 22:39
	Description: Command-line argument handler.
		Singleton class pattern.
*/
#ifndef CMDARGS_H
#define CMDARGS_H

class CmdArgs
{
	public:
		static CmdArgs* instance();
		void parseArgs(int argc, char** argv);
		void printOptions() const;

		// Accessor functions
		bool isExitAfterArgs() const { return exitAfterArgs; }
		bool isAutomatedBritish() const { return automateBritish; }
		bool isRunLargeSeries() const { return runLargeSeries; }
		int getLastTurn() const { return lastTurn; }
		int getNumTrials() const { return numTrials; }

	private:

		// Data
		static CmdArgs* theInstance;
		bool exitAfterArgs = false;
		bool automateBritish = false;
		bool runLargeSeries = false;
		int lastTurn = -1;
		int numTrials = -1;

		// Functions
		CmdArgs();
		int parseArgAsInt(char *s);
};

#endif
