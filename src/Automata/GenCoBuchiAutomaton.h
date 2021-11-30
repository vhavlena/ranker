#ifndef _GEN_CO_BUCHI_AUTOMATON_H_
#define _GEN_CO_BUCHI_AUTOMATON_H_

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
//#include "BuchiAutomaton.h"
#include "../Complement/StateGcoBA.h"
#include "../Algorithms/AuxFunctions.h"
#include "APSymbol.h"


template <typename State, typename Symbol>
class GeneralizedCoBuchiAutomaton : public AutomatonStruct<State, Symbol> {

public:
  typedef std::set<State> SetStates;
  typedef std::set<Symbol> SetSymbols;
  typedef Delta<State, Symbol> Transitions;
  typedef std::map<int, std::set<State>> SetFins;

private:
  SetFins finals;

protected:
  std::string toStringWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr);
  std::string toGraphwizWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr);
  std::string toGffWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr);

public:
  GeneralizedCoBuchiAutomaton(SetStates st, SetFins fin, SetStates ini, Transitions trans) : AutomatonStruct<State, Symbol>(st, ini, trans)
  {
    this->finals = fin;
  }

  GeneralizedCoBuchiAutomaton(SetStates st, SetFins fin, SetStates ini, Transitions trans, SetSymbols alp) : AutomatonStruct<State, Symbol>(st, ini, trans, alp)
  {
    this->finals = fin;
  }

  GeneralizedCoBuchiAutomaton(SetStates st, SetFins fin, SetStates ini, Transitions trans, SetSymbols alp, vector<string> aps) : AutomatonStruct<State, Symbol>(st, ini, trans, alp, aps)
  {
    this->finals = fin;
  }

  GeneralizedCoBuchiAutomaton() : GeneralizedCoBuchiAutomaton({}, {}, {}, {}) {};

  GeneralizedCoBuchiAutomaton(GeneralizedCoBuchiAutomaton<State, Symbol>& other) : AutomatonStruct<State, Symbol>(other)
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


  GeneralizedCoBuchiAutomaton<State, Symbol> operator=(GeneralizedCoBuchiAutomaton<State, Symbol> other)
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
    return *this;
  }


  std::string toString();
  std::string toGraphwiz();
  std::string toGff();
  std::string toHOA();

  GeneralizedCoBuchiAutomaton<int, int> renameAut(int start = 0) {
    int stcnt = start;
    int symcnt = 0;
    std::map<State, int> mpstate;
    std::map<Symbol, int> mpsymbol;
    std::set<int> rstate;
    Delta<int, int> rtrans;
    std::map<int, std::set<int>> rfin;
    std::set<int> rini;
    set<int> rsym;
    this->invRenameMap = std::vector<State>(this->states.size() + start);

    for(auto st : this->states)
    {
      auto it = mpstate.find(st);
      this->invRenameMap[stcnt] = st;
      if(it == mpstate.end())
      {
        mpstate[st] = stcnt++;
      }
    }
    for(const auto& a : this->alph)
    {
      rsym.insert(symcnt);
      mpsymbol[a] = symcnt++;
    }

    rstate = Aux::mapSet(mpstate, this->states);
    rini = Aux::mapSet(mpstate, this->initials);
    rfin = Aux::mapMap(mpstate, this->finals);
    for(auto p : this->trans)
    {
      auto it = mpsymbol.find(p.first.second);
      int val;
      if(it == mpsymbol.end())
      {
        val = symcnt;
        mpsymbol[p.first.second] = symcnt++;
      }
      else
      {
        val = it->second;
      }
      std::set<int> to = Aux::mapSet(mpstate, p.second);
      rtrans.insert({std::make_pair(mpstate[p.first.first], val), to});
    }

    GeneralizedCoBuchiAutomaton<int, int> ret = GeneralizedCoBuchiAutomaton<int, int>(rstate, rfin, rini, rtrans, rsym);
    this->renameStateMap = mpstate;
    this->renameSymbolMap = mpsymbol;

    ret.setAPPattern(this->apsPattern);
    return ret;
  }

  GeneralizedCoBuchiAutomaton<int, int> renameAutDict(map<Symbol, int>& mpsymbol, int start = 0);

  //bool isElevator();

  /*
   * Rename symbols of the automaton.
   * @param mpsymbol Map assigning to each original state a new state
   * @return Renamed automaton
   */
  template<typename NewSymbol>
  GeneralizedCoBuchiAutomaton<State, NewSymbol> renameAlphabet(map<Symbol, NewSymbol>& mpsymbol)
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
    auto ret = GeneralizedCoBuchiAutomaton<State, NewSymbol>(this->states, this->finals, this->initials, rtrans, ralph, this->apsPattern);
    ret.setAPPattern(this->apsPattern);
    return ret;
  }

  /*
   * Get automaton final states.
   * @return Set of final states
   */
  SetFins& getFinals()
  {
    return this->finals;
  }

  void setFinals(SetFins finals)
  {
    this->finals = finals;
  }

  void removeUseless();
  bool isEmpty();

  GeneralizedCoBuchiAutomaton<tuple<State, int>, Symbol> product(GeneralizedCoBuchiAutomaton<int, Symbol>& other);
  GeneralizedCoBuchiAutomaton<tuple<State, int>, Symbol> cartProduct(GeneralizedCoBuchiAutomaton<int, Symbol>& other);
  GeneralizedCoBuchiAutomaton<State, Symbol> unionGcoBA(GeneralizedCoBuchiAutomaton<State, Symbol>& other);
  GeneralizedCoBuchiAutomaton<State, Symbol> reverse();

  std::vector<std::vector<int>> getAllCycles();
  bool circuit(int state, std::vector<int> &stack, std::set<int> &blockedSet, std::map<int,
    std::set<int>> &blockedMap, std::set<int> scc, AdjList adjlist, int startState, std::vector<std::vector<int>> &allCyclesRenamed);
  void unblock(int state, std::set<int> &blockedSet, std::map<int, std::set<int>> &blockedMap);

  void restriction(set<State>& st);

};

#endif
