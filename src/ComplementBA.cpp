
#include "BuchiAutomaton.h"

template <typename State, typename Symbol>
BuchiAutomaton<StateKV<State>, Symbol> BuchiAutomaton<State, Symbol>::complementKV() const
{
  std::set< StateKV<State> > empty = {{{1,2,3}, {2,3}, {}}};
  std::map<std::pair<StateKV<State>, Symbol>, std::set< StateKV<State> > > mp;
  return BuchiAutomaton<StateKV<State>, Symbol>(empty, empty, empty, mp);
}

template class BuchiAutomaton<int, int>;
