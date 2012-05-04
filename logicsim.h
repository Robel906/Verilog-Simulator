#ifndef LOGICSIM_H
#define LOGICSIM_H
#include "global.h"
#include "net.h"
#include "gate.h"
#include "design.h"
#include <string>
#include <deque>

using namespace std;

int defineLine(string identifier);

class LogicSim	{
	public:
		LogicSim(string file);
		void runSimulation(deque<Net *> topolist);
		void outputTheFile(string file);
	private:
		vector<string> pis;
		vector<vector<int> > values;
		
};

#endif