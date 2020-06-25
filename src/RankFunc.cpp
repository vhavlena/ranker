
#include "RankFunc.h"

vector<RankFunc> RankFunc::cartProductMap(vector<RankFunc> s1, vector<std::pair<int, int> > s2)
{
  vector<RankFunc> ret;
  for(auto v1 : s1)
  {
    for(auto v2 : s2)
    {
      RankFunc tmp(v1);
      tmp.addPair(v2);
      ret.push_back(tmp);
    }
  }
  return ret;
}


vector<RankFunc> RankFunc::cartProductMapList(RankConstr slist)
{
  vector<RankFunc> ret;
  if(slist.size() == 0)
    return ret;

  for(auto p : slist[0])
    ret.push_back(RankFunc(map<int, int>({p})));
  for(int i = 1; i < slist.size(); i++)
  {
    ret = RankFunc::cartProductMap(ret, slist[i]);
  }
  return ret;
}


vector<RankFunc> RankFunc::fromRankConstr(RankConstr constr)
{
  return RankFunc::cartProductMapList(constr);
}


bool RankFunc::isSuccValid(RankFunc& prev, map<int, set<int> >& succ) const
{
  bool val = true;
  int fnc = 0;
  for(auto s : succ)
  {
    val = false;
    fnc = prev.find(s.first)->second;
    if(fnc % 2 == 0)
      continue;
    for(int dst : s.second)
    {
      if((this->find(dst)->second) == fnc)
      {
        val = true;
        break;
      }
    }
    if (!val)
      return false;
  }
  return true;
}


std::string RankFunc::toString() const
{
  std::string ret = "{";
  for (auto p : *this)
    ret += std::to_string(p.first) + ":" + std::to_string(p.second) + " ";
  if(ret.back() == ' ')
    ret.pop_back();
  ret += "}";
  return ret;
}


bool RankFunc::isTightRank() const
{
  boost::dynamic_bitset<> rnk((this->maxRank+1)/2);
  if(this->maxRank % 2 == 0)
    return false;
  for(auto v : *this)
  {
    if(v.second > this->maxRank)
      return false;
    if(v.second % 2 == 0)
      continue;
    rnk[(v.second+1)/2 - 1] = 1;
  }
  return rnk.all();
}


bool RankFunc::eqEven() const
{
  for(auto v : *this)
  {
    if(v.second % 2 == 0 && v.second != this->maxRank - 1)
      return false;
  }
  return true;
}
