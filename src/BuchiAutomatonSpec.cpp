
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


void BuchiAutomatonSpec::getSchRanksTight(vector<RankFunc>& out, vector<int>& max,
    set<int>& states, StateSch& macrostate,
    map<int, int> reachCons, int reachMax, BackRel& dirRel, BackRel& oddRel)
{
  RankConstr constr;
  map<int, int> sngmap;

  set<int> fin = getFinals();
  for(int st : states)
  {
    vector<std::pair<int, int> > singleConst;
    if(fin.find(st) == fin.end() /*max[st] % 2 != 0*/)
    {
      for(int i = 0; i < max[st]; i+= 1)
        singleConst.push_back(std::make_pair(st, i));
    }
    else //if(reachMax - reachCons[st] > 1) //BEWARE
    {
      for(int i = 0; i < max[st]; i+= 2)
        singleConst.push_back(std::make_pair(st, i));
    }

    sngmap[st] = max[st];
    singleConst.push_back(std::make_pair(st, max[st]));
    constr.push_back(singleConst);
  }

  //std::cout << " --- " << std::endl;
  //auto ret = RankFunc::tightSuccFromRankConstr(constr, dirRel, oddRel, macrostate.f.getMaxRank(), reachCons, reachMax);
  //std::cout << macrostate.toString() << " : " << ret.size() << std::endl;
  //return ret;

  auto tmp = RankFunc::tightSuccFromRankConstr(constr, dirRel, oddRel, macrostate.f.getMaxRank(), reachCons, reachMax);
  // map<int, int> a = {{0,3}, {2,1}, {4,3}, {5,3}, {7,2}, {10,3}};
  // RankFunc aa(a);
  // std::cout << aa.toStringVer() << " : " << aa.isTightRank() << std::endl;
  //std::cout << aa.toString() << std::endl;
  // if(macrostate.S == set<int>({0,2,4,5,7,10}) && macrostate.f == aa)
  //   std::cout << macrostate.toString() << " : " << tmp.size() << std::endl;

  //out = tmp;
  RankFunc sng(sngmap);
  if(sng.isTightRank() && sng.getMaxRank() == macrostate.f.getMaxRank() && tmp.size() > 0)
    out = vector<RankFunc>({sng});
  else
  {
    //std::cout << sng.toString() << std::endl;
    out = vector<RankFunc>();
  }
  //out = RankFunc::tightSuccFromRankConstr(constr, dirRel, oddRel, macrostate.f.getMaxRank(), reachCons, reachMax);
}

vector<StateSch> BuchiAutomatonSpec::succSetSchStart(set<int>& state, int rankBound,
    map<int, int> reachCons, map<DFAState, int> maxReach, BackRel& dirRel,
    BackRel& oddRel)
{
  vector<StateSch> ret;
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
  for(const RankFunc& item : RankFunc::tightFromRankConstr(constr, dirRel, oddRel, reachCons, reachMaxAct))
  {
    ret.push_back({sprime, set<int>(), item, 0, true});
  }
  return ret;
}


bool BuchiAutomatonSpec::getRankSuccCache(vector<RankFunc>& out, StateSch& state, int symbol)
{
  auto it = this->rankCache.find({state.S, symbol, state.f.getMaxRank()});
  if(it == this->rankCache.end())
  {
    this->rankCache[{state.S, symbol, state.f.getMaxRank()}] = vector<std::pair<RankFunc, vector<RankFunc>>>();
  }
  else
  {
    for(auto& item : it->second)
    {
      if(state.f.isAllLeq(item.first))
      {
        out = item.second;
        return true;
      }
    }
  }
  return false;
}


vector<StateSch> BuchiAutomatonSpec::succSetSchTight(StateSch& state, int symbol,
    map<int, int> reachCons, map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel)
{
  vector<StateSch> ret;
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
    if(fin.find(st) == fin.end())
      succ[st] = dst;

    // BEWARE
    // if(state.f.find(st)->second == 0)
    // {
    //   return ret;
    // }
    if(state.f.find(st)->second == 0 && reachCons[st] > 0)
    {
      return ret;
    }
    if(dst.size() == 0 && state.f.find(st)->second != 0)
    {
      return ret;
    }
  }

  if(this->rankBound[state.S] > state.f.getMaxRank() || this->rankBound[sprime] > state.f.getMaxRank())
  {
    return ret;
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
  set<int> inverseRank;

  if(!getRankSuccCache(tmp, state, symbol))
  {
    getSchRanksTight(tmp, maxRank, sprime, state,
        reachCons, maxReachAct, dirRel, oddRel);
    this->rankCache[{state.S, symbol, state.f.getMaxRank()}].push_back({state.f, tmp});
  }

  for (auto& r : tmp)
  {
    if(!r.isSuccValid(state.f, succ) ||  !r.isMaxRankValid(rnkBnd))
      continue;
    set<int> oprime_tmp;
    inverseRank = r.inverseRank(iprime);
    if (state.O.size() == 0)
      oprime_tmp = inverseRank;
    else
      std::set_intersection(oprime.begin(),oprime.end(),inverseRank.begin(),
        inverseRank.end(), std::inserter(oprime_tmp, oprime_tmp.begin()));
    ret.push_back({sprime, oprime_tmp, r, iprime, true});
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
  vector<StateSch> succ;
  set<int> alph = getAlphabet();
  map<std::pair<StateSch, int>, set<StateSch> > mp;
  map<std::pair<StateSch, int>, vector<StateSch> > mpVect;
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
  {
    if(tmp.S.size() > 0)
    {
      stack.push(tmp);
    }
  }
  //std::cout << nfaStates.size() << std::endl;

  // Compute rank upper bound on the macrostates
  this->rankBound = this->getRankBound(comp, slIgnore, maxReach, reachCons);

  StateSch init = {getInitials(), set<int>(), RankFunc(), 0, false};
  initials.insert(init);

  set<int> cl;
  this->computeRankSim(cl);

  BackRel dirRel = createBackRel(this->getDirectSim());
  BackRel oddRel = createBackRel(this->getOddRankSim());

  bool cnt = true;

  while(stack.size() > 0)
  {
    StateSch st = stack.top();
    stack.pop();
    if(isSchFinal(st))
      finals.insert(st);
    cnt = true;

    for(int sym : alph)
    {
      auto pr = std::make_pair(st, sym);
      //set<StateSch> dst;
      if(st.tight)
      {
        succ = succSetSchTight(st, sym, reachCons, maxReach, dirRel, oddRel);
        //succ = set<StateSch>();
      }
      else
      {
        succ = succSetSchStart(st.S, rankBound[st.S], reachCons, maxReach, dirRel, oddRel);
        cnt = false;
      }
      for (const StateSch& s : succ)
      {
        //dst.insert(s);
        if(comst.find(s) == comst.end())
        {
          stack.push(s);
          comst.insert(s);
        }
      }
      mpVect[pr] = succ; //dst;
      if(!cnt) break;
    }
    //std::cout << comst.size() << " : " << stack.size() << std::endl;
  }

  return BuchiAutomaton<StateSch, int>(comst, finals,
    initials, mp, alph);
}


void BuchiAutomatonSpec::getSchRanksTightReduced(vector<RankFunc>& out, vector<int>& max,
    set<int>& states, int symbol, StateSch& macrostate,
    map<int, int> reachCons, int reachMax, BackRel& dirRel, BackRel& oddRel)
{
  RankConstr constr;
  map<int, int> sngmap;

  set<int> fin = getFinals();
  vector<int> rnkBnd;
  for(int st : states)
  {
    vector<std::pair<int, int> > singleConst;
    if(fin.find(st) == fin.end() /*max[st] % 2 != 0*/)
    {
      for(int i = 0; i < max[st]; i+= 1)
        singleConst.push_back(std::make_pair(st, i));
    }
    else //if(reachMax - reachCons[st] > 1) //BEWARE
    {
      for(int i = 0; i < max[st]; i+= 2)
        singleConst.push_back(std::make_pair(st, i));
    }

    sngmap[st] = max[st];
    singleConst.push_back(std::make_pair(st, max[st]));
    constr.push_back(singleConst);
    rnkBnd.push_back(max[st]);
  }

  vector<RankFunc> tmp;
  int rankSetSize = 0;

  if(!getRankSuccCache(tmp, macrostate, symbol))
  {
    tmp = RankFunc::tightSuccFromRankConstr(constr, dirRel, oddRel, macrostate.f.getMaxRank(), reachCons, reachMax);
    this->rankCache[{macrostate.S, symbol, macrostate.f.getMaxRank()}].push_back({macrostate.f, tmp});
    rankSetSize = tmp.size();
  }
  else
  {
    rankSetSize = tmp.size();
    for(auto& r : tmp)
    {
      if(!r.isMaxRankValid(rnkBnd))
        rankSetSize--;
    }
  }

  RankFunc sng(sngmap);
  if(sng.isTightRank() && sng.getMaxRank() == macrostate.f.getMaxRank() && rankSetSize > 0)
    out = vector<RankFunc>({sng});
  else
  {
    out = vector<RankFunc>();
  }
}


vector<StateSch> BuchiAutomatonSpec::succSetSchTightReduced(StateSch& state, int symbol,
    map<int, int> reachCons, map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel)
{
  vector<StateSch> ret;
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
    if(fin.find(st) == fin.end())
      succ[st] = dst;

    // BEWARE
    // if(state.f.find(st)->second == 0)
    // {
    //   return ret;
    // }
    // if(state.f.find(st)->second == 0 && reachCons[st] > 0)
    // {
    //   return ret;
    // }
    // if(dst.size() == 0 && state.f.find(st)->second != 0)
    // {
    //   return ret;
    // }
  }

  if(this->rankBound[state.S]*2-1 < state.f.getMaxRank() || this->rankBound[sprime]*2-1 < state.f.getMaxRank())
  {
    return ret;
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
  set<int> inverseRank;
  vector<RankFunc> maxRanks;

  getSchRanksTightReduced(maxRanks, maxRank, sprime, symbol, state,
      reachCons, maxReachAct, dirRel, oddRel);

  for (auto& r : maxRanks)
  {
    set<int> oprime_tmp;
    if(this->opt.cutPoint)
    {
      inverseRank = r.inverseRank(iprime);
      if (state.O.size() == 0)
        oprime_tmp = inverseRank;
      else
        std::set_intersection(oprime.begin(),oprime.end(),inverseRank.begin(),
          inverseRank.end(), std::inserter(oprime_tmp, oprime_tmp.begin()));
    }
    else
    {
      auto odd = r.getOddStates();
      std::set_difference(oprime.begin(), oprime.end(), odd.begin(), odd.end(),
        std::inserter(oprime_tmp, oprime_tmp.begin()));
      iprime = 0;
    }
    ret.push_back({sprime, oprime_tmp, r, iprime, true});
  }

  set<StateSch> retAll;
  for(const StateSch& st : ret)
  {
    retAll.insert(st);
    map<int, int> rnkMap((map<int, int>)st.f);

    if(this->opt.cutPoint)
    {
      if(st.i != 0 || st.O.size() == 0)
      {
        for(int o : st.O)
        {
          rnkMap[o]--;
        }
        retAll.insert({st.S, set<int>(), RankFunc(rnkMap), st.i, true});
      }
    }
    else
    {
      set<int> no;
      for(int o : st.O)
      {
        if(rnkMap[o] > 0)
          rnkMap[o]--;
        else
          no.insert(o);
      }
      retAll.insert({st.S, no, RankFunc(rnkMap), st.i, true});
    }
  }

  return vector<StateSch>(retAll.begin(), retAll.end());
}

vector<StateSch> BuchiAutomatonSpec::succSetSchStartReduced(set<int>& state, int rankBound,
    map<int, int> reachCons, map<DFAState, int> maxReach, BackRel& dirRel,
    BackRel& oddRel)
{
  vector<StateSch> ret;
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
  auto tmp = RankFunc::tightFromRankConstrOdd(constr, dirRel, oddRel, reachCons, reachMaxAct);
  vector<RankFunc> maxRanks;
  bool cnt = true;
  for(auto& r : tmp)
  {
    cnt = true;
    for(auto& rp : tmp)
    {
      if(r != rp && r.isAllLeq(rp) && r.getMaxRank() == rp.getMaxRank())
      {
        cnt = false;
        break;
      }
    }
    if(cnt) maxRanks.push_back(r);
  }


  for(const RankFunc& item : maxRanks)
  {
    ret.push_back({sprime, set<int>(), item, 0, true});
  }
  return ret;
}



BuchiAutomaton<StateSch, int> BuchiAutomatonSpec::complementSchReduced()
{
  std::stack<StateSch> stack;
  set<StateSch> comst;
  set<StateSch> initials;
  set<StateSch> finals;
  vector<StateSch> succ;
  set<int> alph = getAlphabet();
  map<std::pair<StateSch, int>, set<StateSch> > mp;
  map<std::pair<StateSch, int>, vector<StateSch> > mpVect;
  map<std::pair<StateSch, int>, set<StateSch> >::iterator it;

  // NFA part of the Schewe construction
  BuchiAutomaton<StateSch, int> comp = this->complementSchNFA(this->getInitials());
  map<std::pair<StateSch, int>, set<StateSch>> prev = comp.getReverseTransitions();
  //std::cout << comp.toGraphwiz() << std::endl;

  set<StateSch> slIgnore = this->nfaSlAccept(comp);
  set<pair<DFAState,int>> slNonEmpty = this->nfaSingleSlNoAccept(comp);
  set<StateSch> ignoreAll;
  for(const auto& t : slNonEmpty)
    ignoreAll.insert({t.first, set<int>(), RankFunc(), 0, false});
  ignoreAll.insert(slIgnore.begin(), slIgnore.end());
  set<StateSch> nfaStates = comp.getStates();
  comst.insert(nfaStates.begin(), nfaStates.end());

  // Compute reachability restrictions
  map<int, int> reachCons = this->getMinReachSize();
  map<DFAState, int> maxReach = this->getMaxReachSize(comp, slIgnore);

  mp.insert(comp.getTransitions().begin(), comp.getTransitions().end());
  finals = set<StateSch>(comp.getFinals());
  // Compute states necessary to generate in the tight part
  set<StateSch> tightStart = comp.getCycleClosingStates(ignoreAll);
  for(const StateSch& tmp : tightStart)
  {
    if(tmp.S.size() > 0)
    {
      stack.push(tmp);
    }
  }

  int newState = this->getTransitions().size(); //Assumes numbered states: from 0, no gaps
  map<pair<DFAState,int>, StateSch> slTrans;
  for(const auto& pr : slNonEmpty)
  {
    StateSch ns = { set<int>({newState}), set<int>(), RankFunc(), 0, false };
    StateSch src = { pr.first, set<int>(), RankFunc(), 0, false };
    slTrans[pr] = ns;
    mp[{ns,pr.second}] = set<StateSch>({ns});
    mp[{src, pr.second}].insert(ns);
    finals.insert(ns);
    comst.insert(ns);
    newState++;
  }


  // Compute rank upper bound on the macrostates
  this->rankBound = this->getRankBound(comp, ignoreAll, maxReach, reachCons);

  StateSch init = {getInitials(), set<int>(), RankFunc(), 0, false};
  initials.insert(init);

  set<int> cl;
  this->computeRankSim(cl);

  BackRel dirRel = createBackRel(this->getDirectSim());
  BackRel oddRel = createBackRel(this->getOddRankSim());

  bool cnt = true;

  while(stack.size() > 0)
  {
    StateSch st = stack.top();
    stack.pop();
    if(isSchFinal(st))
      finals.insert(st);
    cnt = true;

    for(int sym : alph)
    {
      auto pr = std::make_pair(st, sym);
      set<StateSch> dst;
      if(st.tight)
      {
        succ = succSetSchTightReduced(st, sym, reachCons, maxReach, dirRel, oddRel);
      }
      else
      {
        succ = succSetSchStartReduced(st.S, rankBound[st.S], reachCons, maxReach, dirRel, oddRel);
        cnt = false;
      }
      for (const StateSch& s : succ)
      {
        dst.insert(s);
        if(comst.find(s) == comst.end())
        {
          stack.push(s);
          comst.insert(s);
        }
      }

      auto it = slTrans.find({st.S, sym});
      if(it != slTrans.end())
      {
        dst.insert(it->second);
      }
      if(!st.tight)
      {
        if(!cnt)
        {
          for(const auto& a : this->getAlphabet())
          {
            for(const auto& d : prev[{st, a}])
              mp[{d,a}].insert(dst.begin(), dst.end());
          }
        }
        else
        {
          mp[pr].insert(dst.begin(), dst.end());
        }
      }
      else
        mp[pr] = dst;
      if(!cnt) break;
    }
    //std::cout << comst.size() << " : " << stack.size() << std::endl;
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
        StateSch nt = {succSet(st.S, sym), set<int>(), RankFunc(), 0, false};
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
    if(alph.size() == 1)
    {
      if(acceptSl(st, alph))
        slAccept.insert(st);
    }
  }
  return slAccept;
}


set<pair<DFAState,int>> BuchiAutomatonSpec::nfaSingleSlNoAccept(BuchiAutomaton<StateSch, int>& nfaSchewe)
{
  vector<int> alph;
  set<pair<DFAState,int>> slNoAccept;
  for(StateSch st : nfaSchewe.getStates())
  {
    if(st.tight)  continue;
    alph = nfaSchewe.containsSelfLoop(st);
    if(alph.size() == 1)
    {
      if(!acceptSl(st, alph))
        slNoAccept.insert({st.S, alph[0]});
    }
  }
  return slNoAccept;
}


map<DFAState, int> BuchiAutomatonSpec::getRankBound(BuchiAutomaton<StateSch, int>& nfaSchewe, set<StateSch>& slignore, map<DFAState, int>& maxReachSize, map<int, int>& minReachSize)
{
  set<int> nofin;
  set<int> fin = this->getFinals();
  std::set_difference(this->getStates().begin(), this->getStates().end(), fin.begin(),
    fin.end(), std::inserter(nofin, nofin.begin()));
  vector<int> states(nofin.begin(), nofin.end());
  map<StateSch, int> rnkmap;

  for(const StateSch& s : nfaSchewe.getStates())
  {
    rnkmap[s] = 0;
  }

  for(vector<int>& sub : Aux::getAllSubsets(states))
  {
    set<int> st(sub.begin(), sub.end());
    this->computeRankSim(st);
    int classes = Aux::countEqClasses(this->getStates().size(), st, this->getOddRankSim());
    //cout << StateSch::printSet(st) << " : " << classes << endl;
    for(const StateSch& s : nfaSchewe.getStates())
    {
      if(std::includes(s.S.begin(), s.S.end(), st.begin(), st.end()))
      {
        rnkmap[s] = std::max(rnkmap[s], classes);
      }
    }
  }

  // for(const auto& k : rnkmap)
  // {
  //   cout << k.first.toString() << " : " << k.second << std::endl;
  // }


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

  auto initMaxFnc = [this, &maxReachSize, &minReachSize, &rnkmap] (const StateSch& act) -> int
  {
    set<int> ret;
    set<int> fin = this->getFinals();
    std::set_difference(act.S.begin(),act.S.end(),fin.begin(),
      fin.end(), std::inserter(ret, ret.begin()));

    int maxCnt = 0;
    int maxReach = maxReachSize[act.S];
    int minReach = INF;
    vector<int> rechCount(maxReach + 1);
    for(int st : ret)
    {
      if(minReachSize[st] == maxReach)
        maxCnt++;
      minReach = std::min(minReach, minReachSize[st]);
      if(minReachSize[st] <= maxReach)
        rechCount[minReachSize[st]]++;
    }
    int tmp = INF;
    for(int i = 0; i < (int)rechCount.size(); i++)
    {
      if(rechCount[i] > maxReach - i)
      {
        tmp = std::min(tmp, ((int)ret.size() - rechCount[i]) + maxReach - i + 1);
      }
    }
    int rank = std::min((int)ret.size(), tmp);
    for(int st : fin)
    {
      if(minReachSize[st] == maxReach)
        return 0;
    }
    if(maxCnt > 2)
      rank = std::min(rank, (int)ret.size() - maxCnt + 1);
    if(minReach != INF)
      rank = std::max(std::min(rank, maxReach - minReach + 1), 0);
    // if(this->containsRankSimEq(ret) && ret.size() > 1)
    //   rank = std::min(rank, std::max((int)ret.size() - 1, 0));
    rank = std::min(rank, rnkmap[act]);
    return rank;
  };

  auto tmp = nfaSchewe.propagateGraphValues(updMaxFnc, initMaxFnc);
  map<DFAState, int> ret;
  for(const auto& t : tmp)
    ret[t.first.S] = t.second;
  return ret;
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


map<int, int> BuchiAutomatonSpec::getMaxReachSizeInd()
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
    return std::max(a->label, m);
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


// int BuchiAutomatonSpec::countEquivalenceClasses(vector<int>& st, StateRelation& rel)
// {
//   return ret.size();
// }
