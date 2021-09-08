
#include "BuchiDelay.h"

template<typename Symbol>
unsigned BuchiAutomatonDelay<Symbol>::getTransitionsToTight(){
  unsigned count = 0;
  for (auto trans : this->trans){
    if (!trans.first.first.tight){
      for (auto succ : trans.second){
        if (succ.tight)
          count++;
      }
    }
  }
  return count;
}

int fact(int n)
{
    int res = 1;
    for (int i = 2; i <= n; i++)
        res = res * i;
    return res;
}

int nCr(int n, int r)
{
    return fact(n) / (fact(r) * fact(n - r));
}

template<typename Symbol>
unsigned BuchiAutomatonDelay<Symbol> :: getAllPossibleRankings(unsigned maxRank, unsigned accStates, unsigned nonAccStates, DelayVersion version){

  // OLD VERSION
  if (version == oldVersion){
    unsigned even = std::pow(((maxRank+1)/2), accStates);
    unsigned odd = 0;
    unsigned tmpSum = 1;
    unsigned tmp;
    unsigned upperBound = nonAccStates - (maxRank-1)/2;
    unsigned innerUpperBound;
    for (unsigned r=0; r<=(maxRank-1)/2; r++){
      // every rank
      innerUpperBound = nonAccStates - (maxRank-1)/2 + 1 + r;
      tmp = [](unsigned upperBound, unsigned innerUpperBound, unsigned tmpSum){
        unsigned tmpVar = 0;
        for (unsigned i=1; i<=upperBound; i++){
          tmpVar += (nCr(innerUpperBound, i) * tmpSum);
        }
        return tmpVar;
      }(upperBound, innerUpperBound, tmpSum);
      tmpSum = tmp;
      odd = tmp;
    }

    return odd*even;
  }

  // NEW VERSION
  else if (version == newVersion or version == subsetVersion){
    unsigned even = std::pow(((maxRank+1)/2), accStates);
    unsigned oddSum = 0;
    unsigned rank;

    for (unsigned count = 0; count < (maxRank+1)/2; count++){
      unsigned odd = 0;
      unsigned tmpSum = 1;
      unsigned tmp;
      rank = 2*count + 1;
      unsigned upperBound = nonAccStates - (rank-1)/2;
      unsigned innerUpperBound;

      if (nonAccStates < (rank-1)/2)
        continue;
      if (rank == 1 and nonAccStates > 0)
        odd = 1;
      for (unsigned r=0; r<(rank-1)/2; r++){
        innerUpperBound = nonAccStates - (rank-1)/2 + 1 + r;

        tmp = [](unsigned upperBound, unsigned innerUpperBound, unsigned tmpSum){
          unsigned tmpVar = 0;
          for (unsigned i=1; i<=upperBound; i++){
            tmpVar += (nCr(innerUpperBound, i) * tmpSum);
          }
          return tmpVar;
        }(upperBound, innerUpperBound, tmpSum);

        tmpSum = tmp;
        odd = tmp;
      }

      oddSum += odd;
    }

    return oddSum*even;
  }

  // VERSION WITH STIRLING NUMBERS
  else {
    unsigned even = std::pow(((maxRank+1)/2), accStates);
    unsigned count = (maxRank+1)/2;
    unsigned odd = 0;

    for (unsigned tmpCount = 1; tmpCount <= count; tmpCount++){
      odd += [nonAccStates, tmpCount](){
        unsigned tmp = 0;
        for (unsigned i=0; i<=tmpCount; i++){
          int sign = 1;
          if (i%2 == 1)
            sign = -1;
          tmp += sign * nCr(tmpCount, i) * std::pow(tmpCount-i, nonAccStates);
        }
        return tmp;
      }();
    }

    return odd * even;
  }
}

/*
 * Get states closing a cycle in the automaton graph
 * @params slignore States containing self-loops to be ignored
 * @params dmap Delay map (mapping assigning information about macrostates for the Delay optimization)
 * @return Set of states and symbols for which transitions to the tight part should be generated
 */
template <typename Symbol>
std::map<StateSch, std::set<Symbol>> BuchiAutomatonDelay<Symbol> :: getCycleClosingStates(set<StateSch>& slignore, DelayMap<StateSch>& dmap, double w, DelayVersion version, Stat *stats) {

  std::map<StateSch, std::set<Symbol>> statesToGenerate;
  std::vector<std::vector<StateSch>> allCycles;
  std::map<StateSch, double> mapping;
  std::set<StateSch> successors;
  std::set<StateSch> allStates = this->getStates();
  std::set<StateSch> tmpStates;
  std::vector<std::vector<StateSch>> tmpCycles;
  std::set<StateSch> cycleSucc;
  StateSch minState;
  auto trans = this->getTransitions();
  srand(time(0));

  // get all cycles
  auto start = std::chrono::high_resolution_clock::now();
  allCycles = this->getAllCycles();
  auto end = std::chrono::high_resolution_clock::now();
  stats->getAllCycles = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

  // states to generate
  start = std::chrono::high_resolution_clock::now();
  while (not allStates.empty()){

    // NORMAL
    if (version == oldVersion or version == newVersion or version == subsetVersion or version == stirlingVersion){
      mapping.clear();
      // number for every state
      for (auto state : allStates){
        successors = this->getAllSuccessors(state); // all successors
        std::set<StateSch> tmpSucc = successors;

        for (auto succ : successors){
          if (not std::any_of(allCycles.begin(), allCycles.end(), [state, succ](std::vector<StateSch> item){
            return [state, succ, item](){
              for (unsigned i=0; i<item.size()-1; i++){
                if (item[i] == state and item[i+1] == succ)
                  return true;
              }
              return false;
            }();
          })){tmpSucc.erase(succ);}
        }
        successors = tmpSucc;

        unsigned rankings = [successors, this, state, version](DelayMap<StateSch> &dmap){
            unsigned result = 0;
            for (auto succ : successors){
              if (dmap[succ].maxRank != 0){
                result += this->getAllPossibleRankings(dmap[succ].maxRank, dmap[succ].macrostateSize - dmap[succ].nonAccStates, dmap[succ].nonAccStates, version);
              }
            }
            return result;
          }(dmap);

        // number of cycles with this state that are not covered yet
        unsigned cycles = [allCycles, state](){
          unsigned tmp = 0;
          for (std::vector<StateSch> cycle : allCycles){
            if (std::find(cycle.begin(), cycle.end(), state) != cycle.end())
              tmp++;
          }
          return tmp;
        }();

        double result = [rankings, cycles, w](){
          if (cycles == 0)
            return 0.0;
          //return ((double)rankings)/cycles;
          return std::pow(rankings, w) / std::pow(cycles, 1-w);
        }();
        mapping.insert(std::pair<StateSch, double>(state, result));
      }

      // remove states with 0.0
      tmpStates = allStates;
      bool removed = false;
      for (auto state : allStates){
        if (mapping[state] == 0.0){
          removed = true;
          // add them to statesToGenerate with no symbol
          std::set<Symbol> tmpSymbols;
          statesToGenerate.insert(std::pair<StateSch, std::set<Symbol>>(state, tmpSymbols));
          tmpStates.erase(state);
          // remove all cycles with this state
          tmpCycles.clear();
          for (std::vector<StateSch> cycle : allCycles){
            if (std::find(cycle.begin(), cycle.end(), state) == cycle.end())
              tmpCycles.push_back(cycle);
          }
          allCycles = tmpCycles;
        }
      }
      allStates = tmpStates;
      if (removed)
        continue;

      // pick min
      double min = -1.0;
      bool first = true;
      if (allStates.size() == 0)
        break;
      for (auto state : allStates){
        if (first or (mapping[state] <= min and dmap[state].maxRank < dmap[minState].maxRank)) {
          minState = state;
          min = mapping[state];
          first = false;
        }
      }

      if (version == subsetVersion){
        if constexpr (std::is_same<StateSch, StateSch>::value){
          auto tmpStates = allStates;
          auto revTrans = this->getReverseTransitions();
          for (auto succ : cycleSucc){
            for (auto state : allStates){
              std::set<int> tmpSet;
              std::set_difference(state.S.begin(), state.S.end(), succ.S.begin(), succ.S.end(), std::inserter(tmpSet, tmpSet.begin()));
              if (tmpSet.size() == 0 and std::find(tmpStates.begin(), tmpStates.end(), state) != tmpStates.end())
                minState = state;
            }
          }
          allStates = tmpStates;
        }
      }
    }

    // RANDOM
    else if (version == randomVersion){
      // randomly pick one state to generate
      auto it = allStates.cbegin();
      int random = rand() % allStates.size();
      std::advance(it, random);
      minState = *it;
    }

    // which transitions should be generated
    std::set<Symbol> symbols;
    cycleSucc.clear();
    for (auto cycle : allCycles){
      for (unsigned i = 0; i < cycle.size()-1; i++){
        if (cycle[i] == minState){
          cycleSucc.insert(cycle[i+1]);
          break;
        }
      }
    }

    for (auto succ : cycleSucc){
      for (auto a : this->getAlphabet()){
        std::set<StateSch> reachStates = trans[std::pair<StateSch, Symbol>(minState, a)];
        if (reachStates.find(succ) != reachStates.end())
          symbols.insert(a);
      }
    }

    statesToGenerate.insert(std::pair<StateSch, std::set<Symbol>>(minState, symbols));
    // remove all cycles with this state
    tmpCycles.clear();
    for (std::vector<StateSch> cycle : allCycles){
      if (std::find(cycle.begin(), cycle.end(), minState) == cycle.end())
        tmpCycles.push_back(cycle);
    }
    allCycles = tmpCycles;
    allStates.erase(minState);

  }

  end = std::chrono::high_resolution_clock::now();
  stats->statesToGenerate = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

  return statesToGenerate;
}

template<typename Symbol>
void BuchiAutomatonDelay<Symbol> :: unblock(int state, std::set<int> &blockedSet, std::map<int, std::set<int>> &blockedMap){
  blockedSet.erase(state); //
  for (auto w : blockedMap[state]){
    blockedMap[state].erase(w); // delete w from blocked maps
    if (blockedSet.find(w) != blockedSet.end()){ // w is in blockedSet
      //blockedSet.erase(w); //!
      this->unblock(w, blockedSet, blockedMap); //
    }
  }
}

template<typename Symbol>
bool BuchiAutomatonDelay<Symbol> :: circuit(int state, std::vector<int> &stack, std::set<int> &blockedSet, std::map<int,
  std::set<int>> &blockedMap, std::set<int> scc, AdjList adjlist, int startState, std::vector<std::vector<int>> &allCyclesRenamed) {
  bool flag = false;
  stack.push_back(state);
  blockedSet.insert(state);

  // for every successor
  for (auto succ : adjlist[state]){
    if (succ == startState){
      // cycle was closed -> states on stack
      flag = true;
      std::vector<int> cycle;
      for (auto state : stack){
        cycle.push_back(state);
      }
      if (not cycle.empty()){
        cycle.push_back(startState);
        allCyclesRenamed.push_back(cycle); // add new cycle to the vector of cycles
      }
    }
    else if (blockedSet.find(succ) == blockedSet.end()){ // succ is not in blockedSet
      // successor is not in the blocked set
      if (this->circuit(succ, stack, blockedSet, blockedMap, scc, adjlist, startState, allCyclesRenamed)){
        flag = true;
      }
    }
  }

  if (flag){
    // unblock ...
    this->unblock(state, blockedSet, blockedMap);
  } else {
    // for every successor
    for (auto succ : adjlist[state]){
      if (blockedMap[state].find(succ) == blockedMap[state].end()){ // succ is not in blockedMap[state]
        blockedMap[state].insert(succ);
      }
    }
  }

  stack.pop_back(); // remove last element from stack
  return flag;
}

template<typename Symbol>
std::vector<std::vector<StateSch>> BuchiAutomatonDelay<Symbol> :: getAllCycles(){
  BuchiAutomaton<int, int> renAutBA = this->renameAut();

  vector<vector<int>> adjList(this->getStates().size());
  vector<VertItem> vrt;
  vector<set<StateSch>> sccs;

  renAutBA.getAutGraphComponents(adjList, vrt);
  AutGraph gr(adjList, vrt, renAutBA.getFinals());
  gr.computeSCCs(); // all sccs

  std::vector<std::vector<int>> allCyclesRenamed;
  std::vector<std::vector<StateSch>> allCycles;
  const std::set<int> emptySet;
  std::vector<std::set<int>> tmpVector;

  for(auto& scc : gr.getAllComponents()){ // for every scc
    auto tmpScc = scc;
    for (auto &state : scc){ // for every state in scc
      std::vector<int> stack;
      std::set<int> blockedSet;
      std::map<int, std::set<int>> blockedMap;

      // insert all states in scc to blockedMap
      for(auto &state : scc){
        blockedMap.insert(std::pair<int, std::set<int>>(state, emptySet));
      }

      // circuit method: returns all cycles in allCyclesRenamed
      this->circuit(state, stack, blockedSet, blockedMap, tmpScc, adjList, state, allCyclesRenamed);

      tmpScc.erase(state);
      adjList[state].erase(std::remove(adjList[state].begin(), adjList[state].end(), state), adjList[state].end());
    }
  }

  for (auto &cycle : allCyclesRenamed){
    std::vector<StateSch> oneCycle;
    for (auto &state : cycle){
      oneCycle.push_back(this->getInvRenameSymbolMap()[state]);
    }
    allCycles.push_back(oneCycle);
  }

  return allCycles;
}

template class BuchiAutomatonDelay<int>;
