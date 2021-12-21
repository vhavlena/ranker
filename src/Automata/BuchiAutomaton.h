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
#include "StateSemiDet.h"
#include "AutomatonStruct.h"
#include "../Complement/StateSch.h"
#include "../Complement/StateSD.h"
#include "../Algorithms/AuxFunctions.h"
#include "APSymbol.h"
//#include "../Algorithms/Simulations.h"

using std::tuple;

class AutGraph;
struct RankBound;

struct ElevatorOptions{
  bool elevatorRank = false;
  bool detBeginning = false;
};

struct Stat
{
  size_t generatedStates = 0;
  size_t generatedTrans = 0;
  size_t reachStates;
  size_t reachTrans;
  size_t generatedTransitionsToTight = 0;
  size_t transitionsToTight; // generated transitions to the tight part
  bool elevator; // is it an elevator automaton?
  size_t elevatorStates = 0;
  size_t originalStates = 0;
  long duration = 0;
  string engine;
  std::map<std::set<int>, RankBound> ranks;

  // time
  std::chrono::time_point<std::chrono::high_resolution_clock> beginning;
  std::chrono::time_point<std::chrono::high_resolution_clock> end;
  long waitingPart = 0;
  long rankBound = 0;
  long elevatorRank = -1;
  long cycleClosingStates = 0;
  long getAllCycles = -1;
  long statesToGenerate = -1;
  long simulations = 0;
  long tightPart = 0;
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
 * Data structure for information about possible types of scc
 */
struct SccClassif {
  set<int> states;
  bool det = false;
  bool inhWeak = false;
  bool nonDet = false;
  int rank = -1;
  bool detBeginning = false;
};


typedef set<int> DFAState;

/*
 * State labels for the case of the DELAY optimization
 */
template<typename State> using DelayMap = std::map<State, DelayLabel>;


template <typename State, typename Symbol>
class BuchiAutomaton : public AutomatonStruct<State, Symbol> {

public:
  typedef std::set<State> SetStates;
  typedef VecTrans<State, Symbol> VecTransG;
  typedef std::set<Symbol> SetSymbols;
  typedef Delta<State, Symbol> Transitions;
  typedef std::set<std::pair<State, State> > StateRelation;

private:
  SetStates finals;
  VecTransG accTrans;

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
    this->accTrans = VecTransG();
  }

  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans, SetSymbols alp) : AutomatonStruct<State, Symbol>(st, ini, trans, alp)
  {
    this->finals = fin;
    this->accTrans = VecTransG();
  }

  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans, SetSymbols alp, vector<string> aps) : AutomatonStruct<State, Symbol>(st, ini, trans, alp, aps)
  {
    this->finals = fin;
    this->accTrans = VecTransG();
  }

  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans, VecTransG accTr) : AutomatonStruct<State, Symbol>(st, ini, trans)
  {
    this->finals = fin;
    this->accTrans = accTr;
  }

  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans, VecTransG accTr, SetSymbols alp) : AutomatonStruct<State, Symbol>(st, ini, trans, alp)
  {
    this->finals = fin;
    this->accTrans = accTr;
  }

  BuchiAutomaton(SetStates st, SetStates fin, SetStates ini, Transitions trans, VecTransG accTr, SetSymbols alp, vector<string> aps) : AutomatonStruct<State, Symbol>(st, ini, trans, alp, aps)
  {
    this->finals = fin;
    this->accTrans = accTr;
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
    this->accTrans = other.accTrans;
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
    this->accTrans = other.accTrans;
    return *this;
  }


  std::string toString();
  std::string toGraphwiz();
  std::string toHOA();
  std::string toHOA(std::map<int,int> sccs);
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
    VecTrans<int, int> ftrans;

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

    for(unsigned i = 0; i < this->accTrans.size(); i++)
    {
      ftrans.push_back({ .from = mpstate[this->accTrans[i].from],
          .to = mpstate[this->accTrans[i].to],
          .symbol = mpsymbol[this->accTrans[i].symbol] });
    }

    BuchiAutomaton<int, int> ret = BuchiAutomaton<int, int>(rstate, rfin, rini, rtrans, ftrans, rsym);
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

  /*
  * Rename states and symbols of the automaton (to consecutive numbers).
  * @param start Starting number for states
  * @return Renamed automaton
  */
  BuchiAutomaton<int, Symbol> renameStates(int start = 0) {
    int stcnt = start;
    std::map<State, int> mpstate;
    std::set<int> rstate;
    Delta<int, Symbol> rtrans;
    std::set<int> rfin;
    std::set<int> rini;
    VecTrans<int, Symbol> ftrans;

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

    rstate = Aux::mapSet(mpstate, this->states);
    rini = Aux::mapSet(mpstate, this->initials);
    rfin = Aux::mapSet(mpstate, this->finals);
    for(auto& p : this->trans)
    {
      Symbol val = p.first.second;
      std::set<int> to = Aux::mapSet(mpstate, p.second);
      rtrans.insert({std::make_pair(mpstate[p.first.first], val), to});
    }

    for(unsigned i = 0; i < this->accTrans.size(); i++)
    {
      ftrans.push_back({ .from = mpstate[this->accTrans[i].from],
          .to = mpstate[this->accTrans[i].to],
          .symbol = this->accTrans[i].symbol });
    }

    BuchiAutomaton<int, Symbol> ret = BuchiAutomaton<int, Symbol>(rstate, rfin, rini, rtrans, ftrans);
    this->renameStateMap = mpstate;

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
    VecTrans<State, NewSymbol> ftrans;
    for(const auto& al : this->alph)
    {
      ralph.insert(mpsymbol[al]);
    }
    for(auto p : this->trans)
    {
      NewSymbol val = mpsymbol[p.first.second];
      rtrans.insert({std::make_pair(p.first.first, val), p.second});
    }
    for(unsigned i = 0; i < this->accTrans.size(); i++)
    {
      ftrans.push_back({ .from = this->accTrans[i].from, .to = this->accTrans[i].to,
          .symbol = mpsymbol[this->accTrans[i].symbol] });
    }
    auto ret = BuchiAutomaton<State, NewSymbol>(this->states, this->finals, this->initials, rtrans, ftrans, ralph, this->apsPattern);
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

  VecTransG& getFinTrans()
  {
    return this->accTrans;
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
    set<State> rch;
    for(const auto& tr : this->accTrans)
      rch.insert(tr.from);

    set<State> sun;
    set_union(rch.begin(), rch.end(), this->finals.begin(), this->finals.end(),
        inserter(sun, sun.begin()));
    return this->isReachDeterministic(sun);
  }

  void setFinals(SetStates finals){
    this->finals = finals;
  }

  BuchiAutomaton<tuple<State, int, bool>, Symbol> productBA(BuchiAutomaton<int, Symbol>& other);
  BuchiAutomaton<pair<State, int>, Symbol> cartProductBA(BuchiAutomaton<int, Symbol>& other);
  BuchiAutomaton<State, Symbol> unionBA(BuchiAutomaton<State, Symbol>& other);
  BuchiAutomaton<State, Symbol> reverseBA();

  BuchiAutomaton<StateSch, int> getComplStructure(std::map<int, StateSch>& mpst);

  map<State, set<Symbol> > getPredSymbolMap();
  map<State, set<Symbol>> getReverseSymbolMap();

  bool isTBA() const
  {
    return this->accTrans.size() > 0;
  }

  BuchiAutomaton<int, int> copyStateAcc(int start);
  BuchiAutomaton<StateSemiDet, Symbol> semideterminize();
  set<State> succSet(const set<State>& state, const Symbol& symbol);

  BuchiAutomaton<int, int> removeUselessRename();

  BuchiAutomaton<int, Symbol> reduce();
  BuchiAutomaton<int, Symbol> toTBA();
};

#endif
