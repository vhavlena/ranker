
#ifndef _COBUCHI_COMPL_H_
#define _COBUCHI_COMPL_H_

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
#include "../Automata/GenCoBuchiAutomaton.h"
#include "StateGcoBA.h"
#include "Options.h"

using std::vector;
using std::set;
using std::map;

class CoBuchiAutomatonCompl : public GeneralizedCoBuchiAutomaton<int, int>
{

public:
  CoBuchiAutomatonCompl(GeneralizedCoBuchiAutomaton<int, int> *t) : GeneralizedCoBuchiAutomaton<int, int>(*t) { }

  CoBuchiAutomatonCompl(BuchiAutomaton<int,int>& inhWeakBA)
  {
    //TODO: convert inherently weak to co-BA
  }

  BuchiAutomaton<StateGcoBA, int> complementCoBA();
};

#endif
