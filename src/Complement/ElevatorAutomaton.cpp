
#include "ElevatorAutomaton.h"

bool ElevatorAutomaton::isDeterministic(std::set<int>& scc, map<int, set<int> >& predSyms){
  for (auto state : scc){
    for (auto a : predSyms[state]){
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


bool ElevatorAutomaton::isNonDeterministic(std::set<int>& scc){
  if (std::any_of(scc.begin(), scc.end(), [this](int state){return this->getFinals().find(state) != this->getFinals().end();}))
    return false;
  else if (std::any_of(this->getFinTrans().begin(), this->getFinTrans().end(), [scc](auto trans){return scc.find(trans.from) != scc.end() and scc.find(trans.to) != scc.end();}))
    return false;
  else
    return true;
}


bool ElevatorAutomaton::isInherentlyWeak(std::set<int>& scc, map<int, set<int> >& predSyms){
  SetStates st;
  SetStates fin = this->getFinals();
  SetStates empty;
  SetStates ini = this->getInitials();
  SetStates newIni;
  VecTransG finTrans = this->getFinTrans();

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
    if (st.find(it->first.first) != st.end()){
      SetStates newStates;
      // remove accepting transitions
      for (auto state : it->second){
        Transition<int, int> tmp = {.from = it->first.first, .to = state, .symbol = it->first.second};
        if (std::find(finTrans.begin(), finTrans.end(), tmp) == finTrans.end())
          newStates.insert(state);
      }
      newTrans.insert({it->first, newStates});
    }
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
      for (auto symbol : predSyms[state]){
        auto reach = this->getTransitions()[{state, symbol}];
        if (reach.find(state) != reach.end())
          return false;
      }
    }
  }

  return true;
}

BuchiAutomaton<int ,int> ElevatorAutomaton::propagateAccStates(){
  BuchiAutomaton<int, int> ret(*this);
  set<int> fins = ret.getFinals();
  auto finTrans = this->getFinTrans();

  map<int, set<int>> predSyms = this->getPredSymbolMap();
  map<int, set<int>> revSyms = this->getReverseSymbolMap();
  vector<set<int>> sccs = this->getAutGraphSCCs();

  bool change;
  for (auto scc : sccs){
    do {
      change = false;
      for (auto state : scc){
        if (fins.find(state) == fins.end()){
          auto succ = ret.getAllSuccessors(state, predSyms);
          auto symbols = predSyms[state];
          // all successors/predecessors in given scc are accepting
          if (std::all_of(succ.begin(), succ.end(), [scc, fins, symbols, state, finTrans](int succState){return scc.find(succState) == scc.end() or fins.find(succState) != fins.end()
            or not std::any_of(symbols.begin(), symbols.end(), [state, succState, finTrans](int symbol){Transition<int, int> tmp = {.from = state, .to = succState, .symbol = symbol}; return std::find(finTrans.begin(), finTrans.end(), tmp) == finTrans.end();});})){
            change = true;
            fins.insert(state);
          } else {
            auto pred = ret.getAllPredecessors(state, revSyms);
            auto symbols = revSyms[state];
            if (std::all_of(pred.begin(), pred.end(), [scc, fins, symbols, state, finTrans](int predState){return scc.find(predState) == scc.end() or fins.find(predState) != fins.end()
              or not std::any_of(symbols.begin(), symbols.end(), [state, predState, finTrans](int symbol){Transition<int, int> tmp = {.from = predState, .to = state, .symbol = symbol}; return std::find(finTrans.begin(), finTrans.end(), tmp) == finTrans.end();});})){
              change = true;
              fins.insert(state);
            }
          }
        }
      }
    } while (change);
  }

  ret.setFinals(fins);
  return ret;
}

BuchiAutomaton<int, int> ElevatorAutomaton::copyPreprocessing(const std::function<bool(SccClassif)>& pred)
{
  BuchiAutomaton<int, int> aut(*this);
  BuchiAutomaton<int, int> ret(*this);
  set<int> newfins = set<int>(ret.getFinals().begin(), ret.getFinals().end());
  set<int> tmpFins;
  // get all sorted sccs
  map<int, set<int> > predSyms = this->getPredSymbolMap();
  std::vector<std::set<int>> sccs = this->topologicalSort(predSyms);
  std::vector<SccClassif> sccClass;
  for (auto scc : sccs){
    SccClassif tmp = {.states = scc, .det = false, .inhWeak = false, .nonDet = false};
    sccClass.push_back(tmp);
  }

  // scc classification
  for (auto it = sccClass.begin(); it != sccClass.end(); it++){
    // deterministic
    if (isDeterministic(it->states, predSyms))
      it->det = true;
    // nondeterministic
    if (isNonDeterministic(it->states))
      it->nonDet = true;
    // inherently weak
    if (isInherentlyWeak(it->states, predSyms))
      it->inhWeak = true;
  }

  int start = this->getStates().size();
  for (auto it = sccClass.begin(); it != sccClass.end(); it++)
  {
    auto allsub = this->getAllSuccessors(it->states, predSyms);
    bool term = std::includes(it->states.begin(), it->states.end(), allsub.begin(), allsub.end());

    if(pred(*it) && !term)
    {
      tmpFins.clear();
      std::set_difference(newfins.begin(), newfins.end(), it->states.begin(), it->states.end(),
          std::inserter(tmpFins, tmpFins.begin()));
      newfins = tmpFins;
      aut = BuchiAutomaton<int, int>(*this);
      aut.restriction(it->states);

      BuchiAutomaton tmp = aut.copyStateAcc(start);
      tmpFins.clear();
      std::set_union(newfins.begin(), newfins.end(), tmp.getFinals().begin(), tmp.getFinals().end(),
          std::inserter(tmpFins, tmpFins.begin()));
      newfins = tmpFins;
      ret = ret.unionBA(tmp);

      start += it->states.size();
    }
  }

  ret.setFinals(newfins);

  return ret;
}


bool ElevatorAutomaton::isElevator(){

  // get all sorted sccs
  map<int, set<int> > predSyms = this->getPredSymbolMap();
  std::vector<std::set<int>> sccs = this->topologicalSort(predSyms);
  std::vector<SccClassif> sccClass;
  for (auto scc : sccs){
    SccClassif tmp = {.states = scc, .det = false, .inhWeak = false, .nonDet = false};
    sccClass.push_back(tmp);
  }

  // scc classification
  for (auto it = sccClass.begin(); it != sccClass.end(); it++){
    // deterministic
    if (isDeterministic(it->states, predSyms))
    {
      it->det = true;
    }
    // nondeterministic
    else if (isNonDeterministic(it->states))
    {
      it->nonDet = true;
    }
    // inherently weak
    else if (isInherentlyWeak(it->states, predSyms))
    {
      it->inhWeak = true;
    }
    else
    {
      return false;
    }
  }

  return true;
}


void ElevatorAutomaton::topologicalSortUtil(std::set<int> currentScc, std::vector<std::set<int>> allSccs, std::map<std::set<int>, bool> &visited, std::stack<std::set<int>> &Stack, vector<set<int>>& adjList){
  // mark the current node as visited
  visited[currentScc] = true;
  auto trans = this->getTransitions();

  // recursion call for all nonvisited successors
  for (auto& scc : allSccs){
    bool cnt = true;
    if (not visited[scc]){
      for (auto state : currentScc){
        for(auto m : scc)
        {
          if(adjList[state].find(m) != adjList[state].end())
          {
            this->topologicalSortUtil(scc, allSccs, visited, Stack, adjList);
            cnt = false;
            break;
          }
        }
        if(!cnt)
          break;
      }
      //   for (auto a : this->getAlphabet()){
      //     if(trans[{state, a}].size() == 0)
      //       continue;
      //     if (std::any_of(scc.begin(), scc.end(), [this, state, a](int succ){auto trans = this->getTransitions(); return trans[{state, a}].find(succ) != trans[{state, a}].end();}))
      //       this->topologicalSortUtil(scc, allSccs, visited, Stack);
      //   }
      // }
    }
  }

  // push scc to stack storing the topological order
  Stack.push(currentScc);
}

std::vector<std::set<int>> ElevatorAutomaton::topologicalSort(std::map<int, std::set<int>>& predSyms){
  // get all sccs
  vector<set<int>> adjList(this->states.size());
  std::vector<std::set<int>> sccs = this->getAutGraphSCCs();
  auto tr = this->getTransitions();

  for(const auto & st : this->getStates())
  {
    for (const auto& a : predSyms[st])
    {
      for(const auto & d : tr[{st, a}])
        adjList[st].insert(d);
    }
  }


  std::stack<std::set<int>> Stack;
  // no scc is visited
  std::map<std::set<int>, bool> visited;
  for (auto& scc : sccs){
    visited.insert({scc, false});
  }

  // for(int i = 0; i < adjList.size(); i++)
  //   cout << adjList[i].size() << endl;


  // get topological sort starting from all sccs one by one
  for (auto& scc : sccs){
    if (visited[scc] == false)
      this->topologicalSortUtil(scc, sccs, visited, Stack, adjList);
    //std::cerr << "size: " << scc.size() << std::endl;
  }

  //cout << "scc complete " << visited.size() << " " << adjList.size() << endl;

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
  map<int, set<int> > predSyms = this->getPredSymbolMap();
  std::vector<std::set<int>> sortedComponents = this->topologicalSort(predSyms);

  // determine scc type (deterministic, nondeterministic, bad, both)
  std::map<std::set<int>, sccType> typeMap;
  for (auto scc : sortedComponents){
    // is scc deterministic?
    bool det = true;
    for (auto state : scc){
      if (not det)
        break;
      for (auto a : predSyms[state]){
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

  map<int, set<int> > predSyms = this->getPredSymbolMap();

  // get all sorted sccs
  std::vector<std::set<int>> sccs = this->topologicalSort(predSyms);
  std::vector<SccClassif> sccClass;
  for (auto& scc : sccs){
    SccClassif tmp = {.states = scc, .det = false, .inhWeak = false, .nonDet = false};
    sccClass.push_back(tmp);
  }

  // scc classification
  for (auto it = sccClass.begin(); it != sccClass.end(); it++){
    // deterministic
    if (isDeterministic(it->states, predSyms))
    {
      it->det = true;
    }
    // nondeterministic
    if (isNonDeterministic(it->states))
      it->nonDet = true;
    // inherently weak
    if (isInherentlyWeak(it->states, predSyms))
      it->inhWeak = true;
    //cout << it->states.size() << " " << it->det << " " << it->nonDet << " " << it->inhWeak << endl;
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
          for (auto state2 : this->getAllSuccessors(state, predSyms)){
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

    else if (succ.size() == 0 and it->nonDet){
      it->rank = 1;
      it->det = false;
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
          if (scc.nonDet and scc.rank > rank){
            // accepting transition between components
            if (std::any_of(this->getFinTrans().begin(), this->getFinTrans().end(), [scc, it](auto trans){return it->states.find(trans.from) != it->states.end() and scc.states.find(trans.to) != scc.states.end();}))
              rank = scc.rank + 2;
            // no accepting transition between components
            else
              rank = scc.rank;
          }
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
        } else {
          it->inhWeak = false;
        }
      }

      // rule #5: D
      rank = -1;
      if (it->det){
        for (auto scc : succ){
          if (scc.nonDet and scc.rank+1 > rank){
            rank = scc.rank + 1;
          }
          else if ((scc.det or scc.inhWeak) and scc.rank+2 > rank){
            // deterministic transitions -> scc.rank, otherwise scc.rank+2
            bool det = true;
            for (auto state : it->states){
              for (auto a : predSyms[state]){
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
              rank = scc.rank + 2;
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
        } else {
          it->det = false;
        }
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
          if (this->isInherentlyWeak(it->states, predSyms)){
            it->rank = 0;
          }
          else if (this->isDeterministic(it->states, predSyms)){
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
}

bool ElevatorAutomaton::isInherentlyWeakBA()
{
  map<int, set<int>> predSyms = this->getPredSymbolMap();
  auto sccs = this->getAutGraphSCCs();
  auto finals = this->getFinals();
  for (auto scc : sccs){
    if (not isInherentlyWeak(scc, predSyms) and std::any_of(scc.begin(), scc.end(), [finals](int state){return finals.find(state) != finals.end();}))
      return false;
  }

  return true;
}


BuchiAutomaton<int, int> ElevatorAutomaton::convertToWeak()
{
  assert(this->isInherentlyWeakBA());

  assert(false && "convertToWeak is not implemented");

  return BuchiAutomaton<int, int>();
}


map<int, bool> ElevatorAutomaton::nondetStates()
{
  map<int, set<int> > predSyms = this->getPredSymbolMap();
  std::vector<std::set<int>> sccs = this->topologicalSort(predSyms);
  std::vector<SccClassif> sccClass;
  for (const auto& scc : sccs)
  {
    SccClassif tmp = {.states = scc, .det = false, .inhWeak = false, .nonDet = false};
    sccClass.push_back(tmp);
  }

  map<int,int> ranks = this->elevatorRank(false);
  int m = Aux::maxValue(ranks);

  // scc classification
  for (auto it = sccClass.begin(); it != sccClass.end(); it++)
  {
    // deterministic
    if (isDeterministic(it->states, predSyms))
    {
      it->det = true;
    }
    // nondeterministic
    if (isNonDeterministic(it->states))
    {
      it->nonDet = true;
    }
    // inherently weak
    if (isInherentlyWeak(it->states, predSyms))
    {
      it->inhWeak = true;
    }
  }

  map<int, bool> types;
  for (const auto& scc : sccClass)
  {
    for(int st : scc.states)
    {
      if(scc.nonDet && ranks[st] == m)
      {
        types[st] = true;
      }
      else
        types[st] = false;
    }
  }

  return types;
}


BuchiAutomaton<int, int> ElevatorAutomaton::nondetInitDeterminize()
{
  map<int, bool> nondetSt = this->nondetStates();

  map<DFAState, int> stInt;
  int cnt = this->getStates().size();
  std::stack<DFAState> stack;
  set<DFAState> comst;

  set<int> states(this->getStates().begin(), this->getStates().end());

  //set<StateSch> initials;
  //set<StateSch> finals;
  //set<StateSch> succ;
  set<int> alph = getAlphabet();

  Delta<int, int> origTrans = this->getTransitions();
  Delta<int, int> mp(origTrans.begin(), origTrans.end());
  //map<std::pair<StateSch, int>, set<StateSch> >::iterator it;

  DFAState init = this->getInitials();

  for(int st : init)
  {
    if(!nondetSt[st])
      return *this;
  }

  stack.push(init);
  comst.insert(init);
  //initials.insert(init);

  states.insert(cnt);
  stInt[init] = cnt++;

  while(stack.size() > 0)
  {
    DFAState st = stack.top();
    stack.pop();

    if(st.size() == 0)
      continue;

    for(int sym : alph)
    {
      set<int> dst;

      DFAState nt = succSet(st, sym);


      DFAState target;
      for(int s : nt)
      {
        if(nondetSt[s])
        {
          target.insert(s);
        }
        else
        {
          dst.insert(s);
        }
      }

      if(target.size() == 0)
      {
        mp[{stInt[st], sym}] = dst;
        continue;
      }

      auto it = stInt.find(target);
      if(it != stInt.end())
      {
        dst.insert(it->second);
      }
      else
      {
        states.insert(cnt);
        dst.insert(cnt);
        stInt[target] = cnt++;
      }

      if(comst.find(target) == comst.end())
      {
        stack.push(target);
        comst.insert(target);
      }

      mp[{stInt[st], sym}] = dst;
    }
  }

  set<int> fn(this->getFinals().begin(), this->getFinals().end());
  set<int> ini = {stInt[init]};
  BuchiAutomaton<int, int> tmp(states, fn, ini, mp, alph, this->getAPPattern());
  return tmp.renameStates();
}
