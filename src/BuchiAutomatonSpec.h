#ifndef _BUCHI_AUTOMATON_SPEC_H_
#define _BUCHI_AUTOMATON_SPEC_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <algorithm>

#include "AuxFunctions.h"
#include "BuchiAutomaton.h"
#include "ComplementBA.h"

class BuchiAutomatonSpec : public BuchiAutomaton<int, int>
{
protected:
  std::vector<std::map<int, int> > getKVRanks(std::vector<int> max, std::set<int> states) const;
  std::set<StateKV<int> > succSetKV(StateKV<int> state, int symbol) const;
  bool isKVFinal(StateKV<int> state) const { return state.O.size() == 0; }

public:
  BuchiAutomatonSpec(BuchiAutomaton<int, int> &t) : BuchiAutomaton<int, int>(t) {}

  BuchiAutomaton<StateKV<int>, int> complementKV() const;
};

#endif
