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
#include <chrono>

#include "AutGraph.h"
#include "../Complement/StateKV.h"
#include "AutomatonStruct.h"
#include "../Complement/StateSch.h"
#include "../Algorithms/AuxFunctions.h"
#include "APSymbol.h"

using std::tuple;

class AutGraph;

enum delayVersion : unsigned {oldVersion, newVersion, randomVersion, subsetVersion, stirlingVersion};

struct Stat
{
  size_t generatedStates;
  size_t generatedTrans;
  size_t reachStates;
  size_t reachTrans;
  size_t generatedTransitionsToTight;
  size_t transitionsToTight; // generated transitions to the tight part
  bool elevator; // is it an elevator automaton?
  size_t elevatorStates;
  size_t originalStates;
  long duration;
  string engine;

  // time
  std::chrono::time_point<std::chrono::high_resolution_clock> beginning;
  std::chrono::time_point<std::chrono::high_resolution_clock> end;
  long waitingPart;
  long rankBound;
  long elevatorRank = -1;
  long cycleClosingStates;
  long getAllCycles = -1;
  long statesToGenerate = -1;
  long simulations;
  long tightPart;
};

/*
 * Macrostate labels for the case of the DELAY optimization
 */
struct DelayLabel {
  unsigned macrostateSize;
  unsigned maxRank;
  unsigned nonAccStates;
};

/*
 * State labels for the case of the DELAY optimization
 */
template<typename State> using DelayMap = std::map<State, DelayLabel>;


template <typename State, typename Symbol>
class BuchiAutomaton : public AutomatonStruct<State, Symbol> {

public:
  typedef std::set<State> SetStates;
  typedef std::set<Symbol> SetSymbols;
  typedef Delta<State, Symbol> Transitions;
  typedef std::set<std::pair<State, State> > StateRelation;

private:
  SetStates finals;

  StateRelation directSim;
  StateRelation oddRankSim;

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
  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans) : AutomatonStruct<State, Symbol>(st, ini, trans)
  {
    this->finals = fin;
  }

  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans, SetSymbols alp) : AutomatonStruct<State, Symbol>(st, ini, trans, alp)
  {
    this->finals = fin;
  }

  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans, SetSymbols alp, vector<string> aps) : AutomatonStruct<State, Symbol>(st, ini, trans, alp, aps)
  {
    this->finals = fin;
  }

  BuchiAutomaton() : BuchiAutomaton({}, {}, {}, {}) {};

  BuchiAutomaton(BuchiAutomaton<State, Symbol>& other) : AutomatonStruct<State, Symbol>(other)
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

  // BuchiAutomaton<State, Symbol>& operator=(BuchiAutomaton<State, Symbol> other)
  // {
  //   this->states = other.states;
  //   this->finals = other.finals;
  //   this->trans = other.trans;
  //   this->initials = other.initials;
  //   this->alph = other.alph;
  //   this->directSim = other.directSim;
  //   this->oddRankSim = other.oddRankSim;
  //   this->renameStateMap = other.renameStateMap;
  //   this->renameSymbolMap = other.renameSymbolMap;
  //   this->invRenameMap = other.invRenameMap;
  //   this->apsPattern = other.apsPattern;
  //   return *this;
  // }

  BuchiAutomaton<State, Symbol>& operator=(BuchiAutomaton<State, Symbol> other)
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
    return *this;
  }


  std::string toString();
  std::string toGraphwiz();
  std::string toHOA();
  std::string toGff();

  /*
  * Rename states and symbols of the automaton (to consecutive numbers).
  * @param start Starting number for states
  * @return Renamed automaton
  */
  BuchiAutomaton<int, int> renameAut(int start = 0) {
    int stcnt = start;
    int symcnt = 0;
    std::map<State, int> mpstate;
    std::map<Symbol, int> mpsymbol;
    std::set<int> rstate;
    Delta<int, int> rtrans;
    std::set<int> rfin;
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
    rfin = Aux::mapSet(mpstate, this->finals);
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

    BuchiAutomaton<int, int> ret = BuchiAutomaton<int, int>(rstate, rfin, rini, rtrans, rsym);
    this->renameStateMap = mpstate;
    this->renameSymbolMap = mpsymbol;

    std::set<std::pair<int, int> > rdirSim, roddSim;
    for(auto item : this->directSim)
    {
      rdirSim.insert({mpstate[item.first], mpstate[item.second]});
    }
    for(auto item : this->oddRankSim)
    {
      roddSim.insert({mpstate[item.first], mpstate[item.second]});
    }
    ret.setDirectSim(rdirSim);
    ret.setOddRankSim(roddSim);
    ret.setAPPattern(this->apsPattern);
    return ret;
  }

  BuchiAutomaton<int, int> renameAutDict(map<Symbol, int>& mpsymbol, int start = 0);

  bool isElevator();

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
    ret.setDirectSim(this->directSim);
    ret.setOddRankSim(this->oddRankSim);
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

  void addStates(State state){
    this->states.insert(state);
  }

  void addFinals(State state){
    this->finals.insert(state);
  }

  void addNewTransition(std::pair<State, Symbol> src, std::set<State> dst){
    this->trans.insert({src, dst});
  }

  void addNewStatesToTransition(std::pair<State, Symbol> src, std::set<State> dst){
    this->trans[src].insert(dst.begin(), dst.end());
  }

  /*
   * Get odd rank simulation (aka rank simulation) between states
   * @return Set of pairs of states
   */
  StateRelation& getOddRankSim()
  {
    return this->oddRankSim;
  }

  /*
   * Set odd rank simulation (aka rank simulation) between states
   * @param rl Relation between states
   */
  void setOddRankSim(StateRelation rl)
  {
    this->oddRankSim = rl;
  }

  /*
   * Set direct simulation between states
   * @param rl Relation between states
   */
  void setDirectSim(StateRelation rl)
  {
    this->directSim = rl;
  }

  /*
   * Get direct simulation between states
   * @return Set of pairs of states
   */
  StateRelation& getDirectSim()
  {
    return this->directSim;
  }

  void complete(State trap, bool fin = false);
  void completeAPComplement();
  void removeUseless();
  void restriction(set<State>& st);

  void computeRankSim(SetStates& cl);
  bool containsRankSimEq(SetStates& cl);

  set<State> getEventReachable(set<State>& sls);
  SetStates getCycleClosingStates(SetStates& slignore);

  /*
  * Get SCCs of the automaton
  * @return Vector of SCCs (represented as a set of states)
  */
  vector<set<State>> getAutGraphSCCs(){
    BuchiAutomaton<int, int> renAutBA = this->renameAut();

    vector<vector<int>> adjList(this->states.size());
    vector<VertItem> vrt;
    vector<set<State>> sccs;

    renAutBA.getAutGraphComponents(adjList, vrt);
    AutGraph gr(adjList, vrt, renAutBA.getFinals());
    gr.computeSCCs();

    for(auto& scc : gr.getAllComponents())
    {
      set<State> singleScc;
      for(auto &st : scc)
      {
        singleScc.insert(this->invRenameMap[st]);
      }
      sccs.push_back(singleScc);
    }
    return sccs;
  }

  bool isEmpty();

  /*
   * Is the automaton semideterministic
   * @return True semieterministic, false otherwise
   */
  bool isSemiDeterministic()
  {
    return this->isReachDeterministic(this->finals);
  }

  void setFinals(SetStates finals){
    this->finals = finals;
  }

  BuchiAutomaton<tuple<State, int, bool>, Symbol> productBA(BuchiAutomaton<int, Symbol>& other);
  BuchiAutomaton<pair<State, int>, Symbol> cartProductBA(BuchiAutomaton<int, Symbol>& other);
  BuchiAutomaton<State, Symbol> unionBA(BuchiAutomaton<State, Symbol>& other);
  BuchiAutomaton<State, Symbol> reverseBA();

  BuchiAutomaton<StateSch, int> getComplStructure(std::map<int, StateSch>& mpst);
};

#endif
