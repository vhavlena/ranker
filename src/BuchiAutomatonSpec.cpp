
#include "BuchiAutomatonSpec.h"

set<int> BuchiAutomatonSpec::succSet(set<int>& states, int symbol)
{
  set<int> ret;
  for(int st : states)
  {
    set<int> dst = getTransitions()[std::make_pair(st, symbol)];
    ret.insert(dst.begin(), dst.end());
  }
  return ret;
}


set<StateKV> BuchiAutomatonSpec::succSetKV(StateKV& state, int symbol)
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
    sprime.insert(dst.begin(), dst.end());
  }
  for(int st : sprime)
  {
    if(getFinals().find(st) != getFinals().end() && maxRank[st] % 2 != 0)
      maxRank[st] -= 1;
  }
  if(state.O.size() == 0)
  {
    oprime = sprime;
  }
  else
  {
    oprime = succSet(state.O, symbol);
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

RankConstr BuchiAutomatonSpec::rankConstr(vector<int>& max, set<int>& states)
{
  RankConstr constr;
  set<int> fin = getFinals();
  char inc = 1;
  for(int st : states)
  {
    inc = 1;
    vector<std::pair<int, int> > singleConst;
    if(fin.find(st) != fin.end())
      inc = 2;
    for(int i = inc; i <= max[st]; i += inc)
    {
      singleConst.push_back(std::make_pair(st, i));
    }
    constr.push_back(singleConst);
  }
  return constr;
}


vector<RankFunc> BuchiAutomatonSpec::getKVRanks(vector<int>& max, set<int>& states)
{
  RankConstr constr = rankConstr(max, states);
  return RankFunc::fromRankConstr(constr);
}


BuchiAutomaton<StateKV, int> BuchiAutomatonSpec::complementKV()
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

  return BuchiAutomaton<StateKV, int>(comst, finals,
    initials, mp, alph);
}


vector<RankFunc> BuchiAutomatonSpec::getSchRanksTight(vector<int>& max,
    set<int>& states, StateSch& macrostate, map<int, set<int> >& succ,
    BackRel& dirRel, BackRel& oddRel)
{
  RankConstr constr; //= rankConstr(max, states);

  set<int> fin = getFinals();
  char inc = 1;
  for(int st : states)
  {
    inc = 1;
    vector<std::pair<int, int> > singleConst;
    if(max[st] % 2 != 0)
    {
      for(int i = 1; i < max[st]; i+= 1)
        singleConst.push_back(std::make_pair(st, i));
    }

    singleConst.push_back(std::make_pair(st, max[st]));
    constr.push_back(singleConst);
  }

  vector<RankFunc> ret;

  for(RankFunc item : RankFunc::tightSuccFromRankConstr(constr, dirRel, oddRel, macrostate.f.getMaxRank()))
  {
    if(item.isSuccValid(macrostate.f, succ) && item.zeroConsistent())
      ret.push_back(item);
    // if(item.getMaxRank() == macrostate.f.getMaxRank() && item.isTightRank()
    //   && item.relConsistent(this->getDirectSim()) && item.relOddConsistent(this->getOddRankSim()))
    //   ret.push_back(item);
  }
  return ret;
}

set<StateSch> BuchiAutomatonSpec::succSetSchStart(set<int>& state, int symbol, BackRel& dirRel, BackRel& oddRel)
{
  set<StateSch> ret;
  set<int> sprime = succSet(state, symbol);
  set<int> schfinal;
  set<int> fin = getFinals();
  std::set_difference(sprime.begin(),sprime.end(),fin.begin(),
    fin.end(), std::inserter(schfinal, schfinal.begin()));
  int m = std::min((int)(2*schfinal.size() - 1), 7);
  vector<int> maxRank(getStates().size(), m);

  for(int st : sprime)
  {
    if(fin.find(st) != fin.end() && maxRank[st] % 2 != 0)
      maxRank[st] -= 1;
  }

  // bool ign = false;
  // if(sprime.find(7) != sprime.end())
  // {
  //   ign = true;
  // }
  RankConstr constr = rankConstr(maxRank, sprime);
  for(RankFunc item : RankFunc::tightFromRankConstr(constr, dirRel, oddRel))
  {
    // if(ign && item.getOddStates() == schfinal)
    //   continue;
    if(item.zeroConsistent())
    {
      ret.insert({sprime, set<int>(), item, 0, true});
    }
  }
  return ret;
}

set<StateSch> BuchiAutomatonSpec::succSetSchTight(StateSch& state, int symbol,
    BackRel& dirRel, BackRel& oddRel)
{
  set<StateSch> ret;
  set<int> sprime;
  set<int> oprime;
  int iprime;
  vector<int> maxRank(getStates().size(), state.f.getMaxRank());
  map<int, set<int> > succ;

  for(int st : state.S)
  {
    set<int> dst = getTransitions()[std::make_pair(st, symbol)];
    for(int d : dst)
    {
      maxRank[d] = std::min(maxRank[d], state.f[st]);
    }
    sprime.insert(dst.begin(), dst.end());
    succ[st] = dst;

    // BEWARE
    // if(state.f.find(st)->second == 0)
    // {
    //   return ret;
    // }
    if(dst.size() == 0 && state.f.find(st)->second != 0)
    {
      return ret;
    }
  }
  auto fin = getFinals();
  for(int st : sprime)
  {
    if(fin.find(st) != fin.end() && maxRank[st] % 2 != 0)
      maxRank[st] -= 1;
  }
  if(state.O.size() == 0)
  {
    iprime = (state.i + 2) % (state.f.getMaxRank() + 1);
  }
  else
  {
    iprime = state.i;
    oprime = succSet(state.O, symbol);
  }

  vector<RankFunc> ranks = getSchRanksTight(maxRank, sprime, state, succ, dirRel, oddRel);
  set<int> inverseRank;
  for (auto r : ranks)
  {
    set<int> oprime_tmp;
    inverseRank = r.inverseRank(iprime);
    if (state.O.size() == 0)
      oprime_tmp = inverseRank;
    else
      std::set_intersection(oprime.begin(),oprime.end(),inverseRank.begin(),
        inverseRank.end(), std::inserter(oprime_tmp, oprime_tmp.begin()));
    ret.insert({sprime, oprime_tmp, r, iprime, true});
  }
  return ret;
}


BackRel BuchiAutomatonSpec::createBackRel(BuchiAutomaton<int, int>::StateRelation& rel)
{
  BackRel bRel(this->getStates().size());
  for(auto p : rel)
  {
    if(p.first <= p.second)
      bRel[p.second].push_back({p.first, false});
    else
      bRel[p.first].push_back({p.second, true});
  }
  return bRel;
}


BuchiAutomaton<StateSch, int> BuchiAutomatonSpec::complementSch()
{
  std::stack<StateSch> stack;
  set<StateSch> comst;
  set<StateSch> initials;
  set<StateSch> finals;
  set<StateSch> succ;
  set<int> alph = getAlphabet();
  map<std::pair<StateSch, int>, set<StateSch> > mp;
  map<std::pair<StateSch, int>, set<StateSch> >::iterator it;

  StateSch init = {getInitials(), set<int>(), RankFunc(), 0, false};
  stack.push(init);
  comst.insert(init);
  initials.insert(init);

  BackRel dirRel = createBackRel(this->getDirectSim());
  BackRel oddRel = createBackRel(this->getOddRankSim());

  while(stack.size() > 0)
  {
    StateSch st = stack.top();
    stack.pop();
    if(isSchFinal(st))
      finals.insert(st);

    for(int sym : alph)
    {
      auto pr = std::make_pair(st, sym);
      set<StateSch> dst;
      if(st.tight)
      {
        succ = succSetSchTight(st, sym, dirRel, oddRel);
        //succ = set<StateSch>();
      }
      else
      {
        StateSch nt = {succSet(st.S, sym), set<int>(), RankFunc(), false};
        // dst.insert(nt);
        if(comst.find(nt) == comst.end())
        {
          stack.push(nt);
          comst.insert(nt);
        }
        //succ = succSetSchStart(st.S, sym);
        // if(st.S == set<int>({0,1,2,3,4,5}) && sym == 1)
        //   succ = succSetSchStart(st.S, sym);
        // else
          // succ = set<StateSch>();
        if(st.S == set<int>({0,2,5,6,7,9,10}) || st.S == set<int>({0,5,6,7,9,10}) || st.S == set<int>({0,1,4,6,10,11}) )
          succ = succSetSchStart(st.S, sym, dirRel, oddRel);
        else
          succ = set<StateSch>();
      }
      for (auto s : succ)
      {
        // dst.insert(s);
        if(comst.find(s) == comst.end())
        {
          stack.push(s);
          comst.insert(s);
        }
      }
      // mp[pr] = dst;
    }
    //std::cout << comst.size() << std::endl;
  }

  return BuchiAutomaton<StateSch, int>(comst, finals,
    initials, mp, alph);
}


vector<RankFunc> BuchiAutomatonSpec::getSchRanksMin(vector<int>& max,
    set<int>& states, StateSch& macrostate, map<int, set<int> >& succ)
{
  RankConstr constr; // = rankConstr(max, states);
  auto fin = getFinals();
  for(int st : states)
  {
    vector<std::pair<int, int> > singleConst;
    if(max[st] % 2 == 0 && fin.find(st) == fin.end())
    {
      singleConst.push_back(std::make_pair(st, max[st] - 1));
    }
    singleConst.push_back(std::make_pair(st, max[st]));
    constr.push_back(singleConst);
  }

  vector<RankFunc> ret;

  for(RankFunc item : RankFunc::fromRankConstr(constr))
  {
    // if(!item.isSuccValid(macrostate.f, succ))
    //   continue;
    if(item.getMaxRank() == macrostate.f.getMaxRank())
      ret.push_back(item);
  }
  return ret;
}


set<StateSch> BuchiAutomatonSpec::succSetSchTightMin(StateSch& state, int symbol)
{
  set<StateSch> ret;
  set<int> sprime;
  set<int> oprime;
  int iprime;
  vector<int> maxRank(getStates().size(), state.f.getMaxRank());
  map<int, set<int> > succ;
  set<int> zero;

  for(int st : state.S)
  {
    set<int> dst = getTransitions()[std::make_pair(st, symbol)];
    for(int d : dst)
    {
      maxRank[d] = std::min(maxRank[d], state.f[st]);
    }
    sprime.insert(dst.begin(), dst.end());
    succ[st] = dst;

    // BEWARE
    // if(state.f.find(st)->second == 0)
    // {
    //   return ret;
    // }
  }

  // if(sprime == state.S)
  // {
  //   return ret;
  // }

  auto fin = getFinals();
  for(int st : sprime)
  {
    if(fin.find(st) != fin.end() && maxRank[st] % 2 != 0)
      maxRank[st] -= 1;
  }
  if(state.O.size() == 0)
  {
    oprime = sprime;
  }
  else
  {
    oprime = succSet(state.O, symbol);
  }

  vector<RankFunc> ranks = getSchRanksMin(maxRank, sprime, state, succ);
  for (auto r : ranks)
  {
    set<int> oprime_tmp;
    auto odd = r.getOddStates();
    std::set_difference(oprime.begin(), oprime.end(), odd.begin(), odd.end(),
      std::inserter(oprime_tmp, oprime_tmp.begin()));
    ret.insert({sprime, oprime_tmp, r, 0, true});
  }
  return ret;
}

set<StateSch> BuchiAutomatonSpec::succSetSchStartMin(set<int>& state, int symbol)
{
  set<StateSch> ret;
  set<int> sprime = succSet(state, symbol);
  set<int> schfinal;
  set<int> fin = getFinals();
  std::set_difference(sprime.begin(),sprime.end(),fin.begin(),
    fin.end(), std::inserter(schfinal, schfinal.begin()));
  int m = std::min((int)(2*schfinal.size() - 1), 1000000);
  vector<int> maxRank(getStates().size(), m);
  vector<RankFunc> ranks;
  RankConstr constr;

  for(int i = m; i <= m; i+=2)
  {
    for(int st : sprime)
    {
      vector<std::pair<int, int> > singleConst;
      if(fin.find(st) != fin.end())
      {
        singleConst.push_back(std::make_pair(st, m - 1));
      }
      else
      {
        singleConst.push_back(std::make_pair(st,m));
        singleConst.push_back(std::make_pair(st,m - 1));
      }
      constr.push_back(singleConst);
    }
    //
    // RankFunc nw;
    // for (int st : sprime)
    // {
    //   if(fin.find(st) != fin.end())
    //     nw.addPair(std::make_pair(st, i - 1));
    //   else
    //     nw.addPair(std::make_pair(st, i));
    // }
    // ranks.push_back(nw);
  }

  for(RankFunc item : RankFunc::fromRankConstr(constr))
  {
    if(item.getMaxRank() % 2 == 0)
      continue;
    //if(item.isTightRank())
    {
      ret.insert({sprime, set<int>(), item, 0, true});
    }
  }
  return ret;
}


BuchiAutomaton<StateSch, int> BuchiAutomatonSpec::complementSchMin()
{
  std::stack<StateSch> stack;
  set<StateSch> comst;
  set<StateSch> initials;
  set<StateSch> finals;
  set<StateSch> succ;
  set<int> alph = getAlphabet();
  map<std::pair<StateSch, int>, set<StateSch> > mp;
  map<std::pair<StateSch, int>, set<StateSch> >::iterator it;

  StateSch init = {getInitials(), set<int>(), RankFunc(), 0, false};
  stack.push(init);
  comst.insert(init);
  initials.insert(init);

  while(stack.size() > 0)
  {
    StateSch st = stack.top();
    stack.pop();
    if(isSchFinal(st))
      finals.insert(st);

    for(int sym : alph)
    {
      auto pr = std::make_pair(st, sym);
      set<StateSch> dst;
      if(st.tight)
      {
        succ = succSetSchTightMin(st, sym);
      }
      else
      {
        StateSch nt = {succSet(st.S, sym), set<int>(), RankFunc(), false};
        dst.insert(nt);
        if(comst.find(nt) == comst.end())
        {
          stack.push(nt);
          comst.insert(nt);
        }
        if(st.S == set<int>({0,5,6}))
          succ = succSetSchStartMin(st.S, sym);
        else
          succ = set<StateSch>();
      }
      for (auto s : succ)
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
    std::cout << comst.size() << std::endl;
  }

  return BuchiAutomaton<StateSch, int>(comst, finals,
    initials, mp, alph);
}
