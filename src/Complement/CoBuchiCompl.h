
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
    auto sccs = inhWeakBA.getAutGraphSCCs();
    auto finals = inhWeakBA.getFinals();
    std::set<int> fins;
    for (auto scc : sccs){
      if (not std::any_of(scc.begin(), scc.end(), [finals](int state){return finals.find(state) != finals.end();}))
        fins.insert(scc.begin(), scc.end());
    }
    std::map<int, std::set<int>> mp;
    mp.insert({0, fins});
    this->setFinals(mp);

    this->states = inhWeakBA.getStates();
    this->trans = inhWeakBA.getTransitions();
    this->initials = inhWeakBA.getInitials();
    this->alph = inhWeakBA.getAlphabet();
    this->apsPattern = inhWeakBA.getAPPattern();
  }

  BuchiAutomaton<StateGcoBA, int> complementCoBA();
  set<int> succSet(set<int>& states, int symbol);
};

#endif
