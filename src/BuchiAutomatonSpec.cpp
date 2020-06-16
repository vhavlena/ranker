
#include "BuchiAutomatonSpec.h"

set<StateKV> BuchiAutomatonSpec::succSetKV(StateKV state, int symbol) const
{
  set<StateKV> ret;
  set<int> sprime;
  set<int> oprime;
  vector<int> maxRank(getStates().size(), 2*getStates().size());
  for(int st : state.S)
  {
    set<int> dst = getTransitions()[std::make_pair(st, symbol)];
    for(int d : dst)
    {
      maxRank[d] = std::min(maxRank[d], state.f[st]);
    }
    std::set_union(dst.begin(), dst.end(), sprime.begin(), sprime.end(),
      std::inserter(sprime, sprime.begin()));
  }
  for(int i = 0; i < maxRank.size(); i++)
  {
    if(getFinals().find(i) != getFinals().end() && maxRank[i] % 2 != 0)
      maxRank[i] -= 1;
  }
  if(state.O.size() == 0)
  {
    oprime = sprime;
  }
  for(int st : state.O)
  {
    set<int> dst = getTransitions()[std::make_pair(st, symbol)];
    std::set_union(dst.begin(), dst.end(), oprime.begin(), oprime.end(),
      std::inserter(oprime, oprime.begin()));
  }
  vector<RankFunc> ranks = getKVRanks(maxRank, sprime);

  for (auto r : ranks)
  {
    set<int> oprime_tmp;
    auto odd = r.getOddStates();
    std::set_difference(oprime.begin(), oprime.end(), odd.begin(), odd.end(),
      std::inserter(oprime_tmp, oprime_tmp.begin()));
    ret.insert({sprime, oprime_tmp, r});
  }
  return ret;
}

RankConstr BuchiAutomatonSpec::rankConstr(vector<int>& max, set<int>& states) const
{
  RankConstr constr;
  for(int st : states)
  {
    vector<std::pair<int, int> > singleConst;
    for(int i = 0; i <= max[st]; i++)
    {
      singleConst.push_back(std::make_pair(st, i));
    }
    constr.push_back(singleConst);
  }
  return constr;
}


vector<RankFunc> BuchiAutomatonSpec::getKVRanks(vector<int>& max, set<int>& states) const
{
  RankConstr constr = rankConstr(max, states);
  return RankFunc::fromRankConstr(constr);
}


BuchiAutomaton<StateKV, int> BuchiAutomatonSpec::complementKV() const
{
  std::stack<StateKV> stack;
  set<StateKV> comst;
  set<StateKV> initials;
  set<StateKV> finals;
  set<int> alph = getAlphabet();
  map<std::pair<StateKV, int>, set<StateKV> > mp;
  map<std::pair<StateKV, int>, set<StateKV> >::iterator it;

  set<int> init = getInitials();
  vector<int> maxRank(getStates().size(), 2*getStates().size());
  vector<RankFunc> ranks = getKVRanks(maxRank, init);
  for (auto r : ranks)
  {
    StateKV tmp = {getInitials(), set<int>(), r};
    stack.push(tmp);
    comst.insert(tmp);
    initials.insert(tmp);
  }

  while(stack.size() > 0)
  {
    auto st = stack.top();
    stack.pop();
    if(isKVFinal(st))
      finals.insert(st);

    for(int sym : alph)
    {
      auto pr = std::make_pair(st, sym);
      set<StateKV> dst;
      for (auto s : succSetKV(st, sym))
      {
        dst.insert(s);
        if(comst.find(s) == comst.end())
        {
          stack.push(s);
          comst.insert(s);
        }
      }
      mp[pr] = dst;
    }
  }

  return BuchiAutomaton<StateKV, int>(comst, initials,
      finals, mp, alph);
}


vector<RankFunc> BuchiAutomatonSpec::getSchRanks(vector<int> max,
    set<int> states, StateSch macrostate) const
{
  RankConstr constr = rankConstr(max, states);
  vector<RankFunc> ret;

  for(RankFunc item : RankFunc::fromRankConstr(constr))
  {
    if(item.getMaxRank() == macrostate.f.getMaxRank() && item.isTightRank())
      ret.push_back(item);
  }
  return ret;
}

set<StateSch> BuchiAutomatonSpec::succSetSch(StateSch state, int symbol) const
{
  set<StateSch> ret;
  // set<int> sprime;
  // set<int> oprime;
  // vector<int> maxRank(getStates().size(), state.maxRank);
  // for(int st : state.S)
  // {
  //   set<int> dst = getTransitions()[std::make_pair(st, symbol)];
  //   for(int d : dst)
  //   {
  //     maxRank[d] = std::min(maxRank[d], state.f[st]);
  //   }
  //   std::set_union(dst.begin(), dst.end(), sprime.begin(), sprime.end(),
  //     std::inserter(sprime, sprime.begin()));
  // }
  // for(int i = 0; i < maxRank.size(); i++)
  // {
  //   if(getFinals().find(i) != getFinals().end() && maxRank[i] % 2 != 0)
  //     maxRank[i] -= 1;
  // }
  // for(int st : state.O)
  // {
  //   set<int> dst = getTransitions()[std::make_pair(st, symbol)];
  //   std::set_union(dst.begin(), dst.end(), oprime.begin(), oprime.end(),
  //     std::inserter(oprime, oprime.begin()));
  // }
  // vector<Rank> ranks = getSchRanks(maxRank, sprime, state);
  // for (auto r : ranks)
  // {
  //   ret.insert({sprime, oprime, r});
  // }
  return ret;
}
