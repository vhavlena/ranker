
#ifndef _GEN_CO_BUCHI_AUTOMATON_COMPL_H_
#define _GEN_CO_BUCHI_AUTOMATON_COMPL_H_

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

/*
 * Specified Generalized Buchi automata with complementation
 */
class GeneralizedCoBuchiAutomatonCompl : public GeneralizedCoBuchiAutomaton<int, int>
{

public:
  GeneralizedCoBuchiAutomatonCompl(GeneralizedCoBuchiAutomaton<int, int> *t) : GeneralizedCoBuchiAutomaton<int, int>(*t)
  {
  }

  BuchiAutomaton<StateGcoBA, int> complementGcoBA();
  set<int> succSet(set<int>& states, int symbol);
};

#endif