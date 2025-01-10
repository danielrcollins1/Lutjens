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

		// Optional (intermediate) rules
		bool useOptFuelExpenditure() const { return optFuelExpenditure; }
		bool useOptFuelDamage() const { return optFuelDamage; }
		bool useOptScheer() const { return optScheer; }
		bool useOptScharnhorsts() const { return optScharnhorsts; }
		bool useOptTirpitz() const { return optTirpitz; }

	private:

		// Data
		static CmdArgs* theInstance;
		bool exitAfterArgs = false;
		bool automateBritish = false;
		bool runLargeSeries = false;
		int lastTurn = -1;
		int numTrials = -1;

		// Optional rules
		bool optFuelExpenditure = false;
		bool optFuelDamage = false;
		bool optScheer = false;
		bool optScharnhorsts = false;
		bool optTirpitz = false;

		// Functions
		CmdArgs();
		int parseArgAsInt(char *s);
		void parseOptionalRule(char *s);
		void setExitAfterArgs();
};

#endif
