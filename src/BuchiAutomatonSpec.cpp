
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
    for(int i = 0; i <= max[st]; i += inc)
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
    map<int, int> reachCons, int reachMax, BackRel& dirRel, BackRel& oddRel)
{
  RankConstr constr;

  set<int> fin = getFinals();
  for(int st : states)
  {
    vector<std::pair<int, int> > singleConst;
    if(fin.find(st) == fin.end() /*max[st] % 2 != 0*/)
    {
      for(int i = 0; i < max[st]; i+= 1)
        singleConst.push_back(std::make_pair(st, i));
    }
    else if(reachMax - reachCons[st] > 1) //BEWARE
    {
      for(int i = 0; i < max[st]; i+= 2)
        singleConst.push_back(std::make_pair(st, i));
    }

    singleConst.push_back(std::make_pair(st, max[st]));
    constr.push_back(singleConst);
  }

  //vector<RankFunc> ret;
  //vector<RankFunc> tmp = RankFunc::tightSuccFromRankConstr(constr, dirRel, oddRel, macrostate.f.getMaxRank(), reachCons, reachMax);
  vector<RankFunc> ret = RankFunc::tightSuccFromRankConstr(constr, dirRel, oddRel, macrostate.f.getMaxRank(), reachCons, reachMax);
  // for(RankFunc item : tmp)
  // {
  //   if(/* item.isSuccValid(macrostate.f, succ) && */ item.isReachConsistent(reachCons, reachMax))
  //     ret.push_back(item);
  // }

  //std::cout << macrostate.toString() << " : " << tmp.size() << " : " << ret.size() << std::endl;
  return ret;
}

set<StateSch> BuchiAutomatonSpec::succSetSchStart(set<int>& state, int symbol, int rankBound,
    map<int, int> reachCons, map<DFAState, int> maxReach, BackRel& dirRel,
    BackRel& oddRel)
{
  set<StateSch> ret;
  set<int> sprime = state;
  set<int> schfinal;
  set<int> fin = getFinals();
  std::set_difference(sprime.begin(),sprime.end(),fin.begin(),
    fin.end(), std::inserter(schfinal, schfinal.begin()));
  int m = std::min((int)(2*schfinal.size() - 1), 2*rankBound - 1);
  vector<int> maxRank(getStates().size(), m);

  for(int st : sprime)
  {
    if(fin.find(st) != fin.end() && maxRank[st] % 2 != 0)
      maxRank[st] -= 1;
  }

  int reachMaxAct = maxReach[sprime];
  RankConstr constr = rankConstr(maxRank, sprime);
  for(RankFunc item : RankFunc::tightFromRankConstr(constr, dirRel, oddRel, reachCons, reachMaxAct))
  {
    // if(item.isReachConsistent(reachCons, reachMaxAct))
    // {
    ret.insert({sprime, set<int>(), item, 0, true});
    //}
  }
  return ret;
}

set<StateSch> BuchiAutomatonSpec::succSetSchTight(StateSch& state, int symbol,
    map<int, int> reachCons, map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel, SuccRankCache& match)
{
  set<StateSch> ret;
  set<int> sprime;
  set<int> oprime;
  int iprime;
  vector<int> maxRank(getStates().size(), state.f.getMaxRank());
  map<int, set<int> > succ;
  auto fin = getFinals();

  for(int st : state.S)
  {
    set<int> dst = getTransitions()[std::make_pair(st, symbol)];
    for(int d : dst)
    {
      maxRank[d] = std::min(maxRank[d], state.f[st]);
    }
    sprime.insert(dst.begin(), dst.end());
    // if(state.f.find(st)->second % 2 != 0)
    //   succ[st] = dst;
    if(fin.find(st) == fin.end())
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

  vector<int> rnkBnd;
  for(int i : sprime)
  {
    rnkBnd.push_back(maxRank[i]);
  }

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

  int maxReachAct = maxReach[sprime];
  vector<RankFunc> ranks;
  vector<RankFunc> tmp;
  // vector<RankFunc> ranks = getSchRanksTight(maxRank, sprime, state, succ,
  //     reachCons, maxReachAct, dirRel, oddRel);
  set<int> inverseRank;

  bool fn = false;
  auto it = match.find({state.S, symbol, state.f.getMaxRank()});
  if(it == match.end())
    match[{state.S, symbol, state.f.getMaxRank()}] = vector<std::pair<RankFunc, vector<RankFunc>>>();
  else
  {
    for(auto item : it->second)
    {
      auto f = item.first;
      if(state.f.isAllLeq(f))
      {
        fn = true;
        tmp = item.second;
        break;
      }
    }
  }
  if(!fn)
  {
    tmp = getSchRanksTight(maxRank, sprime, state, succ,
        reachCons, maxReachAct, dirRel, oddRel);
    match[{state.S, symbol, state.f.getMaxRank()}].push_back({state.f, tmp});
  }

  for(RankFunc x : tmp)
  {
    if(x.isSuccValid(state.f, succ) && x.isMaxRankValid(rnkBnd))
      ranks.push_back(x);
  }


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

  // NFA part of the Schewe construction
  BuchiAutomaton<StateSch, int> comp = this->complementSchNFA(this->getInitials());
  set<StateSch> slIgnore = this->nfaSlAccept(comp);
  set<StateSch> nfaStates = comp.getStates();
  comst.insert(nfaStates.begin(), nfaStates.end());

  // Compute reachability restrictions
  map<int, int> reachCons = this->getMinReachSize();
  map<DFAState, int> maxReach = this->getMaxReachSize(comp, slIgnore);

  // Compute states necessary to generate in the tight part
  set<StateSch> tightStart = comp.getCycleClosingStates(slIgnore);
  for(const StateSch& tmp : tightStart)
    stack.push(tmp);

  // Compute rank upper bound on the macrostates
  map<StateSch, int> rankBound = this->getRankBound(comp, slIgnore);

  StateSch init = {getInitials(), set<int>(), RankFunc(), 0, false};
  initials.insert(init);

  set<int> cl;
  this->computeRankSim(cl);

  BackRel dirRel = createBackRel(this->getDirectSim());
  BackRel oddRel = createBackRel(this->getOddRankSim());

  SuccRankCache match;

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
        succ = succSetSchTight(st, sym, reachCons, maxReach, dirRel, oddRel, match);
      }
      else
      {
        succ = succSetSchStart(st.S, sym, rankBound[st], reachCons, maxReach, dirRel, oddRel);
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


BuchiAutomaton<StateSch, int> BuchiAutomatonSpec::complementSchNFA(set<int>& start)
{
  std::stack<StateSch> stack;
  set<StateSch> comst;
  set<StateSch> initials;
  set<StateSch> finals;
  //set<StateSch> succ;
  set<int> alph = getAlphabet();
  map<std::pair<StateSch, int>, set<StateSch> > mp;
  map<std::pair<StateSch, int>, set<StateSch> >::iterator it;

  StateSch init = {start, set<int>(), RankFunc(), 0, false};
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
      if(!st.tight)
      {
        StateSch nt = {succSet(st.S, sym), set<int>(), RankFunc(), false};
        dst.insert(nt);
        if(comst.find(nt) == comst.end())
        {
          stack.push(nt);
          comst.insert(nt);
        }
      }
      mp[pr] = dst;
    }
  }

  return BuchiAutomaton<StateSch, int>(comst, finals,
    initials, mp, alph);
}


bool BuchiAutomatonSpec::acceptSl(StateSch& state, vector<int>& alp)
{
  set<int> rel;
  bool all = true;
  set<int> symAcc;
  set<int> fin = getFinals();
  std::stack<set<int>> stack;
  std::set<set<int>> comst;

  if(state.S.size() == 0)
    return false;

  for(int st : state.S)
  {
    if(fin.find(st) != fin.end())
      rel.insert(st);
  }
  if(rel.size() == 0)
    return false;
  for(const int& a : alp)
  {
    for(int st : rel)
    {
      set<int> sng = {st};
      stack = std::stack<set<int>>();
      comst.clear();
      stack.push(succSet(sng, a));
      comst.insert(sng);
      comst.insert(succSet(sng, a));

      while(stack.size() > 0)
      {
        set<int> pst = stack.top();
        stack.pop();
        if(pst.find(st) != pst.end())
        {
          symAcc.insert(a);
          all = true;
          break;
        }
        set<int> dst = succSet(pst, a);
        if(comst.find(dst) == comst.end())
        {
          stack.push(dst);
          comst.insert(dst);
        }
      }
      if(all) break;
    }
  }
  return symAcc.size() == alp.size();
}


set<StateSch> BuchiAutomatonSpec::nfaSlAccept(BuchiAutomaton<StateSch, int>& nfaSchewe)
{
  vector<int> alph;
  set<StateSch> slAccept;
  for(StateSch st : nfaSchewe.getStates())
  {
    if(st.tight)  continue;
    alph = nfaSchewe.containsSelfLoop(st);
    if(alph.size() > 0)
    {
      if(acceptSl(st, alph))
        slAccept.insert(st);
    }
  }
  return slAccept;
}


map<StateSch, int> BuchiAutomatonSpec::getRankBound(BuchiAutomaton<StateSch, int>& nfaSchewe, set<StateSch>& slignore)
{
  auto updMaxFnc = [&slignore] (LabelState<StateSch>* a, const std::vector<LabelState<StateSch>*> sts) -> int
  {
    int m = 0;
    for(const LabelState<StateSch>* tmp : sts)
    {
      if(tmp->state.S == a->state.S && slignore.find(a->state) != slignore.end())
        continue;
      m = std::max(m, tmp->label);
    }
    return std::min(a->label, m);
  };

  auto initMaxFnc = [this] (const StateSch& act) -> int
  {
    set<int> ret;
    set<int> fin = this->getFinals();
    std::set_difference(act.S.begin(),act.S.end(),fin.begin(),
      fin.end(), std::inserter(ret, ret.begin()));
    if(this->containsRankSimEq(ret))
      return std::max((int)ret.size() - 1, 0);
    return ret.size();
  };

  return nfaSchewe.propagateGraphValues(updMaxFnc, initMaxFnc);
}


map<DFAState, int> BuchiAutomatonSpec::getMaxReachSize(BuchiAutomaton<StateSch, int>& nfaSchewe, set<StateSch>& slIgnore)
{
  auto updMaxFnc = [&slIgnore] (LabelState<StateSch>* a, const std::vector<LabelState<StateSch>*> sts) -> int
  {
    int m = 0;
    for(const LabelState<StateSch>* tmp : sts)
    {
      if(tmp->state.S == a->state.S && slIgnore.find(a->state) != slIgnore.end())
        continue;
      m = std::max(m, tmp->label);
    }
    return std::min(a->label, m);
  };

  auto initMaxFnc = [] (const StateSch& act) -> int
  {
    return act.S.size();
  };

  auto tmp = nfaSchewe.propagateGraphValues(updMaxFnc, initMaxFnc);
  map<DFAState, int> ret;
  for(const auto& t : tmp)
    ret[t.first.S] = t.second;
  return ret;
}


map<int, int> BuchiAutomatonSpec::getMinReachSize()
{
  set<StateSch> slIgnore;
  BuchiAutomaton<StateSch, int> comp;
  map<StateSch, int> mp;
  map<int, int> ret;

  auto updMaxFnc = [&slIgnore] (LabelState<StateSch>* a, const std::vector<LabelState<StateSch>*> sts) -> int
  {
    int m = 0;
    for(const LabelState<StateSch>* tmp : sts)
    {
      if(tmp->state.S == a->state.S && slIgnore.find(a->state) != slIgnore.end())
        continue;
      m = std::max(m, tmp->label);
    }
    return std::min(a->label, m);
  };

  auto initMaxFnc = [] (const StateSch& act) -> int
  {
    return act.S.size();
  };

  for(int st : this->getStates())
  {
    set<int> ini = {st};
    comp = this->complementSchNFA(ini);
    slIgnore = this->nfaSlAccept(comp);
    auto sls = comp.getSelfLoops();
    mp = comp.propagateGraphValues(updMaxFnc, initMaxFnc);

    int val = 1000000;
    for(auto t : comp.getEventReachable(sls))
    {
      val = std::min(val, mp[t]);
    }

    ret[st] = val;
  }
  return ret;
}
