#ifndef _BUCHI_AUTOMATON_DEBUG_H_
#define _BUCHI_AUTOMATON_DEBUG_H_

#include "../Automata/APSymbol.h"
#include "../Automata/BuchiAutomaton.h"
#include "../Automata/BuchiAutomatonException.h"

template <typename State, typename Symbol>
class BuchiAutomatonDebug : public BuchiAutomaton<State, Symbol>
{
protected:
  BuchiAutomaton<int, Symbol> createWordAutomaton(vector<Symbol>& handle, vector<Symbol>& loop);

public:
  BuchiAutomatonDebug(BuchiAutomaton<State, Symbol> &t) : BuchiAutomaton<State, Symbol>(t) {  }

  BuchiAutomaton<pair<State,int>, Symbol> getSubAutomatonWord(vector<Symbol>& handle, vector<Symbol>& loop);
};

#endif
