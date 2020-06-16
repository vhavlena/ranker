#ifndef _BUCHI_AUTOMATON_SPEC_H_
#define _BUCHI_AUTOMATON_SPEC_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <stack>

#include <boost/dynamic_bitset.hpp>

#include <iostream>
#include <algorithm>

#include "AuxFunctions.h"
#include "BuchiAutomaton.h"
#include "StateKV.h"

using std::vector;
using std::set;
using std::map;

class BuchiAutomatonSpec : public BuchiAutomaton<int, int>
{
private:
  typedef vector<set<vector<std::pair<int, int> > > > RankConstr;
  typedef map<int, int> Rank;

protected:
  RankConstr rankConstr(vector<int>& max, set<int>& states) const;

  vector<Rank> getKVRanks(vector<int>& max, set<int>& states) const;
  set<StateKV<int> > succSetKV(StateKV<int> state, int symbol) const;
  bool isKVFinal(StateKV<int> state) const { return state.O.size() == 0; }

  bool isTightRank(Rank & r, int maxRank) const;
  vector<Rank> getSchRanks(vector<int> max, std::set<int> states, StateSch<int> macrostate) const;
  set<StateSch<int> > succSetSch(StateSch<int> state, int symbol) const;

public:
  BuchiAutomatonSpec(BuchiAutomaton<int, int> &t) : BuchiAutomaton<int, int>(t) {}

  BuchiAutomaton<StateKV<int>, int> complementKV() const;
};

#endif
