
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

class RankFunc : public map<int,int>
{
private:
  set<int> oddStates;
  int maxRank;
  RankInverse inverse;

  static vector<RankFunc> cartProductMap(vector<RankFunc> s1, vector<std::pair<int, int> > s2);
  static vector<RankFunc> cartProductMapList(RankConstr slist);

public:
  RankFunc() : map<int,int>(), oddStates(), inverse()
  {
    this->maxRank = 0;
  }

  RankFunc(const map<int,int> mp) : map<int,int>(mp)
  {
    RankInverse::iterator it;
    for(auto k : mp)
    {
      if(k.second % 2 != 0)
        this->oddStates.insert(k.first);
      this->maxRank = std::max(this->maxRank, k.second);
      it = this->inverse.find(k.second);
      if(it != this->inverse.end())
        it->second.insert(k.first);
      else
        this->inverse[k.second] = set<int>({k.first});
    }
  }

  inline void addOddStates(set<int>& states)
  {
    this->oddStates.insert(states.begin(), states.end());
  }

  void addPair(std::pair<int, int> val)
  {
    if(val.second % 2 != 0)
      this->oddStates.insert(val.second);
    this->maxRank = std::max(this->maxRank, val.second);
    this->insert(val);
    RankInverse::iterator it = this->inverse.find(val.second);
    if(it != this->inverse.end())
      it->second.insert(val.first);
    else
      this->inverse[val.second] = set<int>({val.first});
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

  bool isSuccValid(RankFunc& prev, map<int, set<int> >& succ) const;

  std::string toString() const;
  bool isTightRank() const;

  static vector<RankFunc> fromRankConstr(RankConstr constr);
};

#endif
