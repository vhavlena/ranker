#ifndef _SEMI_DETERMINISTIC_COMPL_H_
#define _SEMI_DETERMINISTIC_COMPL_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <chrono>
#include <algorithm>

#include <iostream>
#include <algorithm>

#include "../Algorithms/AuxFunctions.h"
#include "../Automata/BuchiAutomaton.h"
#include "StateSD.h"
#include "Options.h"
#include "../Algorithms/Simulations.h"

using std::vector;
using std::set;
using std::map;

/*
 * Semi-deterministic automata complementation
 */
class SemiDeterministicCompl : public BuchiAutomaton<int, int>
{
private:
    std::set<int> det;
    std::set<int> nondet;
    Relation<int> dirSim;
    Relation<int> reachDirSim;

protected:


public:
  SemiDeterministicCompl(BuchiAutomaton<int, int> *t) : BuchiAutomaton<int, int>(*t), det(), nondet()
  {
    SCCs sccs = this->getAutGraphSCCs();
    for (auto scc : sccs){
        if (this->isReachDeterministic(scc)){
            this->det.insert(scc.begin(), scc.end());
        } else {
            this->nondet.insert(scc.begin(), scc.end());
        }
    }

    // direct sim
    this->dirSim = this->getDirectSim();
    // reachability
    Relation<int> rel;
    SCCs reachabilityVector = this->reachableVector();
    for (auto pr : this->dirSim){
      // check reachability
      set<int> intersect;
      std::set_intersection(reachabilityVector[pr.first].begin(), reachabilityVector[pr.first].end(), reachabilityVector[pr.second].begin(), reachabilityVector[pr.second].end(), std::inserter(intersect, intersect.begin()));
      if(intersect.size() == 0)
        rel.insert(pr);

      if (reachabilityVector[pr.first].find(pr.second) != reachabilityVector[pr.first].end() and reachabilityVector[pr.second].find(pr.first) == reachabilityVector[pr.second].end())
        rel.insert(pr);
    }
    this->reachDirSim = rel;
  }

  BuchiAutomaton<StateSD, int> complementSD(ComplOptions opt);

  std::set<int>& getDet(){
      return this->det;
  }

  std::set<int>& getNonDet(){
      return this->nondet;
  }

  void addDetState(int state){
      this->det.insert(state);
  }

  bool isSDStateFinal(StateSD state){
      return state.B.size() == 0;
  }

  std::vector<StateSD> getSuccessorsOriginal(StateSD& state, int symbol);
  std::vector<StateSD> getSuccessorsLazy(StateSD& state, int symbol);
  std::vector<StateSD> getSuccessorsMaxRank(StateSD& state, int symbol);

  void ncsbTransform();

  set<int> getDirectSet(set<int>& states, Relation<int>& dirSim);

  Relation<int>& getWeakDirSim(){
    return this->reachDirSim;
  }
};

#endif
