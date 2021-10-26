
#include "CoBuchiCompl.h"

BuchiAutomaton<StateGcoBA, int> CoBuchiAutomatonCompl::complementCoBA()
{
  //assert(false && "complementCoBA is not implemented");

  auto alph = this->getAlphabet();
  map<std::pair<StateGcoBA, int>, set<StateGcoBA>> mp;
  std::set<StateGcoBA> states;
  std::set<StateGcoBA> finals;

  // initial states
  std::set<StateGcoBA> initials;
  StateGcoBA init = {.S = this->getInitials(), .B = set<int>(), .i = 0};
  init.B.clear();
  std::set_difference(this->getInitials().begin(), this->getInitials().end(), this->getFinals()[0].begin(), this->getFinals()[0].end(), std::inserter(init.B, init.B.begin()));
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
            std::set_difference(S_prime.begin(), S_prime.end(), this->getFinals()[0].begin(), this->getFinals()[0].end(), std::inserter(B_prime, B_prime.begin()));
        } else {
            auto tmp = succSet(state.B, sym);
            std::set_difference(tmp.begin(), tmp.end(), this->getFinals()[state.i].begin(), this->getFinals()[state.i].end(), std::inserter(B_prime, B_prime.begin()));
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

  return BuchiAutomaton<StateGcoBA, int>(states, finals, initials, mp, alph, getAPPattern());
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
