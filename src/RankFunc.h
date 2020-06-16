
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

class RankFunc : public map<int,int>
{
private:
  set<int> oddStates;
  int maxRank;

  static vector<RankFunc> cartProductMap(vector<RankFunc> s1, vector<std::pair<int, int> > s2);
  static vector<RankFunc> cartProductMapList(RankConstr slist);

public:
  RankFunc(const map<int,int> mp) : map<int,int>(mp)
  {
    for(auto k : mp)
    {
      if(k.second % 2 != 0)
        this->oddStates.insert(k.first);
      this->maxRank = std::max(this->maxRank, k.second);
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
  }

  inline set<int>& getOddStates()
  {
    return this->oddStates;
  }

  inline int getMaxRank() const
  {
    return this->maxRank;
  }

  std::string toString() const;
  bool isTightRank() const;

  static vector<RankFunc> fromRankConstr(RankConstr constr);
};

#endif
