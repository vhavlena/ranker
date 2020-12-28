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
#include <numeric>

#include "AutGraph.h"
#include "../Complement/StateKV.h"
#include "../Complement/StateSch.h"
#include "../Algorithms/AuxFunctions.h"
#include "APSymbol.h"

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

template<typename State, typename Symbol> using Delta = std::map<std::pair<State, Symbol>, std::set<State>>;

template <typename State, typename Symbol>
class BuchiAutomaton {

public:
  typedef std::set<State> SetStates;
  typedef std::set<LabelState<State> > SetLabelStates;
  typedef std::set<LabelState<State>* > SetLabelStatesPtr;
  typedef std::vector<LabelState<State>* > VecLabelStatesPtr;
  typedef std::vector<LabelState<State>> VecLabelStates;
  typedef std::set<Symbol> SetSymbols;
  typedef Delta<State, Symbol> Transitions;
  typedef std::set<std::pair<State, State> > StateRelation;

private:
  SetStates states;
  SetStates finals;
  SetStates initials;
  SetSymbols alph;
  Delta<State, Symbol> trans;
  vector<string> apsPattern;

  StateRelation directSim;
  StateRelation oddRankSim;

  std::map<State, int> renameStateMap;
  std::map<Symbol, int> renameSymbolMap;
  std::vector<State> invRenameMap;

protected:
  std::string toStringWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr);
  std::string toGraphwizWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr);
  std::string toGffWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr);

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
    this->apsPattern = vector<string>();
  }

  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans, SetSymbols alp)
  {
    this->states = st;
    this->finals = fin;
    this->trans = trans;
    this->initials = ini;
    this->alph = alp;
    this->apsPattern = vector<string>();
  }

  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans, SetSymbols alp, vector<string> aps)
  {
    this->states = st;
    this->finals = fin;
    this->trans = trans;
    this->initials = ini;
    if(alp.size() == 0)
      this->alph = getAlph();
    else
      this->alph = alp;
    this->apsPattern = aps;
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
    this->renameSymbolMap = other.renameSymbolMap;
    this->invRenameMap = other.invRenameMap;
    this->apsPattern = other.apsPattern;
  }

  std::set<Symbol> getAlph();
  std::string toString();
  std::string toGraphwiz();
  std::string toGff();
  std::string toHOA();
  BuchiAutomaton<int, int> renameAut(int start = 0);
  BuchiAutomaton<int, int> renameAutDict(map<Symbol, int>& mpsymbol, int start = 0);

  template<typename NewSymbol>
  BuchiAutomaton<State, NewSymbol> renameAlphabet(map<Symbol, NewSymbol>& mpsymbol)
  {
    std::set<NewSymbol> ralph;
    Delta<int, NewSymbol> rtrans;
    for(const auto& al : this->alph)
    {
      ralph.insert(mpsymbol[al]);
    }
    for(auto p : this->trans)
    {
      //auto it = mpsymbol.find(p.first.second);
      NewSymbol val = mpsymbol[p.first.second];
      // if(it == mpsymbol.end())
      // {
      //   val = symcnt;
      //   mpsymbol[p.first.second] = symcnt++;
      // }
      // else
      // {
      //   val = it->second;
      // }
      rtrans.insert({std::make_pair(p.first.first, val), p.second});
    }
    auto ret = BuchiAutomaton<State, NewSymbol>(this->states, this->finals, this->initials, rtrans, ralph, this->apsPattern);
    ret.setDirectSim(this->directSim);
    ret.setOddRankSim(this->oddRankSim);
    ret.setAPPattern(this->apsPattern);
    return ret;
  }

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

  void setAlphabet(SetSymbols st)
  {
    this->alph = st;
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

  std::map<Symbol, int>& getRenameSymbolMap()
  {
    return this->renameSymbolMap;
  }

  void setRenameStateMap(std::map<State, int> mp)
  {
    this->renameStateMap = mp;
  }

  vector<string> getAPPattern()
  {
    return this->apsPattern;
  }

  void setAPPattern(vector<string> aps)
  {
    this->apsPattern = aps;
  }

  int getTransCount() const
  {
    int cnt = 0;
    for(const auto& t : this->trans)
    {
      cnt += t.second.size();
    }
    return cnt;
  }

  vector<set<State> > reachableVector();
  void complete(State trap, bool fin = false);
  void completeAPComplement();
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

  BuchiAutomaton<State, Symbol> reverseBA();
  Delta<State, Symbol> getReverseTransitions();

  vector<set<State>> getRunTree(vector<Symbol>& word);
};

#endif
