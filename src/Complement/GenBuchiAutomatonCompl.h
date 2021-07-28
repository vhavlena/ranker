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
#include "StateSchGBA.h"

using std::vector;
using std::set;
using std::map;

/*
 * Specified Generalized Buchi automata with complementation
 */
class GeneralizedBuchiAutomatonCompl : public GeneralizedBuchiAutomaton<int, int>
{
private:
  BackRel createBackRel(BuchiAutomaton<int, int>::StateRelation& rel);

  map<DFAState, RankBound> rankBound;
  SuccRankCache rankCache;

  ComplOptions opt;

public:
  GeneralizedBuchiAutomatonCompl(GeneralizedBuchiAutomaton<int, int> *t) : GeneralizedBuchiAutomaton<int, int>(*t)
  {
    opt = {.cutPoint = false};
  }

  void setComplOptions(ComplOptions& co) { this->opt = co; }
  ComplOptions getComplOptions() const { return this->opt; }

  BuchiAutomaton<StateSchGBA, int> complementSchReduced(bool delay, std::map<int,std::set<int>> originalFinals, double w, delayVersion version, Stat *stats);
};

#endif
