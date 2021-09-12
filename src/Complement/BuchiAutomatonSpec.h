#ifndef _BUCHI_AUTOMATON_SPEC_H_
#define _BUCHI_AUTOMATON_SPEC_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <chrono>

#include <iostream>
#include <algorithm>

#include "../Algorithms/AuxFunctions.h"
#include "../Automata/BuchiAutomaton.h"
#include "BuchiDelay.h"
#include "StateKV.h"
#include "RankFunc.h"
#include "StateSch.h"
#include "Options.h"

using std::vector;
using std::set;
using std::map;

//enum delayVersion : unsigned;
enum sccType {D, ND, BAD, BOTH}; // deterministic with accepting states / nondeterministic without accepting states / bad = nondeterministic with accepting states / both = deterministic without accepting states

/*
 * Data structure for rank bounding data flow analysis
 */
struct RankBound
{
  int bound;
  map<int, int> stateBound;

  bool operator==(RankBound &other)
  {
    return (bound == other.bound) && (stateBound == other.stateBound);
  }

  bool operator!=(RankBound &other)
  {
    return !(*this == other);
  }
};


typedef set<int> DFAState;
/*
 * Successor cache data type
 */
typedef map<std::tuple<DFAState, int, int>, vector<std::pair<RankFunc,vector<RankFunc>>>> SuccRankCache;

/*
 * Specified Buchi automata with complementation
 */
class BuchiAutomatonSpec : public BuchiAutomaton<int, int>
{
private:
  map<DFAState, RankBound> rankBound;
  SuccRankCache rankCache;

  ComplOptions opt;

protected:
  RankConstr rankConstr(vector<int>& max, set<int>& states);
  set<int> succSet(const set<int>& state, int symbol);

  vector<RankFunc> getKVRanks(vector<int>& max, set<int>& states);
  set<StateKV> succSetKV(StateKV& state, int symbol);
  bool isKVFinal(StateKV& state) const { return state.O.size() == 0; }

  vector<RankFunc> getSchRanks(vector<int>& max, std::set<int>& states, StateSch& macrostate);
  void getSchRanksTight(vector<RankFunc>& out, vector<int>& max, set<int>& states,
      StateSch& macrostate, map<int, int> reachCons, int reachMax,
      BackRel& dirRel, BackRel& oddRel);
  vector<StateSch> succSetSchStart(set<int>& state, int rankBound, map<int, int> reachCons,
      map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel);
  vector<StateSch> succSetSchTight(StateSch& state, int symbol, map<int, int> reachCons,
      map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel);
  bool isSchFinal(StateSch& state) const { return state.tight ? state.O.size() == 0 : state.S.size() == 0; }
  bool getRankSuccCache(vector<RankFunc>& out, StateSch& state, int symbol);

  bool acceptSl(StateSch& state, vector<int>& alp);


  void getSchRanksTightOpt(vector<RankFunc>& out, vector<int>& max,
      set<int>& states, StateSch& macrostate,
      map<int, int> reachCons, int reachMax, BackRel& dirRel, BackRel& oddRel);
  vector<StateSch> succSetSchStartOpt(set<int>& state, int rankBound, map<int, int> reachCons,
      map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel);
  vector<StateSch> succSetSchTightOpt(StateSch& state, int symbol, map<int, int> reachCons,
      map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel);

public:
  BuchiAutomatonSpec(BuchiAutomaton<int, int> *t) : BuchiAutomaton<int, int>(*t), rankBound(), rankCache()
  {
    opt = { .cutPoint = false};
  }

  BackRel createBackRel(BuchiAutomaton<int, int>::StateRelation& rel);

  BuchiAutomaton<StateKV, int> complementKV();
  BuchiAutomaton<StateSch, int> complementSch();
  BuchiAutomaton<StateSch, int> complementSchReduced(std::set<int> originalFinals, elevatorOptions elevatorRank, Stat *stats);
  BuchiAutomaton<StateSch, int> complementSchNFA(set<int>& start);
  //BuchiAutomaton<StateSch, int> complementSchOpt(bool delay);
  BuchiAutomaton<StateSch, int> complementSchOpt(bool delay, std::set<int> originalFinals, double w, Stat *stats);

  set<StateSch> nfaSlAccept(BuchiAutomaton<StateSch, int>& nfaSchewe);
  set<pair<DFAState,int>> nfaSingleSlNoAccept(BuchiAutomaton<StateSch, int>& nfaSchewe);
  map<DFAState, RankBound> getRankBound(BuchiAutomaton<StateSch, int>& nfaSchewe, set<StateSch>& slignore, map<DFAState, int>& maxReachSize, map<int, int>& minReachSize);
  map<DFAState, int> getMaxReachSize(BuchiAutomaton<StateSch, int>& nfaSchewe, set<StateSch>& slIgnore);
  map<int, int> getMaxReachSizeInd();
  map<int, int> getMinReachSize();

  void setRankBound(map<DFAState, RankBound> rankbound){
    this->rankBound = rankbound;
  }

  void setComplOptions(ComplOptions& co) { this->opt = co; }
  ComplOptions getComplOptions() const { return this->opt; }

  std::map<int, int> elevatorRank(BuchiAutomaton<StateSch, int> nfaSchewe, bool detBeginning);
  unsigned elevatorStates();
  vector<set<int>> topologicalSort();
  void topologicalSortUtil(set<int> currentScc, vector<set<int>> allSccs, map<set<int>, bool> &visited, stack<set<int>> &Stack);

  bool isDeterministic(set<int> scc);
  bool isNonDeterministic(set<int> scc);
  bool isInherentlyWeak(set<int> scc);

  bool isElevator();

  void getSchRanksTightReduced(vector<RankFunc>& out, vector<int>& max,
      set<int>& states, int symbol, StateSch& macrostate,
      map<int, int> reachCons, int reachMax, BackRel& dirRel, BackRel& oddRel);
  vector<StateSch> succSetSchStartReduced(set<int>& state, int rankBound, map<int, int> reachCons,
      map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel, set<int> finals);
  vector<StateSch> succSetSchTightReduced(StateSch& state, int symbol, map<int, int> reachCons,
      map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel, set<int> finals);
};

#endif
