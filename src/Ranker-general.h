
#ifndef _RANKER_GENERAL_H_
#define _RANKER_GENERAL_H_

#include <cstdlib>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <chrono>
#include <iomanip>

//#include "Compl-config.h"
#include "Complement/Options.h"
#include "Complement/BuchiAutomatonSpec.h"
#include "Algorithms/AuxFunctions.h"
#include "Automata/BuchiAutomaton.h"
#include "Automata/BuchiAutomataParser.h"
#include "Algorithms/Simulations.h"

using namespace std;

enum InFormat
{
  HOA,
  BA
};

struct Params
{
  string output;
  string input;
  bool stats;
};

struct Stat
{
  size_t generatedStates;
  size_t generatedTrans;
  size_t reachStates;
  size_t reachTrans;
  long duration;
  string engine;
};

InFormat parseRenamedAutomaton(ifstream& os);
BuchiAutomaton<int, int> parseRenameHOA(ifstream& os, BuchiAutomaton<int, APSymbol>* orig);
BuchiAutomaton<int, int> parseRenameBA(ifstream& os, BuchiAutomaton<string, string>* orig);

void complementScheweAutWrap(BuchiAutomaton<int, int>& ren, BuchiAutomaton<int, int>* complRes, Stat* stats, bool delay);
void complementAutWrap(BuchiAutomaton<int, int>& ren, BuchiAutomaton<int, int>* complRes, Stat* stats, bool delay);
void printStat(Stat& st);

BuchiAutomaton<int, int> createBA(vector<int>& loop);

/**
 * @brief  Retrieves the help message
 *
 * @param[in]  progName  The name of the executable
 *
 * @returns  The help message
 */
std::string getHelpMsg(const std::string& progName);

#endif
