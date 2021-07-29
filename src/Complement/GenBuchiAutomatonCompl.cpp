#include "GenBuchiAutomatonCompl.h"

BuchiAutomaton<StateSchGBA, int> GeneralizedBuchiAutomatonCompl::getOneTightPart(std::set<int> originalFinals, unsigned number, std::stack<StateSchGBA> stack, std::map<int, int> reachCons, std::map<DFAState, int> maxReach, BackRel dirRel, BackRel oddRel, bool eta4, std::set<StateSchGBA> comst, map<std::pair<StateSchGBA, int>, set<StateSchGBA> > mp, map<std::pair<StateSchGBA, int>, set<StateSchGBA>> prev, BuchiAutomatonSpec &buchi, std::set<StateSchGBA> initials){
    
    bool cnt = true;
    unsigned transitionsToTight = 0;
    vector<StateSch> succBA;
    vector<StateSchGBA> succGBA;
    set<StateSchGBA> finals;

    bool delay = false; //TODO
  
    // tight part construction
    //start = std::chrono::high_resolution_clock::now();
    while(stack.size() > 0)
    {
      StateSchGBA st = stack.top();
      stack.pop();
      if(isSchGBAFinal(st))
        finals.insert(st);
      cnt = true;
  
      for(int sym : alph)
      {
        auto pr = std::make_pair(st, sym);
        set<StateSchGBA> dst;
        if(st.tight)
        {
          StateSch tmpState = {.S =  st.S, .O = st.O, .f = st.f, .i = st.i, .tight = st.tight};
          succBA = buchi.succSetSchTightReduced(tmpState, sym, reachCons, maxReach, dirRel, oddRel, eta4, this->getFinals()[number]);
          for (auto state : succBA){
            StateSchGBA tmpState = {.S = state.S, .O = state.O, .f = state.f, .i = state.i, .j = number, .tight = state.tight};
            succGBA.push_back(tmpState);
          }
        }
        else
        {
          succBA = buchi.succSetSchStartReduced(st.S, rankBound[st.S].bound, reachCons, maxReach, dirRel, oddRel, this->getFinals()[number]);
          for (auto state : succBA){
            StateSchGBA tmpState = {.S = state.S, .O = state.O, .f = state.f, .i = state.i, .j = number, .tight = state.tight};
            succGBA.push_back(tmpState);
          }
          //cout << st.toString() << " : " << succ.size() << endl;
          cnt = false;
          //auto tmp = succSetSchStart(st.S, rankBound[st.S], reachCons, maxReach, dirRel, oddRel);
          //std::cerr << "Size: " << tmp.size() << std::endl;
          //std::cerr << "Rank bound: " << rankBound[st.S].bound << std::endl;
          //std::cerr << "Tight size: " << succ.size() << std::endl;
        }
        for (const StateSchGBA& s : succGBA)
        {
          dst.insert(s);
          if(comst.find(s) == comst.end())
          {
            stack.push(s);
            comst.insert(s);
          }
        }
  
        // TODO self-loops
        /*auto it = slTrans.find({st.S, sym});
        if(it != slTrans.end())
        {
          dst.insert(it->second);
        }*/
        if(!st.tight)
        {
          if(!cnt)
          {
              for(const auto& a : this->getAlphabet())
              {
                for(const auto& d : prev[{st, a}]) {
                  if ((not delay) /*or tightStartDelay[d].find(a) != tightStartDelay[d].end()*/){
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
            /*else {
              if (tightStartDelay[st].find(sym) != tightStartDelay[st].end()){
                  mp[pr].insert(dst.begin(), dst.end());
              }
            }*/
          }
        }
        else{
          mp[pr] = dst;
        }
        if(!cnt) break;
      }
      //std::cout << comst.size() << " : " << stack.size() << std::endl;
    }
    return BuchiAutomaton<StateSchGBA, int>(comst, finals, initials, mp, alph, getAPPattern());
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

    // original buchi automaton without accepting states
    BuchiAutomaton<int, int> *ba = new BuchiAutomaton<int, int>(this->states, set<int>(), this->initials, this->trans, this->alph);
    BuchiAutomatonSpec buchi(ba);
  
    // NFA part of the Schewe construction
    auto start = std::chrono::high_resolution_clock::now();
    BuchiAutomaton<StateSchGBA, int> comp = this->complementSchNFA(this->getInitials());
    auto end = std::chrono::high_resolution_clock::now();
    stats->waitingPart = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    std::cerr << "Waiting part states: " << comp.getStates().size() << std::endl;
  
    // rank bound
    start = std::chrono::high_resolution_clock::now();
  
    map<std::pair<StateSchGBA, int>, set<StateSchGBA>> prev = comp.getReverseTransitions();
    //std::cout << comp.toGraphwiz() << std::endl;

    set<StateSchGBA> nfaStates = comp.getStates();
    comst.insert(nfaStates.begin(), nfaStates.end());

    mp.insert(comp.getTransitions().begin(), comp.getTransitions().end());
    finals = set<StateSchGBA>(comp.getFinals());
  
    //TODO accepting self-loops in gba??? 
    set<StateSchGBA> slIgnore;
    /*set<StateSch> slIgnore = this->nfaSlAccept(comp);
    set<pair<DFAState,int>> slNonEmpty = this->nfaSingleSlNoAccept(comp);*/
    set<StateSchGBA> ignoreAll;
    /*for(const auto& t : slNonEmpty)
      ignoreAll.insert({t.first, set<int>(), RankFunc(), 0, false});
    ignoreAll.insert(slIgnore.begin(), slIgnore.end());*/
  
    //TODO reachability restrictions for gbas
    // Compute reachability restrictions
    //map<int, int> reachCons;
    //map<DFAState, int> maxReach;
    map<int, int> reachCons = buchi.getMinReachSize();
    map<DFAState, int> maxReach = buchi.getMaxReachSize(comp, slIgnore);
  
    //TODO non-accepting self-loops
    /*
    int newState = this->getStates().size(); //Assumes numbered states: from 0, no gaps
    map<pair<DFAState,int>, StateSchGBA> slTrans;
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
    */
  
    //TODO compute rank upper bound for gbas
    // Compute rank upper bound on the macrostates
    auto invComp = comp.reverseBA(); //inverse automaton
    this->rankBound = buchi.getRankBound(invComp, ignoreAll, maxReach, reachCons);
    end = std::chrono::high_resolution_clock::now();
    stats->rankBound = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
  
    //TODO gba elevator???
    // update rank upper bound of each macrostate based on elevator automaton structure
    /*if (elevatorRank){
      start = std::chrono::high_resolution_clock::now();
      this->elevatorRank(comp);
      end = std::chrono::high_resolution_clock::now();;
      stats->elevatorRank = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
    }*/
  
    //TODO delay for GBAs
    /*start = std::chrono::high_resolution_clock::now();
    map<StateSchGBA, DelayLabel> delayMp;
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
    }*/

    // Compute states necessary to generate in the tight part
    set<StateSchGBA> tightStart;
    map<StateSchGBA, set<int>> tightStartDelay;
    if (delay){
      //TODO delay for gbas
      /*BuchiAutomatonDelay<int> delayB(comp);
      tightStartDelay = delayB.getCycleClosingStates(ignoreAll, delayMp, w, version, stats);*/
    }
    else {
      tightStart = comp.getCycleClosingStates(ignoreAll);
    }
    end = std::chrono::high_resolution_clock::now();
    stats->cycleClosingStates = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
  
    std::set<StateSchGBA> tmpSet;
    if (delay){
      for(auto item : tightStartDelay)
        tmpSet.insert(item.first);
    }
    for(const StateSchGBA& tmp : (delay ? tmpSet : tightStart))
    {
      if(tmp.S.size() > 0)
      {
        stack.push(tmp);
      }
    }
  
    StateSchGBA init = {getInitials(), set<int>(), RankFunc(), 0, 0, false};
    initials.insert(init);
  
    //TODO simulations for gbas
    // simulations
    start = std::chrono::high_resolution_clock::now();
    set<int> cl;
    buchi.computeRankSim(cl);
  
    BackRel dirRel = buchi.createBackRel(buchi.getDirectSim());
    BackRel oddRel = buchi.createBackRel(buchi.getOddRankSim());
    end = std::chrono::high_resolution_clock::now();
    stats->simulations = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count();
  
    bool eta4 = false;

    // TIGHT PARTS CONSTRUCTION
    BuchiAutomaton<StateSchGBA, int> final(comst, finals, initials, mp, alph, getAPPattern()); // waiting part
    unsigned j = 1;
    BuchiAutomaton<StateSchGBA, int> oneTightPart;
    for (auto it = originalFinals.begin(); it != originalFinals.end(); it++){
      oneTightPart = getOneTightPart(it->second, j, stack, reachCons, maxReach, dirRel, oddRel, eta4, comst, mp, prev, buchi, initials);
      // TODO merge with the current automaton
      std::cerr << "Tight part size: " << oneTightPart.getStates().size() << std::endl;
      // merge states
      for (auto state : oneTightPart.getStates()){
        if (final.getStates().find(state) == final.getStates().end()){
          final.addStates(state);
        }
      }
      // merge finals
      for (auto fin : oneTightPart.getFinals()){
        if (final.getFinals().find(fin) == final.getFinals().end())
          final.addFinals(fin);
      }
      // merge transitions
      for (auto trans : oneTightPart.getTransitions()){
        if (final.getTransitions().find(trans.first) == final.getTransitions().end())
          final.addNewTransition(trans.first, trans.second);
        else if (trans.second != final.getTransitions().find(trans.first)->second)
          final.addNewStatesToTransition(trans.first, trans.second);
      }
    }
  
    //std::cerr << "Transitions to tight: " << transitionsToTight << std::endl;
  
    end = std::chrono::high_resolution_clock::now();
    stats->tightPart = std::chrono::duration_cast<std::chrono::milliseconds>(end-start).count(); 
  
    //std::cerr << "States: " << final.getStates().size() << std::endl; 
    return final;
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
