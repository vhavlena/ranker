#ifndef _BUCHI_AUTOMATON_SPEC_H_
#define _BUCHI_AUTOMATON_SPEC_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <stack>

#include <iostream>
#include <algorithm>

#include "RankFunc.h"
#include "AuxFunctions.h"
#include "BuchiAutomaton.h"
#include "StateKV.h"
#include "StateSch.h"

using std::vector;
using std::set;
using std::map;

typedef set<int> DFAState;

class BuchiAutomatonSpec : public BuchiAutomaton<int, int>
{
private:
  BackRel createBackRel(BuchiAutomaton<int, int>::StateRelation& rel);

protected:
  RankConstr rankConstr(vector<int>& max, set<int>& states);
  set<int> succSet(set<int>& state, int symbol);

  vector<RankFunc> getKVRanks(vector<int>& max, set<int>& states);
  set<StateKV> succSetKV(StateKV& state, int symbol);
  bool isKVFinal(StateKV& state) const { return state.O.size() == 0; }

  vector<RankFunc> getSchRanks(vector<int>& max, std::set<int>& states, StateSch& macrostate);
  vector<RankFunc> getSchRanksTight(vector<int>& max, set<int>& states,
      StateSch& macrostate, map<int, set<int> >& succ, map<int, int> reachCons, int reachMax,
      BackRel& dirRel, BackRel& oddRel);
  set<StateSch> succSetSchStart(set<int>& state, int symbol, int rankBound, map<int, int> reachCons,
      map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel);
  set<StateSch> succSetSchTight(StateSch& state, int symbol, map<int, int> reachCons,
      map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel, set<std::tuple<DFAState, RankFunc, int>>& match);
  bool isSchFinal(StateSch& state) const { return state.tight ? state.O.size() == 0 : state.S.size() == 0; }


  set<StateSch> succSetSchTightMin(StateSch& state, int symbol);
  vector<RankFunc> getSchRanksMin(vector<int>& max, set<int>& states, StateSch& macrostate, map<int, set<int> >& succ);
  set<StateSch> succSetSchStartMin(set<int>& state, int symbol);

  bool acceptSl(StateSch& state, vector<int>& alp);

public:
  BuchiAutomatonSpec(BuchiAutomaton<int, int> &t) : BuchiAutomaton<int, int>(t) {}

  BuchiAutomaton<StateKV, int> complementKV();
  BuchiAutomaton<StateSch, int> complementSch();
  BuchiAutomaton<StateSch, int> complementSchMin();
  BuchiAutomaton<StateSch, int> complementSchNFA(set<int>& start);

  set<StateSch> nfaSlAccept(BuchiAutomaton<StateSch, int>& nfaSchewe);
  map<StateSch, int> getRankBound(BuchiAutomaton<StateSch, int>& nfaSchewe, set<StateSch>& slignore);
  map<DFAState, int> getMaxReachSize(BuchiAutomaton<StateSch, int>& nfaSchewe, set<StateSch>& slIgnore);
  map<int, int> getMinReachSize();
};

#endif
