#ifndef _BUCHI_AUTOMATON_SPEC_H_
#define _BUCHI_AUTOMATON_SPEC_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <stack>

#include <iostream>
#include <algorithm>

#include "RankFunc.h"
#include "AuxFunctions.h"
#include "BuchiAutomaton.h"
#include "StateKV.h"
#include "StateSch.h"

using std::vector;
using std::set;
using std::map;

class BuchiAutomatonSpec : public BuchiAutomaton<int, int>
{
private:
  //typedef map<int, int> Rank;

protected:
  RankConstr rankConstr(vector<int>& max, set<int>& states) const;

  vector<RankFunc> getKVRanks(vector<int>& max, set<int>& states) const;
  set<StateKV> succSetKV(StateKV state, int symbol) const;
  bool isKVFinal(StateKV state) const { return state.O.size() == 0; }

  vector<RankFunc> getSchRanks(vector<int> max, std::set<int> states, StateSch macrostate) const;
  set<StateSch> succSetSch(StateSch state, int symbol) const;

public:
  BuchiAutomatonSpec(BuchiAutomaton<int, int> &t) : BuchiAutomaton<int, int>(t) {}

  BuchiAutomaton<StateKV, int> complementKV() const;
};

#endif
