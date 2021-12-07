
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
#include "Complement/GenBuchiAutomatonCompl.h"
#include "Algorithms/AuxFunctions.h"
#include "Automata/BuchiAutomaton.h"
#include "Automata/BuchiAutomataParser.h"
#include "Algorithms/Simulations.h"
#include "Debug/BuchiAutomatonDebug.h"
#include "Automata/GenBuchiAutomaton.h"
#include "Automata/GenCoBuchiAutomaton.h"
#include "Complement/GenCoBuchiAutomatonCompl.h"
#include "Complement/CoBuchiCompl.h"

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
AutomatonStruct<int, APSymbol>* parseRenameHOA(ifstream& os);
BuchiAutomaton<int, int> parseRenameBA(ifstream& os, BuchiAutomaton<string, string>* orig);

void complementScheweAutWrap(BuchiAutomaton<int, int>* ren, BuchiAutomaton<int, int>* complRes, Stat* stats, bool delay, double w);
void complementAutWrap(BuchiAutomatonSpec& sp, BuchiAutomaton<int, int>* ren,
    BuchiAutomaton<StateSch, int>* complOrig, BuchiAutomaton<int, int>* complRes,
    Stat* stats, bool updateBounds = false);
void printStat(Stat& st);

BuchiAutomaton<int, int> createBA(vector<int>& loop);

void complementGcoBAWrap(GeneralizedCoBuchiAutomaton<int, int> *ren, BuchiAutomaton<StateGcoBA, int> *complOrig, BuchiAutomaton<int, int>* complRes, Stat* stats);
void complementCoBAWrap(CoBuchiAutomatonCompl *ren, BuchiAutomaton<StateGcoBA, int> *complOrig, BuchiAutomaton<int, int>* complRes, Stat* stats, ComplOptions opt);

void complementSDWrap(BuchiAutomatonSpec& sp ,BuchiAutomaton<int, int>* ren, BuchiAutomaton<int, int>* complRes,
    Stat* stats);

GeneralizedCoBuchiAutomaton<int, APSymbol> parseRenameHOAGCOBA(BuchiAutomataParser& parser);
BuchiAutomaton<int, APSymbol> parseRenameHOABA(BuchiAutomataParser& parser, ComplOptions opt);

/**
 * @brief  Retrieves the help message
 *
 * @param[in]  progName  The name of the executable
 *
 * @returns  The help message
 */
std::string getHelpMsg(const std::string& progName);

#endif
