
#include "ElevatorAutomaton.h"

bool ElevatorAutomaton::isDeterministic(std::set<int> scc){
  for (auto state : scc){
    for (auto a : this->getAlphabet()){
      unsigned trans = 0;
      for (auto succ : this->getTransitions()[{state, a}]){
        if (scc.find(succ) != scc.end()){
          if (trans > 0){
            return false;
          }
          trans++;
        }
      }
    }
  }
  return true;
}


bool ElevatorAutomaton::isNonDeterministic(std::set<int> scc){
  if (std::any_of(scc.begin(), scc.end(), [this](int state){return this->getFinals().find(state) != this->getFinals().end();}))
      return false;
  else
    return true;
}


bool ElevatorAutomaton::isInherentlyWeak(std::set<int> scc){
  SetStates st;
  SetStates fin = this->getFinals();
  SetStates empty;
  SetStates ini = this->getInitials();
  SetStates newIni;

  // states without accepting states
  for (auto state : scc){
    if (fin.find(state) == fin.end()){
      st.insert(state);
      if (ini.find(state) != ini.end())
        newIni.insert(state);
    }
  }

  Transitions newTrans;
  for (auto it = this->getTransitions().begin(); it != this->getTransitions().end(); it++){
    if (st.find(it->first.first) != st.end())
      newTrans.insert({it->first, it->second});
  }

  BuchiAutomaton<int, int> tmp(st, empty, newIni, newTrans, this->getAlphabet(), this->getAPPattern());
  ElevatorAutomaton tmpSpec(tmp);

  auto newSccs = tmpSpec.getAutGraphSCCs();
  for (auto scc : newSccs){
    if (scc.size() > 1)
      return false;
    else if (scc.size() == 1){
      int state;
      for (auto st : scc)
        state = st;
      for (auto symbol : this->getAlphabet()){
        auto reach = this->getTransitions()[{state, symbol}];
        if (reach.find(state) != reach.end())
          return false;
      }
    }
  }

  return true;
}


bool ElevatorAutomaton::isElevator(){
  // get all sorted sccs
  std::vector<std::set<int>> sccs = this->topologicalSort();
  std::vector<SccClassif> sccClass;
  for (auto scc : sccs){
    SccClassif tmp = {.states = scc, .det = false, .inhWeak = false, .nonDet = false};
    sccClass.push_back(tmp);
  }

  // scc classification
  for (auto it = sccClass.begin(); it != sccClass.end(); it++){
    // deterministic
    if (isDeterministic(it->states))
      it->det = true;
    // nondeterministic
    else if (isNonDeterministic(it->states))
      it->nonDet = true;
    // inherently weak
    else if (isInherentlyWeak(it->states))
      it->inhWeak = true;
    else
      return false;
  }

  return true;
}


void ElevatorAutomaton::topologicalSortUtil(std::set<int> currentScc, std::vector<std::set<int>> allSccs, std::map<std::set<int>, bool> &visited, std::stack<std::set<int>> &Stack){
  // mark the current node as visited
  visited[currentScc] = true;

  // recursion call for all nonvisited successors
  for (auto scc : allSccs){
    if (not visited[scc]){
      for (auto state : currentScc){
        for (auto a : this->getAlphabet()){
          if (std::any_of(scc.begin(), scc.end(), [this, state, a](int succ){auto trans = this->getTransitions(); return trans[{state, a}].find(succ) != trans[{state, a}].end();}))
            this->topologicalSortUtil(scc, allSccs, visited, Stack);
        }
      }
    }
  }

  // push scc to stack storing the topological order
  Stack.push(currentScc);
}

std::vector<std::set<int>> ElevatorAutomaton::topologicalSort(){
  // get all sccs
  std::vector<std::set<int>> sccs = this->getAutGraphSCCs();

  std::stack<std::set<int>> Stack;
  // no scc is visited
  std::map<std::set<int>, bool> visited;
  for (auto scc : sccs){
    visited.insert({scc, false});
  }

  // get topological sort starting from all sccs one by one
  for (auto scc : sccs){
    if (visited[scc] == false)
      this->topologicalSortUtil(scc, sccs, visited, Stack);
    //std::cerr << "size: " << scc.size() << std::endl;
  }

  // return topological sort
  std::vector<std::set<int>> sorted;
  while (not Stack.empty()){
    sorted.push_back(Stack.top());
    Stack.pop();
  }
  return sorted;
}


unsigned ElevatorAutomaton::elevatorStates(){

  // topological sort
  std::vector<std::set<int>> sortedComponents = this->topologicalSort();

  // determine scc type (deterministic, nondeterministic, bad, both)
  std::map<std::set<int>, sccType> typeMap;
  for (auto scc : sortedComponents){
    // is scc deterministic?
    bool det = true;
    for (auto state : scc){
      if (not det)
        break;
      for (auto a : this->getAlphabet()){
        if (not det)
          break;
        unsigned trans = 0;
        for (auto succ : this->getTransitions()[{state, a}]){
          if (scc.find(succ) != scc.end()){
            if (trans > 0){
              det = false;
              break;
            }
            trans++;
          }
        }
      }
    }

    // does scc contain accepting states?
    bool finalStates = false;
    if (std::any_of(scc.begin(), scc.end(), [this](int state){return this->getFinals().find(state) != this->getFinals().end();}))
      finalStates = true;

    // type of scc
    sccType type;
    if (det and finalStates)
      type = D;
    else if (not det and not finalStates)
      type = ND;
    else if (det and not finalStates)
      type = BOTH;
    else
      type = BAD;
    typeMap.insert({scc, type});
  }

  // propagate BAD back
  if (sortedComponents.size() > 0){
    for (int i = (int)sortedComponents.size()-1; i >= 0; i--){
      if (typeMap[sortedComponents[i]] == BAD){
        // type of all components before this one will also be BAD
        for (int j = 0; j < i; j++){
          typeMap[sortedComponents[j]] = BAD;
        }
        break;
      }
      if (i == 0) // i is unsigned
        break;
    }
  }

  unsigned elevatorStates = 0;
  for (auto scc : sortedComponents){
    if (typeMap[scc] != BAD)
      elevatorStates += scc.size();
  }
  return elevatorStates;
}


/**
 * Updates rankBound of every state based on elevator automaton structure (minimum of these two options)
 */
std::map<int, int> ElevatorAutomaton::elevatorRank(bool detBeginning){

  // get all sorted sccs
  std::vector<std::set<int>> sccs = this->topologicalSort();
  std::vector<SccClassif> sccClass;
  for (auto scc : sccs){
    SccClassif tmp = {.states = scc, .det = false, .inhWeak = false, .nonDet = false};
    sccClass.push_back(tmp);
  }

  // scc classification
  for (auto it = sccClass.begin(); it != sccClass.end(); it++){
    // deterministic
    if (isDeterministic(it->states))
      it->det = true;
    // nondeterministic
    if (isNonDeterministic(it->states))
      it->nonDet = true;
    // inherently weak
    if (isInherentlyWeak(it->states))
      it->inhWeak = true;
  }

  // for(auto cl : sccClass)
  // {
  //   cout << cl.det << " " << cl.inhWeak << " " << cl.nonDet << endl;
  //   for(auto st : cl.states)
  //     cout << st << " ";
  //   cout << endl << endl;
  // }

  int absoluteMax = -1;

  // apply rules from the last component
  for (auto it = sccClass.rbegin(); it != sccClass.rend(); it++){

    // get all direct scc successors
    std::vector<SccClassif> succ;
    bool skip = false;
    for (auto it2 = it; it2 >= sccClass.rbegin(); it2--){
      skip = false;
      if (it2 != it){
        for (auto state : it->states){
          if (skip)
            break;
          for (auto state2 : this->getAllSuccessors(state)){
            if (it2->states.find(state2) != it2->states.end()){
              succ.push_back(*it2);
              skip = true;
              break;
            }
          }
        }
      }
    }

    // rule #1: IW -|
    if (succ.size() == 0 and it->inhWeak){
      it->rank = 0;
      it->det = false;
      it->nonDet = false;
    }

    // rule #2: D -|
    else if (succ.size() == 0 and it->det){
      it->rank = 2;
      it->inhWeak = false;
      it->nonDet = false;
    }

    // non-elevator component
    else if (succ.size() == 0 and not(it->det or it->inhWeak or it->nonDet)){
      std::vector<int> intersection;
      std::set_intersection(it->states.begin(), it->states.end(), this->getFinals().begin(), this->getFinals().end(), std::back_inserter(intersection));
      it->rank = 2*(it->states.size() - intersection.size());
    }

    else {
      int rank = -1;

      // rule #3: N
      if (it->nonDet){
        for (auto scc : succ){
          if (scc.nonDet and scc.rank > rank)
            rank = scc.rank;
          else if ((scc.det or scc.inhWeak) and scc.rank+1 > rank)
            rank = scc.rank + 1;
          else if (not (scc.det or scc.inhWeak or scc.nonDet)){
            if (scc.rank%2==0 and scc.rank+1 > rank)
              rank = scc.rank + 1;
            else if (scc.rank%2==1 and scc.rank > rank)
              rank = scc.rank;
          }
        }
        it->rank = rank;
        //n = true;
        it->inhWeak = false;
        it->det = false;
      }

      // rule #4 : IW
      rank = -1;
      if (it->inhWeak){
        for (auto scc : succ){
          if ((scc.det or scc.inhWeak) and scc.rank > rank)
            rank = scc.rank;
          else if (scc.nonDet and scc.rank+1 > rank)
            rank = scc.rank + 1;
          else if (not (scc.det or scc.inhWeak or scc.nonDet)){
            if (scc.rank%2==0 and scc.rank > rank)
              rank = scc.rank;
            else if (scc.rank%2==1 and scc.rank+1 > rank)
              rank = scc.rank + 1;
          }
        }
        if (it->rank == -1 or rank < it->rank){
          it->rank = rank;
          it->nonDet = false;
        }

        it->nonDet = false;
        it->det = false;
      }

      // rule #5: D
      rank = -1;
      if (it->det){
        for (auto scc : succ){
          if (scc.nonDet and scc.rank+1 > rank)
            rank = scc.rank + 1;
          else if ((scc.det or scc.inhWeak) and scc.rank+2 > rank){
            // deterministic transitions -> scc.rank, otherwise scc.rank+2
            bool det = true;
            for (auto state : it->states){
              for (auto a : this->getAlphabet()){
                bool self = false;
                bool other = false;
                for (auto succ : this->getTransitions()[{state, a}]){
                  if (it->states.find(succ) != it->states.end())
                    self = true;
                  if (scc.states.find(succ) != scc.states.end())
                    other = true;
                  if (self and other){
                    det = false;
                    break;
                  }
                }
              }
            }
            if (det and scc.rank > rank)
              rank = scc.rank;
            else if ((not det) and scc.rank+2 > rank)
              rank = scc.rank + 2;
          }
          else if (not (scc.det or scc.inhWeak or scc.nonDet)){
            if (scc.rank%2==0 and scc.rank > rank)
              rank = scc.rank;
            else if (scc.rank%2==1 and scc.rank+1 > rank)
              rank = scc.rank + 1;
          }
        }
        // D rank must be at least 2
        if (rank < 2)
          rank = 2;
        if (it->rank == -1 or rank < it->rank){
          it->rank = rank;
          it->nonDet = false;
          it->inhWeak = false;
        }

        it->nonDet = false;
        it->inhWeak = false;
      }

      // non-elevator scc
      if (not (it->det or it->inhWeak or it->nonDet)){
        int mx = -1;
        for (auto scc : succ){
          if (mx == -1)
            mx = scc.rank;
          else if (mx < scc.rank)
            mx = scc.rank;
        }
        std::vector<int> intersection;
        mx = max(mx, 0);
        std::set_intersection(it->states.begin(), it->states.end(), this->getFinals().begin(), this->getFinals().end(), std::back_inserter(intersection));
        it->rank = mx + 2*(it->states.size() - intersection.size());
      }
    }

    // update absolute max
    if (absoluteMax == -1)
      absoluteMax = it->rank;
    else if (absoluteMax < it->rank)
      absoluteMax = it->rank;
  }

  // deterministic / inherently weak beginning
  if (detBeginning){
    for (auto it = sccClass.begin(); it != sccClass.end(); it++){
      // components with absolute max rank
      if (it->rank == absoluteMax){
        if (it->inhWeak){
          it->rank = 0;
        }
        else if (it->det){
          it->detBeginning = true;
        }
        else if (it->nonDet){
          if (this->isInherentlyWeak(it->states)){
            it->rank = 0;
          }
          else if (this->isDeterministic(it->states)){
            it->detBeginning = true;
          }
        }
      }
    }
  }

  std::map<int, int> ranks;
  for (auto scc : sccClass){
    for (auto state : scc.states){
      if (scc.detBeginning){
        if (this->getFinals().find(state) != this->getFinals().end())
          ranks.insert({state, 0});
        else
          ranks.insert({state, 1});
      } else {
        ranks.insert({state, scc.rank});
      }
    }
  }

  // output original automaton with ranks
  //std::cerr << this->toHOA(ranks) << std::endl;

  return ranks;

  // update rank upper bound
  /*
  for (auto macrostate : nfaSchewe.getStates()){
    if (macrostate.S.size() > 0){
      // pick max
      bool first = true;
      unsigned max = 2*(macrostate.S.size()) - 1;
      bool bad = false;
      for (auto state : macrostate.S){
        if (first){
          for (auto scc : sccClass){
            if (scc.states.find(state) != scc.states.end()){
              max = scc.rank;
              if (detBeginning and scc.detBeginning){
                if (this->getFinals().find(state) != this->getFinals().end())
                  max = 0;
                else
                  max = 1;
              }
              break;
            }
          }
          first = false;
        } else {
          for (auto scc : sccClass){
            if (scc.states.find(state) != scc.states.end()){
              if (detBeginning and scc.detBeginning){
                if (this->getFinals().find(state) == this->getFinals().end() and max < 1)
                  max = 1;
              }
              else if (scc.rank > max)
                max = scc.rank;
              break;
            }
          }
        }
      }
      // update rank upper bound if lower
      if (!bad && this->rankBound[macrostate.S].bound > (int)max){
        this->rankBound[macrostate.S].bound = max;
      }
    }
  }
  */
}
