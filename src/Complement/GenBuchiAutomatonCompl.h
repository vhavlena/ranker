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

protected:
  bool isSchGBAFinal(StateSchGBA& state) const { return state.tight ? state.O.size() == 0 : state.S.size() == 0; }
  set<int> succSet(set<int>& state, int symbol);

public:
  GeneralizedBuchiAutomatonCompl(GeneralizedBuchiAutomaton<int, int> *t) : GeneralizedBuchiAutomaton<int, int>(*t)
  {
    opt = {.cutPoint = false};
  }

  void setComplOptions(ComplOptions& co) { this->opt = co; }
  ComplOptions getComplOptions() const { return this->opt; }

  BuchiAutomaton<StateSchGBA, int> complementSchReduced(bool delay, std::map<int,std::set<int>> originalFinals, double w, delayVersion version, Stat *stats);
  BuchiAutomaton<StateSchGBA, int> complementSchNFA(set<int>& start);

  BuchiAutomaton<StateSchGBA, int> getOneTightPart(std::set<int> originalFinals, unsigned number, std::stack<StateSchGBA> stack, std::map<int, int> reachCons, std::map<DFAState, int> maxReach, BackRel dirRel, BackRel oddRel, bool eta4, std::set<StateSchGBA> comst, map<std::pair<StateSchGBA, int>, set<StateSchGBA> > mp, map<std::pair<StateSchGBA, int>, set<StateSchGBA>> prev, BuchiAutomatonSpec &buchi, std::set<StateSchGBA> initials);
};

#endif
