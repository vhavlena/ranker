#ifndef _BUCHI_AUTOMATON_H_
#define _BUCHI_AUTOMATON_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>

#include "ComplementBA.h"

template <typename State, typename Symbol>
struct Transition {
  State from;
  State to;
  Symbol symbol;
};

template <typename State, typename Symbol>
class BuchiAutomaton {

public:
  typedef std::set<State> SetStates;
  typedef std::set<Symbol> SetSymbols;
  typedef std::map<std::pair<State, Symbol>, SetStates> Transitions;

private:
  SetStates states;
  SetStates finals;
  SetStates initials;
  SetSymbols alph;
  Transitions trans;

protected:
  std::string toStringWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr);

  template <typename T>
  std::set<int> mapSet(std::map<T, int> mp, std::set<T> st)
  {
    std::set<int> ret;
    for(auto p : st)
      ret.insert(mp[p]);
    return ret;
  }

public:
  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans)
  {
    this->states = st;
    this->finals = fin;
    this->trans = trans;
    this->initials = ini;
    this->alph = getAlph();
  }

  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans, SetSymbols alp)
  {
    this->states = st;
    this->finals = fin;
    this->trans = trans;
    this->initials = ini;
    this->alph = alp;
  }

  BuchiAutomaton() : BuchiAutomaton({}, {}, {}, {}) {};

  BuchiAutomaton(BuchiAutomaton<State, Symbol>& other)
  {
    this->states = other.states;
    this->finals = other.finals;
    this->trans = other.trans;
    this->initials = other.initials;
    this->alph = other.alph;
  }

  std::set<Symbol> getAlph();
  std::string toString();
  BuchiAutomaton<int, int> renameAut();

  SetStates getStates() const
  {
    return this->states;
  }

  SetStates getFinals() const
  {
    return this->finals;
  }

  SetStates getInitials() const
  {
    return this->initials;
  }

  Transitions getTransitions() const
  {
    return this->trans;
  }

  SetSymbols getAlphabet() const
  {
    return this->alph;
  }
};

#endif
