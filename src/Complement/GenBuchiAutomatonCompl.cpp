#include "GenBuchiAutomatonCompl.h"

/*
 * Set of all successors.
 * @param states Set of states to get successors
 * @param symbol Symbol
 * @return Set of successors over symbol
 */
set<int> GeneralizedBuchiAutomatonCompl::succSet(set<int>& states, int symbol)
{
  set<int> ret;
  for(int st : states)
  {
    set<int> dst = getTransitions()[std::make_pair(st, symbol)];
    ret.insert(dst.begin(), dst.end());
  }
  return ret;
}

BuchiAutomaton<StateSchGBA, int> GeneralizedBuchiAutomatonCompl::complementSchNFA(std::set<int>& start){
    std::stack<StateSchGBA> stack;
    set<StateSchGBA> comst;
    set<StateSchGBA> initials;
    set<StateSchGBA> finals;
    //set<StateSch> succ;
    set<int> alph = getAlphabet();
    map<std::pair<StateSchGBA, int>, set<StateSchGBA> > mp;
    map<std::pair<StateSchGBA, int>, set<StateSchGBA> >::iterator it;
  
    StateSchGBA init = {start, set<int>(), RankFunc(), 0, 0, false};
    stack.push(init);
    comst.insert(init);
    initials.insert(init);
  
    while(stack.size() > 0)
    {
      StateSchGBA st = stack.top();
      stack.pop();
      if(isSchGBAFinal(st))
        finals.insert(st);
  
      for(int sym : alph)
      {
        auto pr = std::make_pair(st, sym);
        set<StateSchGBA> dst;
        if(!st.tight)
        {
          StateSchGBA nt = {succSet(st.S, sym), set<int>(), RankFunc(), 0, false};
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
  
    return BuchiAutomaton<StateSchGBA, int>(comst, finals, initials, mp, alph);
}

//TODO gba complement
BuchiAutomaton<StateSchGBA, int> GeneralizedBuchiAutomatonCompl::complementSchReduced(bool delay, std::map<int,std::set<int>> originalFinals, double w, delayVersion version, Stat *stats){
    std::stack<StateSchGBA> stack;
    set<StateSchGBA> comst;
    set<StateSchGBA> initials;
    set<StateSchGBA> finals;
    vector<StateSchGBA> succ;
    set<int> alph = getAlphabet();
    map<std::pair<StateSchGBA, int>, set<StateSchGBA> > mp;
    map<std::pair<StateSchGBA, int>, vector<StateSchGBA> > mpVect;
    map<std::pair<StateSchGBA, int>, set<StateSchGBA> >::iterator it;
  
    // NFA part of the Schewe construction
    auto start = std::chrono::high_resolution_clock::now();
    BuchiAutomaton<StateSchGBA, int> comp = this->complementSchNFA(this->getInitials());
    auto end = std::chrono::high_resolution_clock::now();
    stats->waitingPart = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
  
    /*
    // rank bound
    start = std::chrono::high_resolution_clock::now();
  
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
    std::cerr << "Size: " << comst.size() << std::endl;
  
  
    // Compute rank upper bound on the macrostates
    auto invComp = comp.reverseBA(); //inverse automaton
    this->rankBound = this->getRankBound(invComp, ignoreAll, maxReach, reachCons);
    end = std::chrono::high_resolution_clock::now();
    stats->rankBound = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
  
    // update rank upper bound of each macrostate based on elevator automaton structure
    if (elevatorRank){
      start = std::chrono::high_resolution_clock::now();
      this->elevatorRank(comp);
      end = std::chrono::high_resolution_clock::now();;
      stats->elevatorRank = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    }
  
    start = std::chrono::high_resolution_clock::now();
    map<StateSch, DelayLabel> delayMp;
    for(const auto& st : comp.getStates())
    {
      delayMp[st] = {
        .macrostateSize = (unsigned)st.S.size(),
        .maxRank = (unsigned)this->rankBound[st.S].bound
      };
  
      // nonaccepting states
      std::set<int> result;
      std::set_difference(st.S.begin(), st.S.end(), originalFinals.begin(), originalFinals.end(), std::inserter(result,   result.end()));
      delayMp[st].nonAccStates = result.size();
    }
    // Compute states necessary to generate in the tight part
    set<StateSch> tightStart;
    map<StateSch, set<int>> tightStartDelay;
    if (delay){
      BuchiAutomatonDelay<int> delayB(comp);
      tightStartDelay = delayB.getCycleClosingStates(ignoreAll, delayMp, w, version, stats);
    }
    else {
      tightStart = comp.getCycleClosingStates(ignoreAll);
    }
    end = std::chrono::high_resolution_clock::now();
    stats->cycleClosingStates = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
  
    std::set<StateSch> tmpSet;
    if (delay){
      for(auto item : tightStartDelay)
        tmpSet.insert(item.first);
    }
    //std::set<StateSch> tmpStackSet;
    for(const StateSch& tmp : (delay ? tmpSet : tightStart))
    {
      if(tmp.S.size() > 0)
      {
        stack.push(tmp);
      }
      //tmpStackSet.insert(tmp);
    }
  
    StateSch init = {getInitials(), set<int>(), RankFunc(), 0, false};
    initials.insert(init);
  
    // simulations
    start = std::chrono::high_resolution_clock::now();
    set<int> cl;
    this->computeRankSim(cl);
  
    BackRel dirRel = createBackRel(this->getDirectSim());
    BackRel oddRel = createBackRel(this->getOddRankSim());
    end = std::chrono::high_resolution_clock::now();
    stats->simulations = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
  
    bool cnt = true;
    unsigned transitionsToTight = 0;
  
    // tight part construction
    start = std::chrono::high_resolution_clock::now();
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
          succ = succSetSchTightReduced(st, sym, reachCons, maxReach, dirRel, oddRel, eta4);
        }
        else
        {
          succ = succSetSchStartReduced(st.S, rankBound[st.S].bound, reachCons, maxReach, dirRel, oddRel);
          //cout << st.toString() << " : " << succ.size() << endl;
          cnt = false;
          //auto tmp = succSetSchStart(st.S, rankBound[st.S], reachCons, maxReach, dirRel, oddRel);
          //std::cerr << "Size: " << tmp.size() << std::endl;
          std::cerr << "Rank bound: " << rankBound[st.S].bound << std::endl;
          std::cerr << "Tight size: " << succ.size() << std::endl;
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
                for(const auto& d : prev[{st, a}]) {
                  if ((not delay) or tightStartDelay[d].find(a) != tightStartDelay[d].end()){
                    mp[{d,a}].insert(dst.begin(), dst.end());
                    transitionsToTight += dst.size();
                  }
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
        else{
          mp[pr] = dst;
        }
        if(!cnt) break;
      }
      //std::cout << comst.size() << " : " << stack.size() << std::endl;
    }
  
    //std::cerr << "Transitions to tight: " << transitionsToTight << std::endl;
  
    end = std::chrono::high_resolution_clock::now();
    stats->tightPart = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
  
    return BuchiAutomaton<StateSch, int>(comst, finals,
    initials, mp, alph, getAPPattern());*/
}