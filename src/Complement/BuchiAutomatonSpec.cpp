
#include "BuchiAutomatonSpec.h"
#include <chrono>

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

  if(this->rankBound[state.S].bound > state.f.getMaxRank() || this->rankBound[sprime].bound > state.f.getMaxRank())
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

  // Compute rank upper bound on the macrostates
  this->rankBound = this->getRankBound(comp, slIgnore, maxReach, reachCons);
  // map<StateSch, DelayLabel> delayMp;
  // for(const auto& st : comp.getStates())
  // {
  //   delayMp[st] = { .macrostateSize = (unsigned)st.S.size(), .maxRank = (unsigned)this->rankBound[st.S].bound, .nonAccStates = 0 };
  // }
  // Compute states necessary to generate in the tight part
  //set<StateSch> tightStart = comp.getCycleClosingStates(slIgnore, delayMp);
  set<StateSch> tightStart = comp.getCycleClosingStates(slIgnore);
  for(const StateSch& tmp : tightStart)
  {
    if(tmp.S.size() > 0)
    {
      stack.push(tmp);
    }
  }


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
        succ = succSetSchStart(st.S, rankBound[st.S].bound, reachCons, maxReach, dirRel, oddRel);
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

  if(this->opt.lowrankopt)
  {
    auto stateBound = this->rankBound[states].stateBound;
    for(const auto & s : states)
    {
      sngmap[s] = std::min(sngmap[s], stateBound[s]);
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
    map<int, int> reachCons, map<DFAState, int> maxReach, BackRel& dirRel, BackRel& oddRel, set<int> finals)
{
  vector<StateSch> ret;
  set<int> sprime;
  set<int> oprime;
  int iprime;
  vector<int> maxRank(getStates().size(), state.f.getMaxRank());
  map<int, set<int> > succ;
  map<int, bool > pre;
  VecTrans<int, int> accTrans = this->getFinTrans();

  this->setFinals(finals);
  auto fin = getFinals();

  if(this->opt.complete)
  {
    for(const auto& p : state.f)
    {
      if(p.second == 0)
        return vector<StateSch>();
    }
  }

  for(int st : state.S)
  {
    set<int> dst = getTransitions()[std::make_pair(st, symbol)];
    for(int d : dst)
    {
      Transition<int, int> tr = { .from = st, .to = d, .symbol = symbol};
      if(std::find(accTrans.begin(), accTrans.end(), tr) != accTrans.end() )
        maxRank[d] = std::min(maxRank[d], BuchiAutomatonSpec::evenceil(state.f[st]));
      else
        maxRank[d] = std::min(maxRank[d], state.f[st]);
    }
    sprime.insert(dst.begin(), dst.end());
    if(fin.find(st) == fin.end())
      succ[st] = dst;

  }

  // if(this->opt.lowrankopt)
  // {
  //   RankFunc ubound(this->rankBound[state.S].stateBound, false);
  //   if(!state.f.isAllLeq(ubound))
  //     return ret;
  // }

  if(this->rankBound[state.S].bound < state.f.getMaxRank() || this->rankBound[sprime].bound < state.f.getMaxRank())
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

    if (this->opt.eta4){
      SCC intersection;
      std::set_intersection(st.S.begin(), st.S.end(), fin.begin(), fin.end(), std::inserter(intersection, intersection.begin()));
      if (intersection.size() == 0)
        continue;
    }

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
            rnkMap[o]--; // = std::min(rnkMap[o] - 1, stateBound[o]);
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
    BackRel& oddRel, set<int> finals)
{
  vector<StateSch> ret;
  set<int> sprime = state;
  set<int> schfinal;
  this->setFinals(finals);
  set<int> fin = getFinals();
  std::set_difference(sprime.begin(),sprime.end(),fin.begin(),
    fin.end(), std::inserter(schfinal, schfinal.begin()));
  int m = std::min((int)(2*schfinal.size() - 1), rankBound);
  vector<int> maxRank(getStates().size(), m);

  for(int st : sprime)
  {
    if(fin.find(st) != fin.end() && maxRank[st] % 2 != 0)
      maxRank[st] -= 1;
  }

  vector<RankFunc> maxRanks;
  vector<RankFunc>* maxPtr;
  vector<RankFunc> maxRanks1;
  set<RankFunc> maxRankLeq;
  vector<RankFunc> maxRanks2;
  vector<RankFunc> maxRanks3;

  RankFunc ubound(this->rankBound[sprime].stateBound, false);
  if(this->opt.lowrankopt)
  {
    maxRanks = RankFunc::getRORanksSD(m, state, fin, this->opt.cutPoint, this->rankBound[sprime].stateBound);
    set<RankFunc> tmpSet1;
    for(const RankFunc& f : maxRanks)
    {
      if(!f.isAllLeq(ubound))
      {
        continue;
      }
      tmpSet1.insert(f);
    }
    maxRanks1 = vector<RankFunc>(tmpSet1.begin(), tmpSet1.end());
    maxPtr = &maxRanks1;
  }
  else if(state.size() >= this->opt.ROMinState && m >= this->opt.ROMinRank)
  {
    maxRanks = RankFunc::getRORanks(rankBound, state, fin, this->opt.cutPoint, this->rankBound[sprime].stateBound);
    maxPtr = &maxRanks;
  }
  else
  {
    int reachMaxAct = maxReach[sprime];
    RankConstr constr = rankConstr(maxRank, sprime);
    auto tmp = RankFunc::tightFromRankConstr(constr, dirRel, oddRel, reachCons, reachMaxAct, this->opt.cutPoint);

    maxRanks = RankFunc::getRORanks(rankBound, state, fin, this->opt.cutPoint, this->rankBound[sprime].stateBound);
    set<RankFunc> tmpSet2(tmp.begin(), tmp.end());
    set<RankFunc> tmpSet1;//(tmp.begin(), tmp.end());
    for(const RankFunc& f : maxRanks)
    {
      if(!f.isAllLeq(ubound))
      {
        continue;
      }
      tmpSet1.insert(f);
      //maxRankLeq.insert(f);
    }
    maxRanks1 = vector<RankFunc>(tmpSet1.begin(), tmpSet1.end());
    //maxRanks1 = getFuncAntichain(maxRankLeq, true);
    maxRanks2 = getFuncAntichain(tmpSet2);

    // bool cnt = false;
    // for(const RankFunc& f : maxRanks3)
    // {
    //   cnt = true;
    //   for(const auto & s: f.getOddStates())
    //   {
    //     if(f.getRanks()[s] > ubound[s])
    //     {
    //       cnt = false;
    //       break;
    //     }
    //   }
    //   if(!cnt)
    //   {
    //     continue;
    //   }
    //   maxRanks2.push_back(f);
    // }

    maxPtr = maxRanks1.size() > maxRanks2.size() ? &maxRanks2 : &maxRanks1;


    // bool cnt = true;
    // for(auto& r : tmpSet)
    // {
    //   cnt = true;
    //   auto it = tmpSet.upper_bound(r);
    //   while(it != tmpSet.end())
    //   {
    //     if(r != (*it) && r.getMaxRank() == it->getMaxRank() && r.isAllLeq(*it))
    //     {
    //       cnt = false;
    //       break;
    //     }
    //     it = std::next(it, 1);
    //   }
    //   if(cnt) maxRanks.push_back(r);
    // }
  }

  for(const RankFunc& item : *maxPtr)
  {
    ret.push_back({sprime, set<int>(), item, 0, true});
  }
  return ret;
}


vector<RankFunc> BuchiAutomatonSpec::getFuncAntichain(set<RankFunc>& tmp, bool oddCheck) const
{
  vector<RankFunc> maxRanks;
  bool cnt = true;
  for(auto& r : tmp)
  {
    cnt = true;
    auto it = tmp.upper_bound(r);
    while(it != tmp.end())
    {
      if(r != (*it) && r.getMaxRank() == it->getMaxRank() && r.isAllLeq(*it))
      {
        if(oddCheck)
        {
          if(r.getOddStates() == it->getOddStates())
          {
            cnt = false;
            break;
          }
        }
        else
        {
          cnt = false;
          break;
        }
      }
      it = std::next(it, 1);
    }
    if(cnt) maxRanks.push_back(r);
  }
  return maxRanks;
}


/*
 * Copute rank bounds and other restrictions, such as reachability constraints,
 * ignored self-loops, maximum reachability constraints, starting macrostates to
 * tight.
 *
 * @param comp: Waiting part of the automaton
 * @param originalFinals: Final states of the original automaton
 * @param stats: Statistical information
 */
void BuchiAutomatonSpec::computeRankBound(BuchiAutomaton<StateSch, int>& comp, Stat *stats)
{
  set<int> originalFinals = this->getFinals();
  // NFA part of the Schewe construction
  auto start = std::chrono::high_resolution_clock::now();
  //BuchiAutomaton<StateSch, int> comp = this->complementSchNFA(this->getInitials());
  auto end = std::chrono::high_resolution_clock::now();
  stats->waitingPart = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

  // rank bound
  start = std::chrono::high_resolution_clock::now();

  map<std::pair<StateSch, int>, set<StateSch>> prev = comp.getReverseTransitions();
  // cout << StateSch::printSet(originalFinals) << endl;
  if(this->opt.debug)
  {
    cout << "Waiting part: " << endl;
    cout << comp.toGraphwiz() << endl << endl;
  }

  set<StateSch> slIgnore;
  this->slNonEmpty = set<pair<DFAState,int>>();
  set<StateSch> ignoreAll;

  if(this->opt.sl)
  {
    slIgnore = this->nfaSlAccept(comp);
    this->slNonEmpty = this->nfaSingleSlNoAccept(comp);

    for(const auto& t : this->slNonEmpty)
      ignoreAll.insert({t.first, set<int>(), RankFunc(), 0, false});
    ignoreAll.insert(slIgnore.begin(), slIgnore.end());
  }

  this->reachCons = map<int, int>();
  // this->maxReach = map<DFAState, int>();
  if(this->opt.reach)
  {
    // Compute reachability restrictions
    this->reachCons = this->getMinReachSize();
    this->maxReach = this->getMaxReachSize(comp, slIgnore);
  }
  else
  {
    for(const auto& t : comp.getStates())
      this->maxReach[t.S] = this->getStates().size();
    for(const auto& t : this->getStates())
      this->reachCons[t] = 0;
  }

  // Compute rank upper bound on the macrostates
  auto invComp = comp.reverseBA(); //inverse automaton
  this->rankBound = this->getRankBound(invComp, ignoreAll, this->maxReach, this->reachCons);

  if(this->opt.debug)
  {
    cout << "Rank bound: " << endl;
    for(auto & s: this->rankBound)
    {
      RankFunc fnc(s.second.stateBound, false);
      cout << fnc.toString() << " : " << s.second.bound << endl;
    }
    cout << endl;
  }

  end = std::chrono::high_resolution_clock::now();
  stats->rankBound = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

  stats->ranks = this->rankBound;

  start = std::chrono::high_resolution_clock::now();
  map<StateSch, DelayLabel> delayMp;
  for(const auto& st : comp.getStates())
  {
    // nonaccepting states
    std::set<int> result;
    std::set_difference(st.S.begin(), st.S.end(), originalFinals.begin(), originalFinals.end(), std::inserter(result, result.end()));

    delayMp[st] = {
      .macrostateSize = (unsigned)st.S.size(),
      .maxRank = (unsigned)this->rankBound[st.S].bound,
      .nonAccStates = (unsigned)result.size()
    };
  }

  // Compute states necessary to generate in the tight part
  set<StateSch> tightStart;
  this->tightStartDelay = map<StateSch, set<int>>();
  if (this->opt.delay){
    BuchiAutomatonDelay<int> delayB(comp);
    this->tightStartDelay = delayB.getCycleClosingStates(ignoreAll, delayMp, this->opt.delayW, this->opt.delayVersion, stats);
  }
  else {
    if(this->isDeterministic() && this->getAlphabet().size() >= 256)
    {
      tightStart = comp.getStates();
    }
    else
    {
      tightStart = comp.getCycleClosingStates(ignoreAll);
    }

  }
  end = std::chrono::high_resolution_clock::now();
  stats->cycleClosingStates = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

  std::set<StateSch> tmpSet;
  if (this->opt.delay){
    for(auto item : this->tightStartDelay)
      tmpSet.insert(item.first);
  }

  this->tightStartStates = this->opt.delay ? tmpSet : tightStart;
}


/*
 * Optimized Schewe complementation procedure
 * @return Complemented automaton
 */

BuchiAutomaton<StateSch, int> BuchiAutomatonSpec::complementSchReduced(Stat *stats, bool updateBounds)
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

  BuchiAutomaton<StateSch, int> comp = this->complementSchNFA(this->getInitials());
  map<std::pair<StateSch, int>, set<StateSch>> prev = comp.getReverseTransitions();

  /*if (comp.getStates().size() == 1){
    comp.setAPPattern(this->getAPPattern());
    return comp;
  }*/

  set<StateSch> nfaStates = comp.getStates();
  comst.insert(nfaStates.begin(), nfaStates.end());
  mp.insert(comp.getTransitions().begin(), comp.getTransitions().end());
  finals = set<StateSch>(comp.getFinals());

  if(updateBounds) this->computeRankBound(comp, stats);

  int newState = this->getStates().size(); //Assumes numbered states: from 0, no gaps
  map<pair<DFAState,int>, StateSch> slTrans;
  for(const auto& pr : this->slNonEmpty)
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

  for(const StateSch& tmp : this->tightStartStates)
  {
    if(tmp.S.size() > 0)
    {
      stack.push(tmp);
    }
  }

  StateSch init = {getInitials(), set<int>(), RankFunc(), 0, false};
  initials.insert(init);

  // simulations
  auto start = std::chrono::high_resolution_clock::now();
  set<int> cl;
  if(this->opt.sim)
  {
    this->computeRankSim(cl);
  }

  BackRel dirRel = createBackRel(this->getDirectSim());
  BackRel oddRel = createBackRel(this->getOddRankSim());
  auto end = std::chrono::high_resolution_clock::now();
  stats->simulations = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

  bool cnt = true;
  unsigned transitionsToTight = 0;

  map<DFAState, set<int> > symsPred;
  auto tr = comp.getTransitions();
  for(const auto& m : comp.getStates())
  {
    symsPred[m.S] = set<int>();
    for(int sym : this->getAlphabet())
    {

      if(tr[{m, sym}].size() > 2)
        symsPred[m.S].insert(sym);
      if(tr[{m, sym}].size() == 1 && tr[{m, sym}].begin()->S.size() > 0)
        symsPred[m.S].insert(sym);

    }
  }

  // tight part construction
  start = std::chrono::high_resolution_clock::now();
  while(stack.size() > 0)
  {
    StateSch st = stack.top();
    stack.pop();
    if(isSchFinal(st))
      finals.insert(st);
    cnt = true;

    //cout << st.toString() << endl;

    for(int sym : symsPred[st.S])
    {
      auto pr = std::make_pair(st, sym);
      set<StateSch> dst;
      if(st.tight)
      {
        succ = succSetSchTightReduced(st, sym, this->reachCons, this->maxReach, dirRel, oddRel, this->getFinals());
      }
      else
      {
        succ = succSetSchStartReduced(st.S, rankBound[st.S].bound, this->reachCons, this->maxReach, dirRel, oddRel, this->getFinals());
        // for(const auto& t : succ)
        // {
        //   cout << t.f.toString() << endl;
        // }
        // cout << endl;
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
      if(!st.tight && it != slTrans.end())
      {
        dst.insert(it->second);
      }
      if(!st.tight)
      {
        if(!cnt)
        {
            for(const auto& a : this->getAlphabet())
            {
              for(const auto& d : prev[{st, a}]) {
                if ((!this->opt.delay) or this->tightStartDelay[d].find(a) != this->tightStartDelay[d].end()){
                  mp[{d,a}].insert(dst.begin(), dst.end());
                  transitionsToTight += dst.size();
                }
              }
            }
        }
        else
        {
          if (!this->opt.delay)
            mp[pr].insert(dst.begin(), dst.end());
          else {
            if (this->tightStartDelay[st].find(sym) != this->tightStartDelay[st].end()){
                mp[pr].insert(dst.begin(), dst.end());
            }
          }
        }
      }
      else{
        mp[pr] = dst;
      }
      if(!cnt) break;
    }
  }

  end = std::chrono::high_resolution_clock::now();
  stats->tightPart = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();

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
  set<pair<int,int>> rel;
  bool all = true;
  set<int> symAcc;
  set<int> fin = getFinals();
  VecTrans<int, int> accTrans = this->getFinTrans();

  std::stack<set<int>> stack;
  std::set<set<int>> comst;

  if(state.S.size() == 0)
    return false;

  for(int st : state.S)
  {
    if(fin.find(st) != fin.end())
      rel.insert({st,st});
  }
  for(const auto& tr : accTrans)
  {
    if(state.S.find(tr.from) != state.S.end() && state.S.find(tr.to) != state.S.end() && find(alp.begin(), alp.end(), tr.symbol) != alp.end())
      rel.insert({tr.to, tr.from});
  }

  if(rel.size() == 0)
    return false;
  for(const int& a : alp)
  {
    for(pair<int, int> st : rel)
    {
      all = false;
      set<int> sng = {st.first};
      stack = std::stack<set<int>>();
      comst.clear();
      stack.push(succSet(sng, a));
      comst.insert(succSet(sng, a));

      while(stack.size() > 0)
      {
        set<int> pst = stack.top();
        stack.pop();
        if(pst.find(st.second) != pst.end())
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
map<DFAState, RankBound> BuchiAutomatonSpec::getRankBound(BuchiAutomaton<StateSch, int>& nfaSchewe, set<StateSch>& slignore, map<DFAState, int>& maxReachSize, map<int, int>& minReachSize)
{
  set<int> nofin;
  set<int> fin = this->getFinals();
  std::set_difference(this->getStates().begin(), this->getStates().end(), fin.begin(),
    fin.end(), std::inserter(nofin, nofin.begin()));
  vector<int> states(nofin.begin(), nofin.end());
  map<StateSch, int> rnkmap;
  map<set<int>, int> classesMap;
  int classes;
  VecTrans<int, int> accTrans = this->getFinTrans();

  map<int, int> elevatorBound;

  if(this->opt.elevator.elevatorRank)
  {
    elevatorBound = this->elev.elevatorRank(this->opt.elevator.detBeginning);
    if(this->opt.debug)
    {
      cout << "Elevator ranks: " << endl;
      for (auto pr : elevatorBound){
        cout << pr.first << ": " << pr.second << endl;
      }
    }

  }
  else
  {
    for(int st : this->getStates())
      elevatorBound[st] = 2*this->getStates().size() - 1;
  }

  bool sd = false;
  if(this->opt.semidetOpt && this->isSemiDeterministic()){
    sd = true;
  }

  for(const StateSch& s : nfaSchewe.getStates())
  {
    rnkmap[s] = 2*s.S.size() - 1;
  }

  if(this->opt.sim)
  {
    for(const StateSch& s : nfaSchewe.getStates())
    {
      rnkmap[s] = 0;
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
  }

  map<DFAState, set<int> > symsPred;
  auto tr = nfaSchewe.getTransitions();
  for(const auto& m : nfaSchewe.getStates())
  {
    symsPred[m.S] = set<int>();
    for(int sym : this->getAlphabet())
    {
      if(tr[{m, sym}].size() > 0)
        symsPred[m.S].insert(sym);

    }
  }

  auto updPred = [this, &slignore, &symsPred, &accTrans] (LabelState<StateSch, RankBound>* dest, const std::vector<LabelState<StateSch, RankBound>*> sts) -> RankBound
  {
    int n = this->getStates().size();
    vector<int> mrank(n, 0);
    set<int> fin = this->getFinals();
    int m = 0;
    RankBound ret = dest->label;

    for(const LabelState<StateSch, RankBound>* tmp : sts)
    {
      if(tmp->state.S != dest->state.S || slignore.find(dest->state) == slignore.end())
        m = std::max(m, tmp->label.bound);

      // set<int> syms;
      // for(int sym : this->getAlphabet())
      // {
      //   if(this->succSet(tmp->state.S, sym) == dest->state.S)
      //     syms.insert(sym);
      // }
      for (const int& sym : symsPred[dest->state.S])
      {
        vector<int> tmpMrank(n, 2*n);
        for(const int& s : tmp->state.S)
        {
          set<int> dst = this->getTransitions()[std::make_pair(s, sym)];
          for(int d : dst)
          {
            auto it = tmp->label.stateBound.find(s);
            Transition<int, int> tr = { .from = s, .to = d, .symbol = sym};
            if(std::find(accTrans.begin(), accTrans.end(), tr) != accTrans.end() )
              tmpMrank[d] = std::min(tmpMrank[d], BuchiAutomatonSpec::evenceil(it->second));
            else
              tmpMrank[d] = std::min(tmpMrank[d], it->second);
          }
        }
        for(const int& i : dest->state.S)
        {
          mrank[i] = max(tmpMrank[i], mrank[i]);
        }
      }
    }

    if(this->opt.flowDirSim)
    {
      for(const auto & sim : this->getDirectSim())
      {
        if(dest->state.S.find(sim.first) != dest->state.S.end() && dest->state.S.find(sim.second) != dest->state.S.end())
        {
          mrank[sim.first] = std::min(mrank[sim.first], mrank[sim.second]);
        }
      }
    }

    int tmpm = 0;
    int tmpmax = 0;
    for(const int& i : dest->state.S)
    {
      tmpm = min(ret.stateBound[i], mrank[i]);
      tmpm = std::min(tmpm, ret.bound);
      if(fin.find(i) != fin.end() && tmpm % 2 != 0)
        tmpm--;
      ret.stateBound[i] = tmpm;
      tmpmax = std::max(tmpm, tmpmax);
    }
    ret.bound = std::min(dest->label.bound, m);

    if(tmpmax < ret.bound)
      ret.bound = tmpmax;
    if(ret.bound % 2 == 0)
      ret.bound = std::max(0, ret.bound - 1);

    return ret;
  };


  auto updMaxFnc = [&slignore] (LabelState<StateSch, RankBound>* a, const std::vector<LabelState<StateSch, RankBound>*> sts) -> RankBound
  {
    int m = 0;
    for(const LabelState<StateSch, RankBound>* tmp : sts)
    {
      if(tmp->state.S == a->state.S && slignore.find(a->state) != slignore.end())
        continue;
      m = std::max(m, tmp->label.bound);
    }
    return { .bound = std::min(a->label.bound, m), .stateBound = a->label.stateBound };
  };

  auto initMaxFnc = [this, &maxReachSize, &minReachSize, &rnkmap, &elevatorBound] (const StateSch& act) -> RankBound
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

    map<int, int> sbound;
    for(const int& s : act.S)
    {
      sbound[s] = 0;
    }

    for(int st : act.S)
    {
      if(fin.find(st) != fin.end() && minReachSize[st] == maxReach)
        return { .bound = 0, .stateBound = sbound };
    }
    if(maxCnt > 2)
      rank = std::min(rank, (int)ret.size() - maxCnt + 1);
    if(minReach != INF)
      rank = std::max(std::min(rank, maxReach - minReach + 1), 0);
    // if(this->containsRankSimEq(ret) && ret.size() > 1)
    //   rank = std::min(rank, std::max((int)ret.size() - 1, 0));
    rank = std::min(rank, rnkmap[act]);

    rank = std::max(2*rank-1, 0);
    int my = 0;
    for(const int& s : act.S)
    {
      if(fin.find(s) != fin.end() && rank % 2 != 0)
        sbound[s] = std::min(std::max(rank - 1, 0), elevatorBound[s]);
      else
        sbound[s] = std::min(rank, elevatorBound[s]);
      my = std::max(my, sbound[s]);
    }

    rank = std::min(rank, my);

    return { .bound = rank, .stateBound = sbound };
  };

  auto tmp = (this->opt.dataFlow == INNER) ? nfaSchewe.propagateGraphValues<RankBound>(updPred, initMaxFnc)
    : nfaSchewe.propagateGraphValues<RankBound>(updMaxFnc, initMaxFnc);
  map<DFAState, RankBound> ret;
  for(const auto& t : tmp){
    ret[t.first.S] = t.second;
  }

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
  auto updMaxFnc = [&slIgnore] (LabelState<StateSch, int>* a, const std::vector<LabelState<StateSch, int>*> sts) -> int
  {
    int m = 0;
    for(const LabelState<StateSch, int>* tmp : sts)
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

  auto tmp = nfaSchewe.propagateGraphValues<int>(updMaxFnc, initMaxFnc);
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
  //BuchiAutomaton<StateSch, int> comp;
  map<StateSch, int> mp;
  map<int, int> ret;

  auto updMaxFnc = [&slIgnore] (LabelState<StateSch, int>* a, const std::vector<LabelState<StateSch, int>*> sts) -> int
  {
    int m = 0;
    for(const LabelState<StateSch, int>* tmp : sts)
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
    BuchiAutomaton<StateSch, int> comp = this->complementSchNFA(ini);
    slIgnore = this->nfaSlAccept(comp);
    auto sls = comp.getSelfLoops();
    mp = comp.propagateGraphValues<int>(updMaxFnc, initMaxFnc);

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
  // BuchiAutomaton<StateSch, int> comp;
  map<StateSch, int> mp;
  map<int, int> ret;

  auto updMaxFnc = [&slIgnore] (LabelState<StateSch, int>* a, const std::vector<LabelState<StateSch, int>*> sts) -> int
  {
    int m = 0;
    for(const LabelState<StateSch, int>* tmp : sts)
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
    BuchiAutomaton<StateSch, int> comp = this->complementSchNFA(ini);
    slIgnore = this->nfaSlAccept(comp);
    auto sls = comp.getSelfLoops();
    mp = comp.propagateGraphValues<int>(updMaxFnc, initMaxFnc);

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
  /*
  TODO: add support for accepting transitions
  */
  assert(this->getFinTrans().size() == 0);

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

  if(this->rankBound[state.S].bound < state.f.getMaxRank() || this->rankBound[sprime].bound < state.f.getMaxRank())
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
  /*
  TODO: add support for accepting transitions
  */
  assert(this->getFinTrans().size() == 0);

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
BuchiAutomaton<StateSch, int> BuchiAutomatonSpec::complementSchOpt(bool delay, std::set<int> originalFinals, double w, Stat *stats)
{
  /*
  TODO: add support for accepting transitions
  */
  assert(this->getFinTrans().size() == 0);

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
  map<StateSch, DelayLabel> delayMp;
  for(const auto& st : comp.getStates())
  {
    // nonaccepting states
    std::set<int> result;
    std::set_difference(st.S.begin(), st.S.end(), originalFinals.begin(), originalFinals.end(), std::inserter(result, result.end()));

    delayMp[st] = {
      .macrostateSize = (unsigned)st.S.size(),
      .maxRank = (unsigned)this->rankBound[st.S].bound,
      .nonAccStates = (unsigned)result.size()
    };
  }
  // Compute states necessary to generate in the tight part
  set<StateSch> tightStart;
  map<StateSch, set<int>> tightStartDelay;
  if (delay){
    BuchiAutomatonDelay<int> delayB(comp);
    tightStartDelay = delayB.getCycleClosingStates(ignoreAll, delayMp, w, this->opt.delayVersion, stats);
  }
  else
    tightStart = comp.getCycleClosingStates(ignoreAll);
  std::set<StateSch> tmpSet;
  if (delay){
    for(auto item : tightStartDelay)
      tmpSet.insert(item.first);
  }
  std::set<StateSch> tmpStackSet;
  for(const StateSch& tmp : (delay ? tmpSet : tightStart))
  {
    if(tmp.S.size() > 0)
    {
      stack.push(tmp);
    }
    tmpStackSet.insert(tmp);
  }

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

    //cout << st.toString() << endl;

    for(int sym : alph)
    {
      auto pr = std::make_pair(st, sym);
      set<StateSch> dst;
      if(st.tight)
      {
        succ = succSetSchTightOpt(st, sym, reachCons, maxReach, dirRel, oddRel);
      }
      else // waiting part
      {
        succ = succSetSchStartOpt(st.S, rankBound[st.S].bound, reachCons, maxReach, dirRel, oddRel);
        //cout << st.toString() << " : " << succ.size() << endl;
        cnt = false;
      }
      for (const StateSch& s : succ)
      {
        dst.insert(s);
        if(comst.find(s) == comst.end())
        {
          if (not delay or std::find(tmpStackSet.begin(), tmpStackSet.end(), s) == tmpStackSet.end()){
            stack.push(s);
            comst.insert(s);
          }
        }
      }

      auto it = slTrans.find({st.S, sym});
      if(it != slTrans.end())
      {
        dst.insert(it->second);
      }
      if(!st.tight) // in the waiting part
      {
        if(!cnt)
        {
            for(const auto& a : this->getAlphabet())
            {
              for(const auto& d : prev[{st, a}]) {
                mp[{d,a}].insert(dst.begin(), dst.end());
              }
            }
        }
        else
        {
          if (not delay)
            mp[pr].insert(dst.begin(), dst.end());
          else {
            if (tightStartDelay[st].find(sym) != tightStartDelay[st].end()){
                mp[pr].insert(dst.begin(), dst.end());
            }
          }
        }
      }
      else {
        mp[pr] = dst;
      }
      if(!cnt) break;
    }
  }

  return BuchiAutomaton<StateSch, int>(comst, finals,
    initials, mp, alph, getAPPattern());
}

bool BuchiAutomatonSpec::meetsBackOff()
{
  for(const StateSch& st : this->tightStartStates)
  {
    for(const auto& p : this->opt.BOBound)
    {
      if(st.S.size() >= p.first && this->rankBound[st.S].bound >= p.second)
        return true;
    }
  }
  return false;
}

//template class LabelState<StateSch, RankBound>;
