#ifndef _BUCHI_DELAY_H_
#define _BUCHI_DELAY_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <chrono>

#include <iostream>
#include <algorithm>

#include "../Algorithms/AuxFunctions.h"
#include "../Automata/BuchiAutomaton.h"
#include "StateKV.h"
#include "RankFunc.h"
#include "StateSch.h"

template <typename Symbol>
class BuchiAutomatonDelay : public BuchiAutomaton<StateSch, Symbol> {

public:

  BuchiAutomatonDelay(BuchiAutomaton<StateSch, Symbol>& other) : BuchiAutomaton<StateSch, Symbol>(other) { }

  unsigned getTransitionsToTight();
  vector<vector<StateSch>> getAllCycles();
  bool circuit(int state, std::vector<int> &stack, std::set<int> &blockedSet, std::map<int, std::set<int>> &blockedMap,
    std::set<int> scc, AdjList adjlist, int startState, std::vector<std::vector<int>> &allCyclesRenamed);
  void unblock(int state, std::set<int> &blockedSet, std::map<int, std::set<int>> &blockedMap);
  unsigned getAllPossibleRankings(unsigned maxRank, unsigned accStates, unsigned nonAccStates, delayVersion version);
  std::map<StateSch, std::set<Symbol>> getCycleClosingStates(set<StateSch>& slignore, DelayMap<StateSch>& dmap, double w, delayVersion version, Stat *stats);
};

#endif
