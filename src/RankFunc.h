
#ifndef _RANK_FUNC_H_
#define _RANK_FUNC_H_

#include <set>
#include <map>
#include <vector>
#include <string>
#include <iostream>

#include <boost/dynamic_bitset.hpp>

using std::map;
using std::set;
using std::vector;

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

  static vector<RankFunc> cartTightProductMap(vector<RankFunc> s1, vector<std::pair<int, int> > s2, int rem, BackRel& rel, BackRel& oddRel, int max = -1);
  static vector<RankFunc> cartTightProductMapList(RankConstr slist, BackRel& rel, BackRel& oddRel, int max = -1);
  static inline bool checkDirectBackRel(std::pair<int, int>& act, RankFunc& tmp, BackRel& rel);
  static inline bool checkOddBackRel(std::pair<int, int>& act, RankFunc& tmp, BackRel& oddRel);

public:
  RankFunc() : map<int,int>(), oddStates(), inverse(), tight(0)
  {
    this->maxRank = 0;
  }

  RankFunc(const map<int,int> mp);
  void addPair(std::pair<int, int> val);

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

  int remTightCount() const
  {
    return this->tight.size() - this->tight.count();
  }

  std::string toString() const;
  bool isTightRank() const;
  bool zeroConsistent(map<int, int>& res, int reachMax) const;

  static vector<RankFunc> fromRankConstr(RankConstr constr);
  static vector<RankFunc> tightFromRankConstr(RankConstr constr, BackRel& rel, BackRel& oddRel);
  static vector<RankFunc> tightSuccFromRankConstr(RankConstr constr, BackRel& rel, BackRel& oddRel, int max);
};

#endif
