
#include "BuchiAutomatonSpec.h"

std::set<StateKV<int> > BuchiAutomatonSpec::succSetKV(StateKV<int> state, int symbol) const
{
  std::set<StateKV<int> > ret;
  std::set<int> sprime;
  std::set<int> oprime;
  std::vector<int> maxRank(getStates().size(), 2*getStates().size());
  for(int st : state.S)
  {
    std::set<int> dst = getTransitions()[std::make_pair(st, symbol)];
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
    std::set<int> dst = getTransitions()[std::make_pair(st, symbol)];
    std::set_union(dst.begin(), dst.end(), oprime.begin(), oprime.end(),
      std::inserter(oprime, oprime.begin()));
  }
  std::vector<std::map<int, int> > ranks = getKVRanks(maxRank, sprime);
  for (auto r : ranks)
  {
    ret.insert({sprime, oprime, r});
  }
  return ret;
}


std::vector<std::map<int, int> > BuchiAutomatonSpec::getKVRanks(std::vector<int> max, std::set<int> states) const
{
  std::vector< std::set< std::vector<std::pair<int, int> > > > constr;
  std::vector<std::map<int, int> > ret;

  for(int st : states)
  {
    std::set< std::vector<std::pair<int, int> > > singleConst;
    for(int i = 0; i <= max[st]; i++)
    {
      singleConst.insert(std::vector<std::pair<int, int> >({std::make_pair(st, i)}));
    }
    constr.push_back(singleConst);
  }

  std::set< std::vector<std::pair<int, int> > > prod = Aux::cartProductList<std::pair<int, int> >(constr);
  for(auto item : prod)
  {
    ret.push_back(std::map<int, int>(item.begin(), item.end()));
  }
  return ret;
}


BuchiAutomaton<StateKV<int>, int> BuchiAutomatonSpec::complementKV() const
{
  std::stack<StateKV<int> > stack;
  std::set<StateKV<int> > comst;
  std::set<StateKV<int> > initials;
  std::set<StateKV<int> > finals;
  std::set<int> alph = getAlphabet();
  std::map<std::pair<StateKV<int>, int>, std::set< StateKV<int> > > mp;
  std::map<std::pair<StateKV<int>, int>, std::set< StateKV<int> > >::iterator it;

  std::vector<int> maxRank(getStates().size(), 2*getStates().size());
  std::vector<std::map<int, int> > ranks = getKVRanks(maxRank, getInitials());
  for (auto r : ranks)
  {
    StateKV<int> tmp = {getInitials(), std::set<int>(), r};
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
      std::set<StateKV<int>> dst;
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
