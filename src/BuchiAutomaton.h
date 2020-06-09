#ifndef _BUCHI_AUTOMATON_H_
#define _BUCHI_AUTOMATON_H_

#include <set>
#include <map>
#include <string>
#include <iostream>

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

public:
  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans)
  {
    this->states = st;
    this->finals = fin;
    this->trans = trans;
    this->initials = ini;
    this->alph = getAlph();
  }

  std::set<Symbol> getAlph();
  std::string toString();

};

#endif
