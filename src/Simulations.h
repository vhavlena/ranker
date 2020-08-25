#ifndef _BUCHI_SIMULATION_H_
#define _BUCHI_SIMULATION_H_

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <exception>
#include <queue>
#include <map>
#include <set>

#include <boost/algorithm/string.hpp>
#include "BuchiAutomaton.h"

using namespace std;

template<typename State> using Relation = set<pair<State, State>>;

class TimeoutException: public exception
{
  virtual const char* what() const throw()
  {
    return "Timeout expired";
  }
};

struct RabitSimLine
{
  pair<string, string> rel;
  pair<string, string> aut;
};

class Simulations {

public:
  BuchiAutomaton<std::string, std::string>::StateRelation parseRabitRelation(istringstream & os);

  static string execCmd(string& cmd, int timeout = 1000);

protected:
  template<typename State, typename Symbol>
  Relation<State> computeDirectCompl(BuchiAutomaton<State, Symbol>& ba);

private:
  static RabitSimLine parseRabitRelLine(string& line);
  static void splitString(string& line, vector<string>& split);

  template<typename State, typename Symbol>
  Relation<State> directSimulation(BuchiAutomaton<State, Symbol>& ba);
};

#endif
