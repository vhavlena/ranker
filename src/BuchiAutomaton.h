#ifndef _BUCHI_AUTOMATON_H_
#define _BUCHI_AUTOMATON_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <functional>

#include "AutGraph.h"
#include "StateKV.h"
#include "StateSch.h"
#include "AuxFunctions.h"

using std::tuple;

class AutGraph;

template <typename State, typename Symbol>
struct Transition {
  State from;
  State to;
  Symbol symbol;
};

template <typename State>
struct LabelState {
  State state;
  int label;
};

template <typename State, typename Symbol>
class BuchiAutomaton {

public:
  typedef std::set<State> SetStates;
  typedef std::set<LabelState<State> > SetLabelStates;
  typedef std::set<LabelState<State>* > SetLabelStatesPtr;
  typedef std::vector<LabelState<State>* > VecLabelStatesPtr;
  typedef std::vector<LabelState<State>> VecLabelStates;
  typedef std::set<Symbol> SetSymbols;
  typedef std::map<std::pair<State, Symbol>, SetStates> Transitions;
  typedef std::set<std::pair<State, State> > StateRelation;

private:
  SetStates states;
  SetStates finals;
  SetStates initials;
  SetSymbols alph;
  Transitions trans;

  StateRelation directSim;
  StateRelation oddRankSim;

  std::map<State, int> renameStateMap;
  std::vector<State> invRenameMap;

protected:
  std::string toStringWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr);
  std::string toGraphwizWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr);

  bool isRankLeq(std::set<State>& set1, std::set<State>& set2, StateRelation& rel);
  bool deriveRankConstr(State& st1, State& st2, StateRelation& rel);
  void propagateFwd(State& st1, State& st2, SetStates& set1, SetStates& set2,
    StateRelation& rel,StateRelation& nw);
  void transitiveClosure(StateRelation& rel, SetStates& cl);

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
    this->directSim = other.directSim;
    this->oddRankSim = other.oddRankSim;
    this->renameStateMap = other.renameStateMap;
    this->invRenameMap = other.invRenameMap;
  }

  std::set<Symbol> getAlph();
  std::string toString();
  std::string toGraphwiz();
  BuchiAutomaton<int, int> renameAut(int start = 0);
  BuchiAutomaton<int, int> renameAutDict(map<Symbol, int>& mpsymbol, int start = 0);

  SetStates& getStates()
  {
    return this->states;
  }

  SetStates& getFinals()
  {
    return this->finals;
  }

  SetStates& getInitials()
  {
    return this->initials;
  }

  Transitions& getTransitions()
  {
    return this->trans;
  }

  SetSymbols& getAlphabet()
  {
    return this->alph;
  }

  StateRelation& getOddRankSim()
  {
    return this->oddRankSim;
  }
  void setOddRankSim(StateRelation rl)
  {
    this->oddRankSim = rl;
  }

  void setDirectSim(StateRelation rl)
  {
    this->directSim = rl;
  }
  StateRelation& getDirectSim()
  {
    return this->directSim;
  }

  std::map<State, int>& getRenameStateMap()
  {
    return this->renameStateMap;
  }

  void setRenameStateMap(std::map<State, int> mp)
  {
    this->renameStateMap = mp;
  }

  vector<set<State> > reachableVector();
  void complete(State trap);
  void removeUseless();
  void restriction(set<State>& st);

  void computeRankSim(SetStates& cl);
  bool containsRankSimEq(SetStates& cl);
  vector<Symbol> containsSelfLoop(State& state);

  void getAutGraphComponents(AdjList& adjList, Vertices& vrt);
  vector<set<State>> getAutGraphSCCs();
  set<State> getEventReachable(set<State>& sls);
  set<State> getSelfLoops();

  std::map<State, int> propagateGraphValues(const std::function<int(LabelState<State>*,VecLabelStatesPtr)>& updFnc,
    const std::function<int(const State&)>& initFnc);

  SetStates getCycleClosingStates(SetStates& slignore);
  bool reachWithRestriction(const State& from, const State& to, SetStates& restr, SetStates& high);

  bool isEmpty();

  BuchiAutomaton<tuple<State, int, bool>, Symbol> productBA(BuchiAutomaton<int, Symbol>& other);
  BuchiAutomaton<State, Symbol> unionBA(BuchiAutomaton<State, Symbol>& other);
  void singleInitial(State init);

  vector<set<State>> getRunTree(vector<Symbol>& word);

  Transitions getReverseTransitions();
};

#endif
