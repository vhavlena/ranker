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
#include "../Complement/StateSch.h"
#include "../Algorithms/AuxFunctions.h"
#include "APSymbol.h"

using std::tuple;

class AutGraph;

//enum delayVersion : unsigned;
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
  std::chrono::_V2::system_clock::time_point beginning;
  std::chrono::_V2::system_clock::time_point end;
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
 * Single transition
 */
template <typename State, typename Symbol>
struct Transition {
  State from;
  State to;
  Symbol symbol;
};

/*
 * States extended with labels
 */
template <typename State>
struct LabelState {
  State state;
  int label;
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
 * Transition function
 */
template<typename State, typename Symbol> using Delta = std::map<std::pair<State, Symbol>, std::set<State>>;
/*
 * State labels for the case of the DELAY optimization
 */
template<typename State> using DelayMap = std::map<State, DelayLabel>;


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

  bool isReachDeterministic(set<State>& start);

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

  unsigned getTransitionsToTight();
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

  /*
   * Get automaton states.
   * @return Set of states
   */
  SetStates& getStates()
  {
    return this->states;
  }

  /*
   * Get automaton final states.
   * @return Set of final states
   */
  SetStates& getFinals()
  {
    return this->finals;
  }

  /*
   * Get automaton initial states.
   * @return Set of initial states
   */
  SetStates& getInitials()
  {
    return this->initials;
  }

  /*
   * Get automaton transitions.
   * @return Transitions: map<pair<State, Symbol>, Set<States>>
   */
  Transitions& getTransitions()
  {
    return this->trans;
  }

  /*
   * Get automaton alphabet.
   * @return Set of symbols
   */
  SetSymbols& getAlphabet()
  {
    return this->alph;
  }

  /*
   * Set automaton alphabet.
   * @params st New set of symbols
   */
  void setAlphabet(SetSymbols st)
  {
    this->alph = st;
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

  /*
   * Get mapping used for renaming states of the automaton (created by calling
   * of renameAut(dict) method)
   * @return Mapping of states to int
   */
  std::map<State, int>& getRenameStateMap()
  {
    return this->renameStateMap;
  }

  /*
   * Get mapping used for renaming symbols of the automaton (created by calling
   * of renameAut method)
   * @return Mapping of symbols to int
   */
  std::map<Symbol, int>& getRenameSymbolMap()
  {
    return this->renameSymbolMap;
  }

  /*
   * Set mapping used for renaming states of the automaton.
   * @param mp Mapping of states to int
   */
  void setRenameStateMap(std::map<State, int> mp)
  {
    this->renameStateMap = mp;
  }

  /*
   * Get atomic propositions
   * @return Vector of atomic propositions
   */
  vector<string> getAPPattern()
  {
    return this->apsPattern;
  }

  /*
   * Set atomic propositions
   * @param aps Vector of atomic propositions
   */
  void setAPPattern(vector<string> aps)
  {
    this->apsPattern = aps;
  }

  /*
   * Get the number of simple transitions
   * @return Transitions count
   */
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
  vector<vector<State>> getAllCycles();
  bool circuit(int state, std::vector<int> &stack, std::set<int> &blockedSet, std::map<int, std::set<int>> &blockedMap, 
    std::set<int> scc, AdjList adjlist, int startState, std::vector<std::vector<int>> &allCyclesRenamed);
  void unblock(int state, std::set<int> &blockedSet, std::map<int, std::set<int>> &blockedMap);
  set<State> getAllSuccessors(State state);
  unsigned getAllPossibleRankings(unsigned maxRank, unsigned accStates, unsigned nonAccStates, delayVersion version);

  std::map<State, int> propagateGraphValues(const std::function<int(LabelState<State>*,VecLabelStatesPtr)>& updFnc,
    const std::function<int(const State&)>& initFnc);

  SetStates getCycleClosingStates(SetStates& slignore);
  std::map<State, std::set<Symbol>> getCycleClosingStates(SetStates& slignore, DelayMap<State>& dmap, double w, delayVersion version, Stat *stats);
  bool reachWithRestriction(const State& from, const State& to, SetStates& restr, SetStates& high);

  bool isEmpty();

  /*
   * Is the automaton deterministic
   * @return True deterministic, false otherwise
   */
  bool isDeterministic()
  {
    return this->initials.size() <= 1 && isReachDeterministic(this->initials);
  }

  /*
   * Is the automaton semideterministic
   * @return True semieterministic, false otherwise
   */
  bool isSemiDeterministic()
  {
    return isReachDeterministic(this->finals);
  }

  BuchiAutomaton<tuple<State, int, bool>, Symbol> productBA(BuchiAutomaton<int, Symbol>& other);
  BuchiAutomaton<State, Symbol> unionBA(BuchiAutomaton<State, Symbol>& other);
  void singleInitial(State init);

  BuchiAutomaton<State, Symbol> reverseBA();
  Delta<State, Symbol> getReverseTransitions();

  vector<set<State>> getRunTree(vector<Symbol>& word);

  BuchiAutomaton<StateSch, int> getComplStructure(std::map<int, StateSch>& mpst);
};

#endif
