
#include "RankFunc.h"


RankFunc::RankFunc(const map<int,int>& mp) : map<int,int>(mp)
{
  RankInverse::iterator it;
  for(const auto& k : mp)
  {
    this->maxRank = std::max(this->maxRank, k.second);
    this->ranks.push_back(k.second);
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


void RankFunc::addPair(const std::pair<int, int>& val)
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
  this->ranks.push_back(val.second);
  RankInverse::iterator it = this->inverse.find(val.second);
  if(it != this->inverse.end())
    it->second.insert(val.first);
  else
    this->inverse[val.second] = set<int>({val.first});
}


vector<RankFunc> RankFunc::cartTightProductMap(vector<RankFunc>& s1, vector<std::pair<int, int> >& s2,
    int rem, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax)
{
  vector<RankFunc> ret;
  int maxRank;
  for(const auto& v1 : s1)
  {
    for(const auto& v2 : s2)
    {
      RankFunc tmp(v1);
      tmp.addPair(v2);
      if(rem < tmp.remTightCount())
        continue;
      if(max != -1 && rem == 0 && tmp.getMaxRank() != max)
        continue;

      maxRank = std::min(tmp.getReachRestr(), v2.second + 2*(reachMax - reachRes[v2.first]));
      if(maxRank < tmp.getMaxRank())
        continue;
      tmp.setReachRestr(maxRank);

      if(!RankFunc::checkDirectBackRel(v2, tmp, rel))
        continue;
      if(!RankFunc::checkOddBackRel(v2, tmp, oddRel))
        continue;
      ret.push_back(tmp);
    }
  }
  return ret;
}

vector<RankFunc> RankFunc::cartTightProductMapOdd(vector<RankFunc>& s1, vector<std::pair<int, int> >& s2,
    int rem, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax)
{
  vector<RankFunc> ret;
  int maxRank;
  bool ins;
  for(const auto& v1 : s1)
  {
    ins = false;
    for(const auto& v2 : s2)
    {
      if(v2.second % 2 == 0)
      {
        if(!ins)
          ret.push_back(v1);
        ins = true;
        continue;
      }
      RankFunc tmp(v1);
      tmp.addPair(v2);
      if(rem < tmp.remTightCount())
        continue;
      if(max != -1 && rem == 0 && tmp.getMaxRank() != max)
        continue;

      maxRank = std::min(tmp.getReachRestr(), v2.second + 2*(reachMax - reachRes[v2.first]));
      if(maxRank < tmp.getMaxRank())
        continue;
      tmp.setReachRestr(maxRank);

      if(!RankFunc::checkDirectBackRel(v2, tmp, rel))
        continue;
      if(!RankFunc::checkOddBackRel(v2, tmp, oddRel))
        continue;
      ret.push_back(tmp);
    }
  }
  return ret;
}


bool RankFunc::checkDirectBackRel(const std::pair<int, int>& act, RankFunc& tmp, BackRel& rel)
{
  for(auto st : rel[act.first])
  {
    auto it = tmp.find(st.first);
    if(it != tmp.end())
    {
      if(st.second && it->second < act.second)
      {
        return false;
      }
      if(!st.second && it->second > act.second)
      {
        return false;
      }
    }
  }
  return true;
}


bool RankFunc::checkOddBackRel(const std::pair<int, int>& act, RankFunc& tmp, BackRel& oddRel)
{
  if(act.second % 2 != 0)
  {
    for(auto st : oddRel[act.first])
    {
      auto it = tmp.find(st.first);
      if(it != tmp.end())
      {
        if(it->second % 2 == 0)
          continue;
        if(st.second && it->second < act.second)
        {
          return false;
        }
        if(!st.second && it->second > act.second)
        {
          return false;
        }
      }
    }
  }
  return true;
}


vector<RankFunc> RankFunc::cartTightProductMapList(RankConstr slist, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax)
{
  vector<RankFunc> ret;
  if(slist.size() == 0)
    return ret;

  for(auto p : slist[0])
  {
    RankFunc sing(map<int, int>({p}));
    sing.setReachRestr(p.second + 2*(reachMax - reachRes[p.first]));
    ret.push_back(sing);
  }
  for(int i = 1; i < slist.size(); i++)
  {
    ret = RankFunc::cartTightProductMap(ret, slist[i], slist.size() - i - 1, rel, oddRel, max, reachRes, reachMax);
  }
  return ret;
}


vector<RankFunc> RankFunc::cartTightProductMapListOdd(RankConstr slist, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax)
{
  vector<RankFunc> ret;
  vector<RankFunc> retcp;
  if(slist.size() == 0)
    return ret;
  set<int> states;

  for(auto p : slist[0])
  {
    RankFunc sing(map<int, int>({p}));
    sing.setReachRestr(p.second + 2*(reachMax - reachRes[p.first]));
    ret.push_back(sing);
    states.insert(p.first);
  }
  for(int i = 1; i < slist.size(); i++)
  {
    if(slist[i].size() == 0)
      return ret;
    ret = RankFunc::cartTightProductMapOdd(ret, slist[i], slist.size() - i - 1, rel, oddRel, max, reachRes, reachMax);
    states.insert(slist[i][0].first);
    //std::cout << i << " : " << ret.size() << std::endl;
  }

  for(int i = 0; i < ret.size(); i++)
  {
    if(ret[i].getMaxRank() % 2 == 0)
      continue;
    auto odd(ret[i].getOddStates());
    for(int s : states)
    {
      if(odd.find(s) == odd.end())
      {
        ret[i].addPair({s, ret[i].getMaxRank() - 1});
      }
    }
    retcp.push_back(ret[i]);
  }

  return retcp;
}


vector<RankFunc> RankFunc::fromRankConstr(RankConstr constr)
{
  vector<vector<std::pair<int,bool> > > emp;
  map<int, int> empMap;
  return RankFunc::cartTightProductMapList(constr, emp, emp, -1, empMap, INF);
}


vector<RankFunc> RankFunc::tightFromRankConstr(RankConstr constr, BackRel& rel, BackRel& oddRel, map<int, int>& reachRes, int reachMax)
{
  return RankFunc::cartTightProductMapList(constr, rel, oddRel, -1, reachRes, reachMax);
}


vector<RankFunc> RankFunc::tightFromRankConstrOdd(RankConstr constr, BackRel& rel, BackRel& oddRel, map<int, int>& reachRes, int reachMax)
{
  return RankFunc::cartTightProductMapListOdd(constr, rel, oddRel, -1, reachRes, reachMax);
}


vector<RankFunc> RankFunc::tightSuccFromRankConstr(RankConstr constr, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax)
{
  return RankFunc::cartTightProductMapList(constr, rel, oddRel, max, reachRes, reachMax);
}


bool RankFunc::isSuccValid(RankFunc& prev, map<int, set<int> >& succ) const
{
  bool val = true;
  int fnc = 0;
  for(const auto& s : succ)
  {
    val = false;
    fnc = prev.find(s.first)->second;
    if(fnc % 2 == 0)
    {
      if(s.second.size() == 0)
        val = true;
      for(int dst : s.second)
      {
        if(this->find(dst)->second % 2 == 0)
        {
          val = true;
          break;
        }
      }
    }
    else
    {
      for(int dst : s.second)
      {
        if(this->find(dst)->second == fnc)
        {
          val = true;
          break;
        }
      }
    }

    if (!val)
      return false;
  }
  return true;
}


bool RankFunc::isMaxRankValid(vector<int>& maxRank) const
{
  for(int i = 0; i < maxRank.size(); i++)
  {
    if(maxRank[i] < this->ranks[i])
    {
      return false;
    }
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
  return this->tight.all();
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

bool RankFunc::isAllLeq(RankFunc& f)
{
  vector<int> rnk = f.getRanks();
  if(this->ranks.size() != rnk.size())
    return false;
  for(int i = 0; i < this->ranks.size(); i++)
  {
    if(this->ranks[i] > rnk[i])
      return false;
  }
  return true;
}


bool RankFunc::relConsistent(set<std::pair<int, int> >& rel) const
{
  for(auto item : rel)
  {
    auto it1 = this->find(item.first);
    auto it2 = this->find(item.second);
    if(it1 != this->end() && it2 != this->end() && it1->second > it2->second)
      return false;
  }
  return true;
}


bool RankFunc::relOddConsistent(set<std::pair<int, int> >& rel) const
{
  for(auto item : rel)
  {
    auto it1 = this->find(item.first);
    auto it2 = this->find(item.second);
    if(it1 != this->end() && it2 != this->end() && it1->second % 2 != 0 && it2->second % 2 != 0 && it1->second > it2->second)
      return false;
  }
  return true;
}


bool RankFunc::isReachConsistent(map<int, int>& res, int reachMax) const
{
  for(const auto& t : res)
  {
    auto it = this->find(t.first);
    if(it != this->end())
    {
      if(it->second < this->getMaxRank() - 2*(reachMax - t.second))
        return false;
    }
  }
  return true;
}
