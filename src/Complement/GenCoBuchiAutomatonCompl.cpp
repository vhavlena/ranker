#include "GenCoBuchiAutomatonCompl.h"

BuchiAutomaton<StateGcoBA, int> GeneralizedCoBuchiAutomatonCompl::complementGcoBA(){
    auto alph = this->getAlphabet();
    map<std::pair<StateGcoBA, int>, set<StateGcoBA>> mp;
    std::set<StateGcoBA> states;
    std::set<StateGcoBA> finals;
    
    // initial states
    std::set<StateGcoBA> initials;
    StateGcoBA init = {.S = this->getInitials(), .i = 0};
    init.B.clear();
    std::set_difference(this->getInitials().begin(), this->getInitials().end(), this->getFinals()[0].begin(), this->getFinals()[0].end(), std::inserter(init.B, init.B.begin())); 
    initials.insert(init); 
    states.insert(init);  
    if (init.B.size() == 0)
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
            int i_prime;
            std::set<int> B_prime;
            if (state.B.empty()){
                i_prime = (state.i + 1) % this->getFinals().size();
                std::set_difference(state.S.begin(), state.S.end(), this->getFinals()[i_prime].begin(), this->getFinals()[i_prime].end(), std::inserter(B_prime, B_prime.begin()));
            } else {
                i_prime = state.i;
                auto tmp = succSet(state.B, sym);
                std::set_difference(tmp.begin(), tmp.end(), this->getFinals()[state.i].begin(), this->getFinals()[state.i].end(), std::inserter(B_prime, B_prime.begin()));
            }

            StateGcoBA newState = {.S = S_prime, .B = B_prime, .i = i_prime};

            // push new state on stack
            dst.insert(newState);
            if (states.find(newState) == states.end()){
                states.insert(newState);
                stack.push(newState);
                if (newState.B.size() == 0 and newState.i == 0)
                    finals.insert(newState);
            }

            // transitions
            mp.insert({pr, dst});
        }
    }

    auto ba = BuchiAutomaton<StateGcoBA, int>(states, finals, initials, mp, alph, getAPPattern());
    //std::cerr << ba.toString() << std::endl;
    return ba;
}

/*
 * Set of all successors.
 * @param states Set of states to get successors
 * @param symbol Symbol
 * @return Set of successors over symbol
 */
set<int> GeneralizedCoBuchiAutomatonCompl::succSet(set<int>& states, int symbol)
{
  set<int> ret;
  for(int st : states)
  {
    set<int> dst = getTransitions()[std::make_pair(st, symbol)];
    ret.insert(dst.begin(), dst.end());
  }
  return ret;
}