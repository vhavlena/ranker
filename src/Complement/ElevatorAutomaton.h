
#ifndef _ELEVATOR_AUTOMATON_H_
#define _ELEVATOR_AUTOMATON_H_

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
#include "BuchiDelay.h"
#include "StateKV.h"
#include "RankFunc.h"
#include "StateSch.h"
#include "Options.h"

enum sccType {D, ND, BAD, BOTH}; // deterministic with accepting states / nondeterministic without accepting states / bad = nondeterministic with accepting states / both = deterministic without accepting states

class ElevatorAutomaton : public BuchiAutomaton<int, int> {

public:
  ElevatorAutomaton(BuchiAutomaton<int, int> t) : BuchiAutomaton<int, int>(t) { }

  bool isElevator();
  std::vector<std::set<int>> topologicalSort(map<int, set<int>>& predSyms);

  unsigned elevatorStates();
  std::map<int, int> elevatorRank(bool detBeginning);

  BuchiAutomaton<int, int> copyPreprocessing(const std::function<bool(SccClassif)>& pred);
  BuchiAutomaton<int, int> propagateAccStates();

  bool isInherentlyWeakBA();
  BuchiAutomaton<int, int> convertToWeak();

  BuchiAutomaton<int, int> nondetInitDeterminize();

protected:
  bool isDeterministic(std::set<int>& scc, map<int, set<int> >& predSyms);
  bool isNonDeterministic(std::set<int>& scc);
  bool isInherentlyWeak(const std::set<int>& scc, map<int, set<int> >& predSyms);

  void topologicalSortUtil(std::set<int> currentScc, std::vector<std::set<int>> allSccs, std::map<std::set<int>, bool> &visited, std::stack<std::set<int>> &Stack, vector<set<int>>& adjList);

private:
  map<int, bool> nondetStates();

};

#endif
