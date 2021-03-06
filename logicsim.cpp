#include "logicsim.h"
#include <iostream>
#include <fstream>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace std;

static unsigned int lineNum = 0;
enum{INPUTS, VALUES, BLANK};

LogicSim::LogicSim(string file, Design *design)
{
	ifstream ifile(file.c_str(), ifstream::in);
	while(ifile.good())
	{
		lineNum++;
    				
		string currentline;
		getline(ifile, currentline);
		stringstream ss(currentline);
		string firsttoken;
		ss >> firsttoken;
		try{
			switch(defineLine(firsttoken))
			{
				case INPUTS	:
				{
					pi_order.push_back(firsttoken);
          pi_set.insert(firsttoken);
          
          // Check existence
          if(design->get_design_nets()->find(firsttoken) == design->get_design_nets()->end()) {
            throw runtime_error("A specified PI on the first line does not exist in the design");
          }
          
					while(ss.good())
					{
						string temp;
						ss >> temp;
						pi_order.push_back(temp);
            pi_set.insert(temp);
            
            // Check existence
            if(design->get_design_nets()->find(temp) == design->get_design_nets()->end()) {
              throw runtime_error("A specified PI on the first line does not exist in the design");
            }
            
					}
          
          // Check size
          if(design->get_pi_nets().size() != pi_set.size())
            throw runtime_error("PI count in sim disagrees with definition");
					break;
				}
				case VALUES	:
				{
          map<string, char> line;
          int index = 0;
          line[pi_order.at(index)] = firsttoken[0];
          if(!(firsttoken[0] == '1' || firsttoken[0] == '0' || firsttoken[0] == 'X')) { throw runtime_error("Input is not in the set {1, 0, X}");}
          while(ss.good()) {
            index++;
            char tmp;
            ss >> tmp;
            if(!ss.good()) {
              break;
            }
            
            if(!(tmp == '1' || tmp == '0' || tmp == 'X')) { throw runtime_error("Input is not in the set {1, 0, X}");}
            
            if(index < pi_order.size())
              line[pi_order.at(index)] = tmp;
            else {
              stringstream ss;
              ss << "The number of input values for a vector is not equal to than the number of PI's specified on the first line";
              throw runtime_error(ss.str());
              
            }
          } 
              
                  
          // Check count
          if(line.size() != pi_order.size()) {
            stringstream ss;
            ss << "The number of input values for a vector is not equal to than the number of PI's specified on the first line";
            throw runtime_error(ss.str());
          }
          
          values.push_back(line);
					break;
				}
				case BLANK	:
				{
					// done reading
					break;
				}
				default	:
				{
					throw runtime_error("grammar error");
					break;
				}
			}
		} catch(runtime_error &ex)	{
			string line;
			stringstream ss;
			ss << lineNum;
			line = ss.str();
			throw runtime_error("On line " + line + " " + ex.what());
		}
	}
	
	
}

void LogicSim::runSimulation(deque<Net *> topolist)
{  
  // For each line in the sim
  for(vector<map<string,char> >::iterator lineit=values.begin();
  lineit!=values.end();
  lineit++) 
  {
    map<string,char> values = *lineit;
    map<string,char> result;
    
    for(deque<Net*>::iterator it=topolist.begin();
    it!=topolist.end();
    it++)
    {
      Net *n = *it;
      string name = n->name();
      if(pi_set.find(name) != pi_set.end()) {
        n->setVal(values[name]);
        LOG("Set input value of " << name << " to " << values[name]);
      } else {
        n->computeVal();
        LOG("Computed value of " << name << " is " << n->getVal());
        result[name] = n->getVal();
      }
    }
    cout << endl;
    
    results.push_back(result);
  
  }
}

void LogicSim::outputTheFile(string file, Design* design)
{
  ofstream outfile(file.c_str());

  // Header
  for(vector<string>::iterator it=pi_order.begin(); it!=pi_order.end(); it++) {
    outfile << *it << " ";
  }
  outfile << "=> ";
  vector<Net*> nets = design->get_po_nets();
  for(vector<Net*>::iterator it=nets.begin(); it!=nets.end(); it++) {
    outfile << (*it)->name() << " ";
  }
  outfile << endl;
  
  // For each line
  int index = 0;
  for(vector<map<string,char> >::iterator it=values.begin(); it!=values.end(); it++){
    map<string,char> value = *it;
    for(vector<string>::iterator jt=pi_order.begin(); jt!=pi_order.end(); jt++) {
      outfile << value[*jt] << " ";
    }
    outfile << "=> ";
    vector<Net*> pos = design->get_po_nets();
    for(vector<Net*>::iterator jt=pos.begin(); jt!=pos.end(); jt++) {
      outfile << results.at(index)[(*jt)->name()] << "@" << (*jt)->computeDelay() << " ";
    }
    index++;
    outfile << endl;
  }
  
	
  outfile.close();
}

int defineLine(string identifier)
{
	if((identifier == "0") | (identifier == "1"))
	{
		return VALUES;
	}
	else if((identifier == " ") | (identifier == "\n") | (identifier == "\t") | (identifier == ""))
	{
		return BLANK;
	}
	else if(isalpha(identifier.c_str()[0]))
	{
		return INPUTS;
	}
}