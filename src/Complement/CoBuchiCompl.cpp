
#include "CoBuchiCompl.h"

BuchiAutomaton<StateGcoBA, int> CoBuchiAutomatonCompl::complementCoBA()
{
  //assert(false && "complementCoBA is not implemented");

  auto alph = this->getAlphabet();
  map<std::pair<StateGcoBA, int>, set<StateGcoBA>> mp;
  std::set<StateGcoBA> states;
  std::set<StateGcoBA> finals;

  auto finalStates = this->getFinals()[0];

  // initial states
  std::set<StateGcoBA> initials;
  StateGcoBA init = {.S = this->getInitials(), .B = set<int>(), .i = 0};
  init.B.clear();
  std::set_difference(this->getInitials().begin(), this->getInitials().end(), finalStates.begin(), finalStates.end(), std::inserter(init.B, init.B.begin()));
  initials.insert(init);
  states.insert(init);
  if (init.B.size() == 0 or init.S.size() == 0)
    finals.insert(init);

  // push initial state on stack
  std::stack<StateGcoBA> stack;
  stack.push(init);
  while (not stack.empty()){
    // pop state from stack
    StateGcoBA state = stack.top();
    stack.pop();

    // generate transitions and new states
    for (int sym : alph){
        auto pr = std::make_pair(state, sym);
        std::set<StateGcoBA> dst;

        auto S_prime = succSet(state.S, sym);
        std::set<int> B_prime;

        if (state.B.empty()){
            std::set_difference(S_prime.begin(), S_prime.end(), finalStates.begin(), finalStates.end(), std::inserter(B_prime, B_prime.begin()));
        } else {
            auto tmp = succSet(state.B, sym);
            std::set_difference(tmp.begin(), tmp.end(), finalStates.begin(), finalStates.end(), std::inserter(B_prime, B_prime.begin()));
        }

        StateGcoBA newState = {.S = S_prime, .B = B_prime, .i = 0};

        // push new state on stack
        dst.insert(newState);
        if (states.find(newState) == states.end()){
            states.insert(newState);
            stack.push(newState);
            if ((newState.B.size() == 0) or newState.S.size() == 0)
              finals.insert(newState);
        }

        // transitions
        mp.insert({pr, dst});
    }
  }

  auto ret = BuchiAutomaton<StateGcoBA, int>(states, finals, initials, mp, alph, getAPPattern());
  //std::cerr << ret.toString() << std::endl;
  return ret;
}

BuchiAutomaton<StateGcoBA, int> CoBuchiAutomatonCompl::complementCoBASim(ComplOptions opt){

  /*for (auto pr : this->getWeakDirSim()){
    std::cout << pr.first << " < " << pr.second << std::endl;
  }*/

  Relation<int> sim;
  set<int> (CoBuchiAutomatonCompl::*getSet)(set<int>&, Relation<int>&) = NULL;
  if (opt.iwSim){
    sim = this->getWeakDirSim();
    getSet = &CoBuchiAutomatonCompl::getDirectSet;
  }
  else if (opt.iwSat){
    sim = this->getDirSim();
    getSet = &CoBuchiAutomatonCompl::getSatSet;
  }

  auto alph = this->getAlphabet();
  map<std::pair<StateGcoBA, int>, set<StateGcoBA>> mp;
  std::set<StateGcoBA> states;
  std::set<StateGcoBA> finals;

  // initial states
  std::set<StateGcoBA> initials;
  auto initialsSim = (this->*getSet)(this->getInitials(), sim);
  StateGcoBA init = {.S = initialsSim, .B = set<int>(), .i = 0};
  init.B.clear();
  std::set_difference(initialsSim.begin(), initialsSim.end(), this->getFinals()[0].begin(), this->getFinals()[0].end(), std::inserter(init.B, init.B.begin()));
  initials.insert(init);
  states.insert(init);
  if (init.B.size() == 0 or init.S.size() == 0)
    finals.insert(init);

  // push initial state on stack
  std::stack<StateGcoBA> stack;
  stack.push(init);
  while (not stack.empty()){
    // pop state from stack
    StateGcoBA state = stack.top();
    stack.pop();

    // generate transitions and new states
    for (int sym : alph){
        auto pr = std::make_pair(state, sym);
        std::set<StateGcoBA> dst;

        auto ssucc = succSet(state.S, sym);
        auto S_prime = (this->*getSet)(ssucc, sim);
        std::set<int> B_prime;
        if (state.B.empty()){
            std::set_difference(S_prime.begin(), S_prime.end(), this->getFinals()[0].begin(), this->getFinals()[0].end(), std::inserter(B_prime, B_prime.begin()));
        } else {
            auto tmp = succSet(state.B, sym);
            std::vector<int> intersectionSet;
            std::set_intersection(tmp.begin(), tmp.end(), S_prime.begin(), S_prime.end(), std::back_inserter(intersectionSet));
            std::set_difference(intersectionSet.begin(), intersectionSet.end(), this->getFinals()[state.i].begin(), this->getFinals()[state.i].end(), std::inserter(B_prime, B_prime.begin()));
        }

        StateGcoBA newState = {.S = S_prime, .B = B_prime, .i = 0};

        // push new state on stack
        dst.insert(newState);
        if (states.find(newState) == states.end()){
            states.insert(newState);
            stack.push(newState);
            if ((newState.B.size() == 0) or newState.S.size() == 0)
              finals.insert(newState);
        }

        // transitions
        mp.insert({pr, dst});
    }
  }

  auto ret = BuchiAutomaton<StateGcoBA, int>(states, finals, initials, mp, alph, getAPPattern());
  //std::cerr << ret.toString() << std::endl;
  return ret;
}

set<int> CoBuchiAutomatonCompl::succSet(set<int>& states, int symbol)
{
  set<int> ret;
  for(int st : states)
  {
    set<int> dst = getTransitions()[std::make_pair(st, symbol)];
    ret.insert(dst.begin(), dst.end());
  }
  return ret;
}

set<int> CoBuchiAutomatonCompl::getDirectSet(set<int>& states, Relation<int>& dirSim)
{
  // remove smaller states w.r.t. direct simulation on weak automaton
  std::set<int> retStates;
  for (const auto& state : states){
    if (not std::any_of(dirSim.begin(), dirSim.end(), [state, states](std::pair<int, int> pr){ return state == pr.first and state != pr.second and states.find(pr.second) != states.end(); }))
      retStates.insert(state);
  }

  return retStates;
}

set<int> CoBuchiAutomatonCompl::getSatSet(set<int>& allStates, Relation<int>& dirSim){
  // add smaller states w.r.t. direct simulation on weak automaton
  std::set<int> retStates = allStates;
  for (const auto& pr : dirSim){
    if (allStates.find(pr.second) != allStates.end())
      retStates.insert(pr.first);
  }

  return retStates;
}
