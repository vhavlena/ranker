
#include "BuchiAutomatonSpec.h"

/*
 * Set of all successors.
 * @param states Set of states to get successors
 * @param symbol Symbol
 * @return Set of successors over symbol
 */
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


/*
 * Set of all successors in KV construction.
 * @param state State
 * @param symbol Symbol
 * @return Set of successors over symbol in KV construction
 */
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

/*
 * Compute rank restriction for the generation of all possible ranks
 * @param max Vector of maximal ranks (indexed by states)
 * @param states Set of states in a macrostate (the S-set)
 * @return Rank restriction
 */
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


/*
 * Get all ranks in KV construction
 * @param max Vector of maximal ranks (indexed by states)
 * @param states Set of states in a macrostate (the S-set)
 * @return All ranks fulfilling the max constraint
 */
vector<RankFunc> BuchiAutomatonSpec::getKVRanks(vector<int>& max, set<int>& states)
{
  RankConstr constr = rankConstr(max, states);
  return RankFunc::fromRankConstr(constr);
}


/*
 * KV complementation proceudre
 * @return Complemented automaton
 */
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
    initials, mp, alph, getAPPattern());
}


/*
 * Get all tight ranks
 * @param out Out parameter to store tight ranks
 * @param max Vector of maximal ranks (indexed by states)
 * @param states Set of states in a macrostate (the S-set)
 * @param macrostate Current macrostate
 * @param reachCons SuccRank restriction
 * @param reachMax Maximum reachable macrostate
 * @param dirRel Direct simulation
 * @param oddRel Rank simulation
 */
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

  auto tmp = RankFunc::tightSuccFromRankConstr(constr, dirRel, oddRel, macrostate.f.getMaxRank(),
    reachCons, reachMax, true);
  RankFunc sng(sngmap, true);
  if(sng.isTightRank() && sng.getMaxRank() == macrostate.f.getMaxRank() && tmp.size() > 0)
    out = vector<RankFunc>({sng});
  else
  {
    //std::cout << sng.toString() << std::endl;
    out = vector<RankFunc>();
  }
}


/*
 * Get starting states of the tight part
 * @param state DFA macrostate
 * @param rankBound Maximum rank
 * @param macrostate Current macrostate
 * @param reachCons SuccRank restriction
 * @param reachMax Maximum reachable macrostate
 * @param dirRel Direct simulation
 * @param oddRel Rank simulation
 * @return Set of first states in the tight part
 */
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
  for(const RankFunc& item : RankFunc::tightFromRankConstr(constr, dirRel, oddRel, reachCons, reachMaxAct, true))
  {
    ret.push_back({sprime, set<int>(), item, 0, true});
  }
  return ret;
}


/*
 * Get all ranking functions (use cache memory)
 * @param out Out parameter to store tight ranks
 * @param state Schewe state (macrostate)
 * @param symbol Symbol
 * @return Is successor found in cache?
 */
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


/*
 * Get all Schewe successros
 * @param state Schewe state
 * @param symbol Symbol
 * @param reachCons SuccRank restriction
 * @param reachMax Maximum reachable macrostate
 * @param dirRel Direct simulation
 * @param oddRel Rank simulation
 * @return Set of all successors
 */
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


/*
 * Create backward relation from a relation (special representation)
 * @param rel Relation between states
 * @return Backward representation of rel
 */
BackRel BuchiAutomatonSpec::createBackRel(BuchiAutomaton<int, int>::StateRelation& rel)
{
  BackRel bRel(this->getStates().size());
  for(auto p : rel)
  {
    if(p.first == p.second)
      continue;
    if(p.first <= p.second)
      bRel[p.second].push_back({p.first, false});
    else
      bRel[p.first].push_back({p.second, true});
  }
  return bRel;
}


/*
 * Schewe complementation proceudre
 * @return Complemented automaton
 */
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
    initials, mp, alph, getAPPattern());
}


/*
 * Get all tight ranks in optimized Schewe construction
 * @param out Out parameter to store tight ranks
 * @param max Vector of maximal ranks (indexed by states)
 * @param states Set of states in a macrostate (the S-set)
 * @param macrostate Current macrostate
 * @param reachCons SuccRank restriction
 * @param reachMax Maximum reachable macrostate
 * @param dirRel Direct simulation
 * @param oddRel Rank simulation
 */
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
  int rankSetSize = 1;

  if(this->opt.succEmptyCheck && macrostate.S.size() <= this->opt.CacheMaxState && macrostate.f.getMaxRank() <= this->opt.CacheMaxRank)
  {
    if(!getRankSuccCache(tmp, macrostate, symbol))
    {
      tmp = RankFunc::tightSuccFromRankConstr(constr, dirRel, oddRel, macrostate.f.getMaxRank(),
        reachCons, reachMax, this->opt.cutPoint);
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
  }

  RankFunc sng(sngmap, this->opt.cutPoint);
  if(sng.isTightRank() && sng.getMaxRank() == macrostate.f.getMaxRank() && rankSetSize > 0)
    out = vector<RankFunc>({sng});
  else
  {
    out = vector<RankFunc>();
  }
}


/*
 * Get all Schewe successros (optimized version)
 * @param state Schewe state
 * @param symbol Symbol
 * @param reachCons SuccRank restriction
 * @param reachMax Maximum reachable macrostate
 * @param dirRel Direct simulation
 * @param oddRel Rank simulation
 * @return Set of all successors
 */
vector<StateSch> BuchiAutomatonSpec::succSetSchTightReduced(StateSch& state, int symbol,
    map<int, int> reachCons, map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel)
{
  vector<StateSch> ret;
  set<int> sprime;
  set<int> oprime;
  int iprime;
  vector<int> maxRank(getStates().size(), state.f.getMaxRank());
  map<int, set<int> > succ;
  map<int, bool > pre;
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
      if (state.O.size() == 0)
      {
        std::set_difference(sprime.begin(), sprime.end(), odd.begin(), odd.end(),
          std::inserter(oprime_tmp, oprime_tmp.begin()));
      }
      else
      {
        std::set_difference(oprime.begin(), oprime.end(), odd.begin(), odd.end(),
          std::inserter(oprime_tmp, oprime_tmp.begin()));
      }
      iprime = 0;
    }
    ret.push_back({sprime, oprime_tmp, r, iprime, true});
  }

  set<StateSch> retAll;
  for(const StateSch& st : ret)
  {
    retAll.insert(st);
    map<int, int> rnkMap((map<int, int>)st.f);

    if(state.O.size() == 0)
      continue;
    if(this->opt.cutPoint)
    {
      set<int> no;
      if(st.i != 0 || st.O.size() == 0)
      {
        for(int o : st.O)
        {
          if(rnkMap[o] > 0 && fin.find(o) == fin.end())
            rnkMap[o]--;
          else
            no.insert(o);
        }
        retAll.insert({st.S, no, RankFunc(rnkMap, this->opt.cutPoint), st.i, true});
      }
    }
    else
    {
      set<int> no;
      //bool cnt = true;
      for(int o : st.O)
      {
        if(rnkMap[o] > 0 && fin.find(o) == fin.end())
          rnkMap[o]--;
        else
          no.insert(o);
      }
      // if(!cnt)
      //   continue;
      retAll.insert({st.S, no, RankFunc(rnkMap, this->opt.cutPoint), st.i, true});
    }
  }

  return vector<StateSch>(retAll.begin(), retAll.end());
}


/*
 * Get starting states of the tight part (optimized version)
 * @param state DFA macrostate
 * @param rankBound Maximum rank
 * @param reachCons SuccRank restriction
 * @param maxReach Maximum reachable macrostate
 * @param dirRel Direct simulation
 * @param oddRel Rank simulation
 * @return Set of first states in the tight part (optimized version)
 */
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

  vector<RankFunc> maxRanks;

  if(state.size() >= this->opt.ROMinState && m >= this->opt.ROMinRank)
  {
    maxRanks = RankFunc::getRORanks(rankBound, state, fin, this->opt.cutPoint);
  }
  else
  {
    int reachMaxAct = maxReach[sprime];
    RankConstr constr = rankConstr(maxRank, sprime);
    auto tmp = RankFunc::tightFromRankConstr(constr, dirRel, oddRel, reachCons, reachMaxAct, this->opt.cutPoint);

    set<RankFunc> tmpSet(tmp.begin(), tmp.end());

    bool cnt = true;
    for(auto& r : tmp)
    {
      cnt = true;
      auto it = tmpSet.upper_bound(r);
      while(it != tmpSet.end())
      {
        if(r != (*it) && r.getMaxRank() == it->getMaxRank() && r.isAllLeq(*it))
        {
          cnt = false;
          break;
        }
        it = std::next(it, 1);
      }
      if(cnt) maxRanks.push_back(r);
    }
  }

  for(const RankFunc& item : maxRanks)
  {
    ret.push_back({sprime, set<int>(), item, 0, true});
  }
  return ret;
}


/*
 * Optimized Schewe complementation procedure
 * @return Complemented automaton
 */
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

  int newState = this->getStates().size(); //Assumes numbered states: from 0, no gaps
  map<pair<DFAState,int>, StateSch> slTrans;
  for(const auto& pr : slNonEmpty)
  {
    //std::cout << StateSch::printSet(pr.first) << std::endl;
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
        //cout << st.toString() << " : " << succ.size() << endl;
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
    initials, mp, alph, getAPPattern());
}


/*
 * Get deterministic part in Schewe construction
 * @return Deterministic part (NFA part)
 */
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


/*
 * Is the self-loop accepting?
 * @param state Macrostate with selfloop
 * @param alp Alphabet
 * @return Is sl accepting
 */
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
      all = false;
      set<int> sng = {st};
      stack = std::stack<set<int>>();
      comst.clear();
      stack.push(succSet(sng, a));
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


/*
 * Get macrostates with accepting self-loop
 * @param nfaSchewe Deterministic part
 * @return Set of accepting self-loops
 */
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


/*
 * Get macrostates with self-loop over single symbol not accepting
 * @param nfaSchewe Deterministic part
 * @return Set of not accepting self-loops with symbols
 */
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


/*
 * Get rank bound for each macrostate
 * @param nfaSchewe Deterministic part
 * @param slignore Self-loops to be ignored
 * @param maxReachSize Maximum reachable macrostate
 * @param minReachSize Minimum reachable macrostate
 * @return Rank bound for each macrostate
 */
map<DFAState, int> BuchiAutomatonSpec::getRankBound(BuchiAutomaton<StateSch, int>& nfaSchewe, set<StateSch>& slignore, map<DFAState, int>& maxReachSize, map<int, int>& minReachSize)
{
  set<int> nofin;
  set<int> fin = this->getFinals();
  std::set_difference(this->getStates().begin(), this->getStates().end(), fin.begin(),
    fin.end(), std::inserter(nofin, nofin.begin()));
  vector<int> states(nofin.begin(), nofin.end());
  map<StateSch, int> rnkmap;
  map<set<int>, int> classesMap;
  int classes;

  bool sd = false;
  if(this->opt.semidetOpt && this->isSemiDeterministic())
    sd = true;

  for(const StateSch& s : nfaSchewe.getStates())
  {
    rnkmap[s] = 0;
  }

  for(const StateSch& s : nfaSchewe.getStates())
  {
    for(vector<int>& sub : Aux::getAllSubsets(vector<int>(s.S.begin(), s.S.end())))
    {
      set<int> st(sub.begin(), sub.end());

      if(classesMap.find(st) == classesMap.end())
      {
        this->computeRankSim(st);
        classes = Aux::countEqClasses(this->getStates().size(), st, this->getOddRankSim());
      }
      else
      {
        classes = classesMap[st];
      }
      rnkmap[s] = std::max(rnkmap[s], classes);
      if(sd)
      {
        rnkmap[s] = std::min(rnkmap[s], 3);
      }
    }
  }


  // cout << " : " << states.size() << endl;
  // for(vector<int>& sub : Aux::getAllSubsets(states))
  // {
  //   set<int> st(sub.begin(), sub.end());
  //   this->computeRankSim(st);
  //   int classes = Aux::countEqClasses(this->getStates().size(), st, this->getOddRankSim());
  //   //cout << StateSch::printSet(st) << " : " << classes << endl;
  //   for(const StateSch& s : nfaSchewe.getStates())
  //   {
  //     if(std::includes(s.S.begin(), s.S.end(), st.begin(), st.end()))
  //     {
  //       rnkmap[s] = std::max(rnkmap[s], classes);
  //     }
  //   }
  // }
  // cout << " end "  << endl;


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

    for(int st : act.S)
    {
      if(fin.find(st) != fin.end() && minReachSize[st] == maxReach)
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


/*
 * Get maximum reachable macrostate for each macrostate
 * @param nfaSchewe Deterministic part
 * @param slignore Self-loops to be ignored
 * @return Maximum reachable macrostate for each macrostate
 */
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


/*
 * Get maximum reachable macrostate for each state of the original automaton
 * @return Maximum reachable macrostate for each state
 */
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


/*
 * Get maximum reachable macrostate for each state
 * @return Maximum reachable macrostate for each state
 */
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


/*
 * Get all tight ranks (with RankRestr)
 * @param out Out parameter to store tight ranks
 * @param max Vector of maximal ranks (indexed by states)
 * @param states Set of states in a macrostate (the S-set)
 * @param macrostate Current macrostate
 * @param reachCons SuccRank restriction
 * @param reachMax Maximum reachable macrostate
 * @param dirRel Direct simulation
 * @param oddRel Rank simulation
 */
void BuchiAutomatonSpec::getSchRanksTightOpt(vector<RankFunc>& out, vector<int>& max,
    set<int>& states, StateSch& macrostate,
    map<int, int> reachCons, int reachMax, BackRel& dirRel, BackRel& oddRel)
{
  RankConstr constr;
  map<int, int> sngmap;

  set<int> fin = getFinals();
  for(int st : states)
  {
    vector<std::pair<int, int> > singleConst;
    if(fin.find(st) == fin.end())
    {
      for(int i = 0; i < max[st]; i+= 1)
        singleConst.push_back(std::make_pair(st, i));
    }
    else
    {
      for(int i = 0; i < max[st]; i+= 2)
        singleConst.push_back(std::make_pair(st, i));
    }

    sngmap[st] = max[st];
    singleConst.push_back(std::make_pair(st, max[st]));
    constr.push_back(singleConst);
  }

  out = RankFunc::tightSuccFromRankConstrPure(constr, dirRel, oddRel, macrostate.f.getMaxRank(),
    reachCons, reachMax, true);
}


/*
 * Get all Schewe successros (with RankRestr)
 * @param state Schewe state
 * @param symbol Symbol
 * @param reachCons SuccRank restriction
 * @param maxReach Maximum reachable macrostate
 * @param dirRel Direct simulation
 * @param oddRel Rank simulation
 * @return Set of all successors
 */
vector<StateSch> BuchiAutomatonSpec::succSetSchTightOpt(StateSch& state, int symbol,
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

    if(state.f.find(st)->second == 0 && reachCons[st] > 0)
    {
      return ret;
    }
    if(dst.size() == 0 && state.f.find(st)->second != 0)
    {
      return ret;
    }
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
  vector<RankFunc> ranks;
  vector<RankFunc> tmp;
  set<int> inverseRank;

  if(!getRankSuccCache(tmp, state, symbol))
  {
    getSchRanksTightOpt(tmp, maxRank, sprime, state,
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


/*
 * Get starting states of the tight part (with RankRestr)
 * @param state DFA macrostate
 * @param rankBound Maximum rank
 * @param macrostate Current macrostate
 * @param reachCons SuccRank restriction
 * @param maxReach Maximum reachable macrostate
 * @param dirRel Direct simulation
 * @param oddRel Rank simulation
 * @return Set of first states in the tight part
 */
vector<StateSch> BuchiAutomatonSpec::succSetSchStartOpt(set<int>& state, int rankBound,
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
  for(const RankFunc& item : RankFunc::tightFromRankConstrPure(constr, dirRel, oddRel, reachCons, reachMaxAct, true))
  {
    ret.push_back({sprime, set<int>(), item, 0, true});
  }
  return ret;
}


/*
 * Schewe complementation proceudre (with RankRestr)
 * @return Complemented automaton
 */
BuchiAutomaton<StateSch, int> BuchiAutomatonSpec::complementSchOpt()
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
    //std::cout << StateSch::printSet(pr.first) << std::endl;
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
        succ = succSetSchTightOpt(st, sym, reachCons, maxReach, dirRel, oddRel);
      }
      else
      {
        succ = succSetSchStartOpt(st.S, rankBound[st.S], reachCons, maxReach, dirRel, oddRel);
        //cout << st.toString() << " : " << succ.size() << endl;
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
    initials, mp, alph, getAPPattern());
}
