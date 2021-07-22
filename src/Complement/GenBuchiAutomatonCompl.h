#ifndef _GEN_BUCHI_COMPL_H_
#define _GEN_BUCHI_COMPL_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <chrono>

#include <iostream>
#include <algorithm>

#include "../Algorithms/AuxFunctions.h"
#include "BuchiAutomatonSpec.h"
#include "../Automata/BuchiAutomaton.h"
#include "../Automata/GeneralizedBuchiAutomaton.h"
#include "BuchiDelay.h"
#include "StateKV.h"
#include "RankFunc.h"
#include "StateSch.h"
#include "Options.h"

using std::vector;
using std::set;
using std::map;

/*
 * Specified Generalized Buchi automata with complementation
 */
class GeneralizedBuchiAutomatonCompl : public GeneralizedBuchiAutomaton<int, int>
{

public:
  BuchiAutomatonSpec(BuchiAutomaton<int, int> &t) : GeneralizedBuchiAutomaton<int, int>(t)
  {
  }


  GeneralizedBuchiAutomaton<int, int> complementSchReduced(bool delay, std::set<int> originalFinals, double w, delayVersion version, Stat *stats);
};

#endif
