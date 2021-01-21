
#include "RankFunc.h"


/*
 * Constructor for the Ranking function
 * @param mp Mapping representing the ranking function
 * @param useInverse Use inverse mapping
 */
RankFunc::RankFunc(const map<int,int>& mp, bool useInverse) : map<int,int>(mp),
  oddStates(), inverse(), ranks(), tight()
{
  this->maxRank = 0;
  this->reachRest = INF;
  RankInverse::iterator it;
  for(const auto& k : mp)
  {
    this->maxRank = std::max(this->maxRank, k.second);
    this->ranks.push_back(k.second);
    int tmp = this->maxRank;
    if(tmp % 2 == 0) tmp++;
    if((int)this->tight.size() < (tmp-1)/2 + 1)
      this->tight.resize((tmp-1)/2 + 1, 0);
    if(k.second % 2 != 0)
    {
      this->oddStates.insert(k.first);
      //this->tight.set((k.second - 1) / 2);
      this->tight[(k.second - 1) / 2] = 1;
    }

    if(useInverse)
    {
      it = this->inverse.find(k.second);
      if(it != this->inverse.end())
        it->second.insert(k.first);
      else
        this->inverse[k.second] = set<int>({k.first});
    }
  }
}


/*
 * Add pair to the ranking function
 * @param val Pair to be added
 * @param useInverse Use inverse mapping
 */
void RankFunc::addPair(const std::pair<int, int>& val, bool useInverse)
{
  this->maxRank = std::max(this->maxRank, val.second);
  int tmp = this->maxRank;
  if(tmp % 2 == 0) tmp++;
  if((int)this->tight.size() < (tmp-1)/2 + 1)
    this->tight.resize((tmp-1)/2 + 1, 0);
  if(val.second % 2 != 0)
  {
    this->oddStates.insert(val.first);
    //this->tight.set((val.second - 1) / 2);
    this->tight[(val.second - 1) / 2] = 1;
  }

  this->insert(val);
  this->ranks.push_back(val.second);

  if(useInverse)
  {
    RankInverse::iterator it = this->inverse.find(val.second);
    if(it != this->inverse.end())
      it->second.insert(val.first);
    else
      this->inverse[val.second] = set<int>({val.first});
  }
}


/*
 * Cartesian product of a set of ranking functions and pairs to be added
 * @param s1 Set of ranking funtions
 * @param s2 Vector of pairs to be added to s1
 * @param rem Number of remaining tight positions
 * @param rel Direct simulation
 * @param oddRel Rank simulation
 * @param reachRes Reachability restriction (SuccRank)
 * @param maxReach Maximum reachable macrostate
 * @return Set of combinations of ranking functions with joined pairs
 */
vector<RankFunc> RankFunc::cartTightProductMap(vector<RankFunc>& s1, vector<std::pair<int, int> >& s2,
    int rem, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax, bool useInverse)
{
  vector<RankFunc> ret;
  int maxRank;
  for(const auto& v1 : s1)
  {
    for(const auto& v2 : s2)
    {
      RankFunc tmp(v1);
      tmp.addPair(v2, useInverse);
      if(rem < tmp.remTightCount())
        continue;
      if(max != -1 && rem == 0 && tmp.getMaxRank() != max)
        continue;

      maxRank = std::min(tmp.getReachRestr(), v2.second + 2*(reachMax - reachRes[v2.first]));
      if(maxRank < tmp.getMaxRank())
        continue;
      tmp.setReachRestr(maxRank);

      // if(!RankFunc::checkDirectBackRel(v2, tmp, rel))
      //   continue;
      if(!RankFunc::checkOddBackRel(v2, tmp, oddRel))
        continue;
      ret.push_back(tmp);
    }
  }
  return ret;
}


/*
 * Cartesian product of a set of ranking functions and pairs to be added (odd variant)
 * @param s1 Set of ranking funtions
 * @param s2 Vector of pairs to be added to s1
 * @param rem Number of remaining tight positions
 * @param rel Direct simulation
 * @param oddRel Rank simulation
 * @param max Maximum rank
 * @param reachRes Reachability restriction (SuccRank)
 * @param maxReach Maximum reachable macrostate
 * @param useInverse Use inverse function
 * @return Set of combinations of ranking functions with joined pairs
 */
vector<RankFunc> RankFunc::cartTightProductMapOdd(vector<RankFunc>& s1, vector<std::pair<int, int> >& s2,
    int rem, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax, bool useInverse)
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
      tmp.addPair(v2, useInverse);
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


/*
 * Check if combination or tmp with joined act fulfills direct simulation restr
 * @param act Pair
 * @param tmp Ranking function
 * @param rel Direct simulation
 * @return DirSim Compatibility
 */
bool RankFunc::checkDirectBackRel(const std::pair<int, int>& act, const RankFunc& tmp, BackRel& rel)
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


/*
 * Check if combination or tmp with joined act fulfills rank simulation restr
 * @param act Pair
 * @param tmp Ranking function
 * @param oddRel Direct simulation
 * @return RankSim Compatibility
 */
bool RankFunc::checkOddBackRel(const std::pair<int, int>& act, const RankFunc& tmp, BackRel& oddRel)
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


/*
 * Generate all ranking functions based on restriction slist
 * @param slist Restriction of ranking functions
 * @param rel Direct simulation
 * @param oddRel Rank simulation
 * @param max Maximum rank
 * @param reachRes Reachability restriction (SuccRank)
 * @param reachMax Maximum reachable macrostate
 * @param useInverse Use inverse function
 * @return Set of all ranking functions
 */
vector<RankFunc> RankFunc::cartTightProductMapList(RankConstr slist, BackRel& rel, BackRel& oddRel,
    int max, map<int, int>& reachRes, int reachMax, bool useInverse)
{
  vector<RankFunc> ret;
  if(slist.size() == 0)
    return ret;

  for(auto p : slist[0])
  {
    RankFunc sing(map<int, int>({p}), useInverse);
    sing.setReachRestr(p.second + 2*(reachMax - reachRes[p.first]));
    ret.push_back(sing);
  }
  for(int i = 1; i < (int)slist.size(); i++)
  {
    ret = RankFunc::cartTightProductMap(ret, slist[i], slist.size() - i - 1, rel, oddRel,
      max, reachRes, reachMax, useInverse);
  }
  return ret;
}


/*
 * Generate all ranking functions based on restriction slist (odd version)
 * @param slist Restriction of ranking functions
 * @param rel Direct simulation
 * @param oddRel Rank simulation
 * @param max Maximum rank
 * @param reachRes Reachability restriction (SuccRank)
 * @param reachMax Maximum reachable macrostate
 * @param useInverse Use inverse function
 * @return Set of all ranking functions
 */
vector<RankFunc> RankFunc::cartTightProductMapListOdd(RankConstr slist, BackRel& rel, BackRel& oddRel,
    int max, map<int, int>& reachRes, int reachMax, bool useInverse)
{
  vector<RankFunc> ret;
  vector<RankFunc> retcp;
  if(slist.size() == 0)
    return ret;
  set<int> states;

  for(auto p : slist[0])
  {
    RankFunc sing(map<int, int>({p}), useInverse);
    sing.setReachRestr(p.second + 2*(reachMax - reachRes[p.first]));
    ret.push_back(sing);
    states.insert(p.first);
  }
  for(int i = 1; i < (int)slist.size(); i++)
  {
    if(slist[i].size() == 0)
      return ret;
    ret = RankFunc::cartTightProductMapOdd(ret, slist[i], slist.size() - i - 1, rel, oddRel, max,
      reachRes, reachMax, useInverse);
    states.insert(slist[i][0].first);
    //std::cout << i << " : " << ret.size() << std::endl;
  }

  for(unsigned i = 0; i < ret.size(); i++)
  {
    if(ret[i].getMaxRank() % 2 == 0 || ret[i].remTightCount() != 0)
      continue;
    auto odd(ret[i].getOddStates());
    for(int s : states)
    {
      if(odd.find(s) == odd.end())
      {
        ret[i].addPair({s, ret[i].getMaxRank() - 1}, useInverse);
      }
    }
    retcp.push_back(ret[i]);
  }

  return retcp;
}


/*
 * Generate all ranking functions based on restriction constr (basic version)
 * @param constr Restriction of ranking functions
 * @return Set of all ranking functions
 */
vector<RankFunc> RankFunc::fromRankConstr(RankConstr constr)
{
  vector<vector<std::pair<int,bool> > > emp;
  map<int, int> empMap;
  return RankFunc::cartTightProductMapList(constr, emp, emp, -1, empMap, INF, false);
}


/*
 * Generate all tight ranking functions based on restriction (no max rank, tight start)
 * @param constr Restriction of ranking functions
 * @param rel Direct simulation
 * @param oddRel Rank simulation
 * @param reachRes Reachability restriction (SuccRank)
 * @param reachMax Maximum reachable macrostate
 * @param useInverse Use inverse function
 * @return Set of all tight ranking functions
 */
vector<RankFunc> RankFunc::tightFromRankConstr(RankConstr constr, BackRel& rel, BackRel& oddRel,
    map<int, int>& reachRes, int reachMax, bool useInverse)
{
  return RankFunc::cartTightProductMapList(constr, rel, oddRel, -1, reachRes, reachMax, useInverse);
}


/*
 * Generate all tight ranking functions based on restriction (odd version)
 * @param constr Restriction of ranking functions
 * @param rel Direct simulation
 * @param oddRel Rank simulation
 * @param reachRes Reachability restriction (SuccRank)
 * @param reachMax Maximum reachable macrostate
 * @param useInverse Use inverse function
 * @return Set of all tight ranking functions
 */
vector<RankFunc> RankFunc::tightFromRankConstrOdd(RankConstr constr, BackRel& rel, BackRel& oddRel,
    map<int, int>& reachRes, int reachMax, bool useInverse)
{
  return RankFunc::cartTightProductMapListOdd(constr, rel, oddRel, -1, reachRes, reachMax, useInverse);
}


/*
 * Generate all tight ranking functions based on restriction (tailored for
 * successors with given maximum rank)
 * @param constr Restriction of ranking functions
 * @param rel Direct simulation
 * @param oddRel Rank simulation
 * @param max Maximum rank
 * @param reachRes Reachability restriction (SuccRank)
 * @param reachMax Maximum reachable macrostate
 * @param useInverse Use inverse function
 * @return Set of all tight ranking functions
 */
vector<RankFunc> RankFunc::tightSuccFromRankConstr(RankConstr constr, BackRel& rel, BackRel& oddRel,
    int max, map<int, int>& reachRes, int reachMax, bool useInverse)
{
  return RankFunc::cartTightProductMapList(constr, rel, oddRel, max, reachRes, reachMax, useInverse);
}


/*
 * does the ranking function fulfills the RankRestr constraint
 * @param prev Ranking function
 * @param succ RankRestr restriction
 * @return True valid, otherwise false
 */
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


/*
 * Is the ranking function consistent wrt max ranks of each state
 * @param maxRank Vector of maximum ranks
 * @return Max rank consistency
 */
bool RankFunc::isMaxRankValid(vector<int>& maxRank) const
{
  for(unsigned i = 0; i < maxRank.size(); i++)
  {
    if(maxRank[i] < this->ranks[i])
    {
      return false;
    }
  }
  return true;
}


/*
 * Convert ranking function to string
 * @return String representation
 */
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


/*
 * Is the ranking function tight
 * @return Tight?
 */
bool RankFunc::isTightRank() const
{
  // for(const auto& t : this->tight)
  // {
  //   if(!t)  return false;
  // }
  // return true;
  return this->tight.all();
}


/*
 * Has even states assigned maximum even rank
 * @return Even check
 */
bool RankFunc::eqEven() const
{
  for(auto v : *this)
  {
    if(v.second % 2 == 0 && v.second != this->maxRank - 1)
      return false;
  }
  return true;
}

/*
 * Implementation of <
 * @return this < f
 */
bool RankFunc::isAllLeq(const RankFunc& f)
{
  vector<int> rnk = f.getRanks();
  if(this->ranks.size() != rnk.size())
    return false;
  for(unsigned i = 0; i < this->ranks.size(); i++)
  {
    if(this->ranks[i] > rnk[i])
      return false;
  }
  return true;
}


/*
 * Is the ranking function consistent wrt direct simulation
 * @param rel Direct simulation
 * @return Consistency
 */
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


/*
 * Is the ranking function consistent wrt rank simulation
 * @param rel Rank simulation
 * @return Consistency
 */
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


/*
 * Is the ranking function consistent wrt reachability constraints
 * @param res Reachability of states
 * @param reachMax Maximum macrostate reachability
 * @return Reachability consistency
 */
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


std::string RankFunc::toStringVer() const
{
  string ret;
  // for(auto i : this->tight)
  //   ret += std::to_string(i);
  return ret;
}


/*
 * Get Schewe reduced outdegree ranking functions
 * @param ranks Maximum rank
 * @param states Macrostate
 * @param fin Final states
 * @param useInverse Use inverse ranking function
 * @return Ranking functions (RO)
 */
vector<RankFunc> RankFunc::getRORanks(int ranks, std::set<int>& states, std::set<int>& fin, bool useInverse)
{
  vector<RankFunc> ret;
  set<int> nofin;
  std::set_difference(states.begin(), states.end(), fin.begin(),
    fin.end(), std::inserter(nofin, nofin.begin()));

  vector<int> nfvec(nofin.begin(), nofin.end());
  vector<vector<int>> subsets = Aux::getAllSubsets(nfvec);
  for(auto& sb : subsets)
  {
    if((int)sb.size() > ranks || sb.size() == 0)
      continue;

    vector<int> perm(sb.begin(), sb.end());
    do {
      std::map<int, int> rnk;
      int i = 1;
      for(int st : states)
      {
        if(fin.find(st) != fin.end())
          rnk.insert({st, 2*(sb.size()-1)});
        else
          rnk.insert({st, 2*sb.size()-1});
      }
      for(int item : perm)
      {
        rnk[item] = i;
        i += 2;
      }
      RankFunc fnc(rnk, useInverse);
      ret.push_back(fnc);
    } while(std::next_permutation(perm.begin(), perm.end()));
  }
  return ret;
}


/*
 * Cartesian product of a set of ranking functions and pairs to be added
 * @param s1 Set of ranking funtions
 * @param s2 Vector of pairs to be added to s1
 * @param rem Number of remaining tight positions
 * @param rel Direct simulation
 * @param oddRel Rank simulation
 * @param max Max rank
 * @param reachRes Reachability restriction (SuccRank)
 * @param reachMax Maximum reachable macrostate
 * @param useInverse Use inverse ranking function
 * @return Set of combinations of ranking functions with joined pairs
 */
vector<RankFunc> RankFunc::cartTightProductMapPure(vector<RankFunc>& s1, vector<std::pair<int, int> >& s2,
    int rem, BackRel& rel, BackRel& oddRel, int max, map<int, int>& reachRes, int reachMax, bool useInverse)
{
  vector<RankFunc> ret;
  int maxRank;
  for(const auto& v1 : s1)
  {
    for(const auto& v2 : s2)
    {
      RankFunc tmp(v1);
      tmp.addPair(v2, useInverse);
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


/*
 * Generate all ranking functions based on restriction slist (pure version)
 * @param slist Restriction of ranking functions
 * @param rel Direct simulation
 * @param oddRel Rank simulation
 * @param max Maximum rank
 * @param reachRes Reachability restriction (SuccRank)
 * @param reachMax Maximum reachable macrostate
 * @param useInverse Use inverse function
 * @return Set of all ranking functions
 */
vector<RankFunc> RankFunc::cartTightProductMapListPure(RankConstr slist, BackRel& rel, BackRel& oddRel,
    int max, map<int, int>& reachRes, int reachMax, bool useInverse)
{
  vector<RankFunc> ret;
  if(slist.size() == 0)
    return ret;

  for(auto p : slist[0])
  {
    RankFunc sing(map<int, int>({p}), useInverse);
    sing.setReachRestr(p.second + 2*(reachMax - reachRes[p.first]));
    ret.push_back(sing);
  }
  for(int i = 1; i < (int)slist.size(); i++)
  {
    ret = RankFunc::cartTightProductMap(ret, slist[i], slist.size() - i - 1, rel, oddRel,
      max, reachRes, reachMax, useInverse);
  }
  return ret;
}


/*
 * Generate all tight ranking functions based on restriction (no max rank, tight start) (pure version)
 * @param constr Restriction of ranking functions
 * @param rel Direct simulation
 * @param oddRel Rank simulation
 * @param reachRes Reachability restriction (SuccRank)
 * @param reachMax Maximum reachable macrostate
 * @param useInverse Use inverse function
 * @return Set of all tight ranking functions
 */
vector<RankFunc> RankFunc::tightFromRankConstrPure(RankConstr constr, BackRel& rel, BackRel& oddRel,
    map<int, int>& reachRes, int reachMax, bool useInverse)
{
  return RankFunc::cartTightProductMapListPure(constr, rel, oddRel, -1, reachRes, reachMax, useInverse);
}


/*
 * Generate all tight ranking functions based on restriction (no max rank, tight start) (pure version)
 * @param constr Restriction of ranking functions
 * @param rel Direct simulation
 * @param oddRel Rank simulation
 * @param max Maximum rank
 * @param reachRes Reachability restriction (SuccRank)
 * @param reachMax Maximum reachable macrostate
 * @param useInverse Use inverse function
 * @return Set of all tight ranking functions
 */
vector<RankFunc> RankFunc::tightSuccFromRankConstrPure(RankConstr constr, BackRel& rel, BackRel& oddRel,
    int max, map<int, int>& reachRes, int reachMax, bool useInverse)
{
  return RankFunc::cartTightProductMapListPure(constr, rel, oddRel, max, reachRes, reachMax, useInverse);
}
