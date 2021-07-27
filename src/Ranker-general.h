
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
#include "Debug/BuchiAutomatonDebug.h"

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
  string checkWord;
};

InFormat parseRenamedAutomaton(ifstream& os);
BuchiAutomaton<int, APSymbol>* parseRenameHOA(ifstream& os/*, BuchiAutomaton<int, APSymbol>* orig*/); 
AutomatonStruct<int, int>* parseRenameBA(ifstream& os, BuchiAutomaton<string, string>* orig);

void complementScheweAutWrap(AutomatonStruct<int, int>* ren, BuchiAutomaton<int, int>* complRes, Stat* stats, bool delay, double w, delayVersion version); 
void complementAutWrap(AutomatonStruct<int, int>* ren, BuchiAutomaton<StateSch, int>* complOrig, BuchiAutomaton<int, int>* complRes, Stat* stats, bool delay, double w, delayVersion version, bool elevatorRank, bool eta4);
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
