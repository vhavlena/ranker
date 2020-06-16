
#include "BuchiAutomatonSpec.h"

set<StateKV<int> > BuchiAutomatonSpec::succSetKV(StateKV<int> state, int symbol) const
{
  set<StateKV<int> > ret;
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
  for(int st : state.O)
  {
    set<int> dst = getTransitions()[std::make_pair(st, symbol)];
    std::set_union(dst.begin(), dst.end(), oprime.begin(), oprime.end(),
      std::inserter(oprime, oprime.begin()));
  }
  vector<map<int, int> > ranks = getKVRanks(maxRank, sprime);
  for (auto r : ranks)
  {
    ret.insert({sprime, oprime, r});
  }
  return ret;
}

BuchiAutomatonSpec::RankConstr BuchiAutomatonSpec::rankConstr(vector<int>& max, set<int>& states) const
{
  RankConstr constr;
  for(int st : states)
  {
    set< vector<std::pair<int, int> > > singleConst;
    for(int i = 0; i <= max[st]; i++)
    {
      singleConst.insert(vector<std::pair<int, int> >({std::make_pair(st, i)}));
    }
    constr.push_back(singleConst);
  }
  return constr;
}


vector<BuchiAutomatonSpec::Rank> BuchiAutomatonSpec::getKVRanks(vector<int>& max, set<int>& states) const
{
  RankConstr constr = rankConstr(max, states);
  vector<Rank > ret;

  set< vector<std::pair<int, int> > > prod = Aux::cartProductList<std::pair<int, int> >(constr);
  for(auto item : prod)
  {
    ret.push_back(map<int, int>(item.begin(), item.end()));
  }
  return ret;
}


BuchiAutomaton<StateKV<int>, int> BuchiAutomatonSpec::complementKV() const
{
  std::stack<StateKV<int> > stack;
  set<StateKV<int> > comst;
  set<StateKV<int> > initials;
  set<StateKV<int> > finals;
  set<int> alph = getAlphabet();
  map<std::pair<StateKV<int>, int>, set<StateKV<int> > > mp;
  map<std::pair<StateKV<int>, int>, set<StateKV<int> > >::iterator it;

  set<int> init = getInitials();
  vector<int> maxRank(getStates().size(), 2*getStates().size());
  vector<map<int, int> > ranks = getKVRanks(maxRank, init);
  for (auto r : ranks)
  {
    StateKV<int> tmp = {getInitials(), set<int>(), r};
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
      set<StateKV<int>> dst;
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

  return BuchiAutomaton<StateKV<int>, int>(comst, initials,
      finals, mp, alph);
}


bool BuchiAutomatonSpec::isTightRank(BuchiAutomatonSpec::Rank & r, int maxRank) const
{
  boost::dynamic_bitset<> rnk((maxRank+1)/2);
  for(auto v : r)
  {
    if(v.second > maxRank)
      return false;
    rnk[(v.second+1)/2 - 1] = 1;
  }
  return rnk.all();
}

vector<BuchiAutomatonSpec::Rank> BuchiAutomatonSpec::getSchRanks(vector<int> max,
    set<int> states, StateSch<int> macrostate) const
{
  RankConstr constr = rankConstr(max, states);
  vector<Rank > ret;

  set< vector<std::pair<int, int> > > prod = Aux::cartProductList<std::pair<int, int> >(constr);
  Rank tmp;
  for(auto item : prod)
  {
    tmp = Rank(item.begin(), item.end());
    if (isTightRank(tmp, macrostate.maxRank))
      ret.push_back(tmp);
  }
  return ret;
}

set<StateSch<int> > BuchiAutomatonSpec::succSetSch(StateSch<int> state, int symbol) const
{
  set<StateSch<int> > ret;
  // set<int> sprime;
  // set<int> oprime;
  // vector<int> maxRank(getStates().size(), 2*getStates().size());
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
  // vector<map<int, int> > ranks = getKVRanks(maxRank, sprime);
  // for (auto r : ranks)
  // {
  //   ret.insert({sprime, oprime, r});
  // }
  return ret;
}
