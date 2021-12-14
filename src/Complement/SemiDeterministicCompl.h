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
  }

  BuchiAutomaton<StateSD, int> complementSD(bool original);

  std::set<int> getDet(){
      return this->det;
  }

  std::set<int> getNonDet(){
      return this->nondet;
  }

  bool isSDStateFinal(StateSD state){
      return state.B.size() == 0;
  }

  std::vector<StateSD> getSuccessorsOriginal(StateSD state, int symbol);
  std::vector<StateSD> getSuccessorsLazy(StateSD state, int symbol);

  std::vector<std::vector<int>> getSubsets(std::vector<int> states);

};

#endif
