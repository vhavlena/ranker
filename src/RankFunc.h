
#ifndef _RANK_FUNC_H_
#define _RANK_FUNC_H_

#include <set>
#include <map>
#include <vector>
#include <string>
#include <iostream>
#include <string>

#include <boost/dynamic_bitset.hpp>

#define INF 100000000

using std::map;
using std::set;
using std::vector;
using std::string;

class RankFunc;

typedef vector<vector<std::pair<int, int> > > RankConstr;
typedef map<int, set<int> > RankInverse;
typedef vector<vector<std::pair<int,bool> > > BackRel;

class RankFunc : public map<int,int>
{
private:
  boost::dynamic_bitset<> tight;
  set<int> oddStates;
  int maxRank;
  RankInverse inverse;
  vector<int> ranks;
  int reachRest;

  static vector<RankFunc> cartTightProductMap(vector<RankFunc>& s1, vector<std::pair<int, int> >& s2, int rem, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax);
  static vector<RankFunc> cartTightProductMapList(RankConstr slist, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax);
  static inline bool checkDirectBackRel(const std::pair<int, int>& act, RankFunc& tmp, BackRel& rel);
  static inline bool checkOddBackRel(const std::pair<int, int>& act, RankFunc& tmp, BackRel& oddRel);

  static vector<RankFunc> cartTightProductMapOdd(vector<RankFunc>& s1, vector<std::pair<int, int> >& s2, int rem, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax);
  static vector<RankFunc> cartTightProductMapListOdd(RankConstr slist, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax);

public:
  RankFunc() : map<int,int>(), oddStates(), inverse(), tight(0), ranks()
  {
    this->maxRank = 0;
    this->reachRest = INF;
  }

  RankFunc(const map<int,int>& mp);
  void addPair(const std::pair<int, int>& val);

  inline void addOddStates(set<int>& states)
  {
    this->oddStates.insert(states.begin(), states.end());
  }

  inline set<int>& getOddStates()
  {
    return this->oddStates;
  }

  inline set<int>& inverseRank(int i)
  {
    return this->inverse[i];
  }

  inline int getMaxRank() const
  {
    return this->maxRank;
  }

  bool eqEven() const;
  bool relConsistent(set<std::pair<int, int> >& rel) const;
  bool relOddConsistent(set<std::pair<int, int> >& rel) const;
  bool isSuccValid(RankFunc& prev, map<int, set<int> >& succ) const;
  bool isMaxRankValid(vector<int>& maxRank) const;

  bool isAllLeq(RankFunc& f);

  int remTightCount() const
  {
    return this->tight.size() - this->tight.count();
  }

  std::string toString() const;
  bool isTightRank() const;
  bool isReachConsistent(map<int, int>& res, int reachMax) const;

  vector<int>& getRanks() { return this->ranks; }
  int getReachRestr() { return this->reachRest; }
  void setReachRestr(int val) { this->reachRest = val; }

  static vector<RankFunc> fromRankConstr(RankConstr constr);
  static vector<RankFunc> tightFromRankConstr(RankConstr constr, BackRel& rel, BackRel& oddRel, map<int, int>& reachRes, int reachMax);
  static vector<RankFunc> tightFromRankConstrOdd(RankConstr constr, BackRel& rel, BackRel& oddRel, map<int, int>& reachRes, int reachMax);
  static vector<RankFunc> tightSuccFromRankConstr(RankConstr constr, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax);
};

#endif
