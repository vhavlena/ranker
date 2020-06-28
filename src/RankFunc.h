
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
  boost::dynamic_bitset<> tight;
  set<int> oddStates;
  int maxRank;
  RankInverse inverse;

  static vector<RankFunc> cartTightProductMap(vector<RankFunc> s1, vector<std::pair<int, int> > s2, int rem, vector<vector<std::pair<int,bool> > >& rel);
  static vector<RankFunc> cartTightProductMapList(RankConstr slist, vector<vector<std::pair<int,bool> > >& rel);

public:
  RankFunc() : map<int,int>(), oddStates(), inverse(), tight(0)
  {
    this->maxRank = 0;
  }

  RankFunc(const map<int,int> mp) : map<int,int>(mp)
  {
    RankInverse::iterator it;
    for(auto k : mp)
    {
      this->maxRank = std::max(this->maxRank, k.second);
      int tmp = this->maxRank;
      if(tmp % 2 == 0) tmp++;
      if(this->tight.size() < (tmp-1)/2 + 1)
        this->tight.resize((tmp-1)/2 + 1);
      if(k.second % 2 != 0)
      {
        this->oddStates.insert(k.first);
        this->tight.set((k.second - 1) / 2);
      }

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
    this->maxRank = std::max(this->maxRank, val.second);
    int tmp = this->maxRank;
    if(tmp % 2 == 0) tmp++;
    if(this->tight.size() < (tmp-1)/2 + 1)
      this->tight.resize((tmp-1)/2 + 1);
    if(val.second % 2 != 0)
    {
      this->oddStates.insert(val.first);
      this->tight.set((val.second - 1) / 2);
    }

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

  static vector<RankFunc> fromRankConstr(RankConstr constr);
  static vector<RankFunc> tightFromRankConstr(RankConstr constr, vector<vector<std::pair<int,bool> > >& rel);
};

#endif
