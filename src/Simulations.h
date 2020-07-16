#ifndef _BUCHI_SIMULATION_H_
#define _BUCHI_SIMULATION_H_

#include <iostream>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include "BuchiAutomaton.h"

using namespace std;

struct RabitSimLine
{
  pair<string, string> rel;
  pair<string, string> aut;
};

class Simulations {

public:
  BuchiAutomaton<std::string, std::string>::StateRelation parseRabitRelation(istringstream & os);

private:
  static RabitSimLine parseRabitRelLine(string& line);
  static void splitString(string& line, vector<string>& split);
};

#endif
