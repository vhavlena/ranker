#ifndef _GENERALIZED_BA_H_
#define _GENERALIZED_BA_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <functional>
#include <numeric>
#include <chrono>

#include "AutGraph.h"
#include "AutomatonStruct.h"
#include "BuchiAutomaton.h"
#include "../Complement/StateKV.h"
#include "../Complement/StateSch.h"
#include "../Algorithms/AuxFunctions.h"
#include "APSymbol.h"


template <typename State, typename Symbol>
class GeneralizedBuchiAutomaton : public AutomatonStruct<State, Symbol> {

public:
  typedef std::set<State> SetStates;
  typedef std::set<Symbol> SetSymbols;
  typedef Delta<State, Symbol> Transitions;

private:
  SetStates finals; //Modify according to GBAs


protected:
  std::string toStringWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr);
  std::string toGraphwizWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr);
  std::string toGffWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr);

public:
  GeneralizedBuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans) : AutomatonStruct<State, Symbol>(st, ini, trans)
  {
    this->finals = fin;
  }

  GeneralizedBuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans, SetSymbols alp) : AutomatonStruct<State, Symbol>(st, ini, trans, alp)
  {
    this->finals = fin;
  }

  GeneralizedBuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans, SetSymbols alp, map<string, int> aps) : AutomatonStruct<State, Symbol>(st, ini, trans, alp, aps)
  {
    this->finals = fin;
  }

  GeneralizedBuchiAutomaton() : GeneralizedBuchiAutomaton({}, {}, {}, {}) {};

  GeneralizedBuchiAutomaton(GeneralizedBuchiAutomaton<State, Symbol>& other)
  {
    this->states = other.states;
    this->finals = other.finals;
    this->trans = other.trans;
    this->initials = other.initials;
    this->alph = other.alph;
    this->renameStateMap = other.renameStateMap;
    this->renameSymbolMap = other.renameSymbolMap;
    this->invRenameMap = other.invRenameMap;
    this->apsPattern = other.apsPattern;
  }

  std::string toString();
  std::string toGraphwiz();
  std::string toGff();
  std::string toHOA();
  BuchiAutomaton<int, int> renameAut(int start = 0);
  BuchiAutomaton<int, int> renameAutDict(map<Symbol, int>& mpsymbol, int start = 0);

  //bool isElevator();

  /*
   * Rename symbols of the automaton.
   * @param mpsymbol Map assigning to each original state a new state
   * @return Renamed automaton
   */
  template<typename NewSymbol>
  BuchiAutomaton<State, NewSymbol> renameAlphabet(map<Symbol, NewSymbol>& mpsymbol)
  {
    std::set<NewSymbol> ralph;
    Delta<State, NewSymbol> rtrans;
    for(const auto& al : this->alph)
    {
      ralph.insert(mpsymbol[al]);
    }
    for(auto p : this->trans)
    {
      NewSymbol val = mpsymbol[p.first.second];
      rtrans.insert({std::make_pair(p.first.first, val), p.second});
    }
    auto ret = BuchiAutomaton<State, NewSymbol>(this->states, this->finals, this->initials, rtrans, ralph, this->apsPattern);
    ret.setAPPattern(this->apsPattern);
    return ret;
  }

  /*
   * Get automaton final states.
   * @return Set of final states
   */
  SetStates& getFinals()
  {
    return this->finals;
  }


  void removeUseless();
  bool isEmpty();

  BuchiAutomaton<tuple<State, int, bool>, Symbol> productGBA(BuchiAutomaton<int, Symbol>& other);
  BuchiAutomaton<pair<State, int>, Symbol> cartProductGBA(BuchiAutomaton<int, Symbol>& other);
  BuchiAutomaton<State, Symbol> unionGBA(BuchiAutomaton<State, Symbol>& other);
  BuchiAutomaton<State, Symbol> reverseGBA();

};

#endif
