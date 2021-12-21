
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
#include "../Algorithms/Simulations.h"

using std::vector;
using std::set;
using std::map;

class CoBuchiAutomatonCompl : public GeneralizedCoBuchiAutomaton<int, int>
{

private:
  Relation<int> dirSim;
  Relation<int> reachDirSim;

public:
  CoBuchiAutomatonCompl(GeneralizedCoBuchiAutomaton<int, int> *t) : GeneralizedCoBuchiAutomaton<int, int>(*t) { }

  CoBuchiAutomatonCompl(BuchiAutomaton<int,int>& inhWeakBA)
  {
    // convert inherently weak to co-BA
    auto sccs = inhWeakBA.getAutGraphSCCs();
    auto finals = inhWeakBA.getFinals();
    auto finTrans = inhWeakBA.getFinTrans();
    std::set<int> fins;
    for (auto scc : sccs){
      if ((not std::any_of(scc.begin(), scc.end(), [finals](int state){return finals.find(state) != finals.end();})) and
          (not std::any_of(finTrans.begin(), finTrans.end(), [scc](auto tr){return scc.find(tr.from) != scc.end() and scc.find(tr.to) != scc.end();})))
        fins.insert(scc.begin(), scc.end());
    }
    std::map<int, std::set<int>> mp;
    mp.insert({0, fins});
    this->setFinals(mp);

    // change IW to weak
    std::set<int> weakFins;
    std::set_difference(inhWeakBA.getStates().begin(), inhWeakBA.getStates().end(), fins.begin(), fins.end(), std::inserter(weakFins, weakFins.begin()));
    inhWeakBA.setFinals(weakFins);

    // compute direct sim on weak automaton
    this->dirSim = inhWeakBA.getDirectSim();

    // reachability
    Relation<int> rel;
    SCCs reachabilityVector = inhWeakBA.reachableVector();
    for (auto pr : this->dirSim){
      // check reachability
      //if (reachabilityVector[pr.second].find(pr.first) == reachabilityVector[pr.second].end())

      set<int> intersect;
      std::set_intersection(reachabilityVector[pr.first].begin(), reachabilityVector[pr.first].end(), reachabilityVector[pr.second].begin(), reachabilityVector[pr.second].end(), std::inserter(intersect, intersect.begin()));
      if(intersect.size() == 0)
        rel.insert(pr);

      if (reachabilityVector[pr.first].find(pr.second) != reachabilityVector[pr.first].end() and reachabilityVector[pr.second].find(pr.first) == reachabilityVector[pr.second].end())
        rel.insert(pr);
    }
    this->reachDirSim = rel;

    this->states = inhWeakBA.getStates();
    this->trans = inhWeakBA.getTransitions();
    this->initials = inhWeakBA.getInitials();
    this->alph = inhWeakBA.getAlphabet();
    this->apsPattern = inhWeakBA.getAPPattern();
  }

  BuchiAutomaton<StateGcoBA, int> complementCoBA();
  BuchiAutomaton<StateGcoBA, int> complementCoBASim(ComplOptions opt);
  set<int> succSet(set<int>& states, int symbol);
  set<int> getDirectSet(set<int>& states, Relation<int>& dirSim);
  set<int> getSatSet(set<int>& allStates, Relation<int>& dirSim);

  Relation<int>& getWeakDirSim(){
    return this->reachDirSim;
  }

  Relation<int>& getDirSim(){
    return this->dirSim;
  }
};

#endif
