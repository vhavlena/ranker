
#include "BuchiAutomaton.h"

template <typename State, typename Symbol>
std::set<Symbol> BuchiAutomaton<State, Symbol>::getAlph()
{
  SetSymbols sym;
  for (auto p : this->trans)
    sym.insert(p.first.second);
  return sym;
}


template <typename State, typename Symbol>
BuchiAutomaton<int, int> BuchiAutomaton<State, Symbol>::renameAut(int start)
{
  int stcnt = start;
  int symcnt = 0;
  std::map<State, int> mpstate;
  std::map<Symbol, int> mpsymbol;
  std::set<int> rstate;
  Delta<int, int> rtrans;
  std::set<int> rfin;
  std::set<int> rini;
  this->invRenameMap = std::vector<State>(this->states.size() + start);

  for(auto st : this->states)
  {
    auto it = mpstate.find(st);
    this->invRenameMap[stcnt] = st;
    if(it == mpstate.end())
    {
      mpstate[st] = stcnt++;
    }
  }

  rstate = Aux::mapSet(mpstate, this->states);
  rini = Aux::mapSet(mpstate, this->initials);
  rfin = Aux::mapSet(mpstate, this->finals);
  for(auto p : this->trans)
  {
    auto it = mpsymbol.find(p.first.second);
    int val;
    if(it == mpsymbol.end())
    {
      val = symcnt;
      mpsymbol[p.first.second] = symcnt++;
    }
    else
    {
      val = it->second;
    }
    std::set<int> to = Aux::mapSet(mpstate, p.second);
    rtrans.insert({std::make_pair(mpstate[p.first.first], val), to});
  }

  auto ret = BuchiAutomaton<int, int>(rstate, rfin, rini, rtrans);
  this->renameStateMap = mpstate;
  this->renameSymbolMap = mpsymbol;

  std::set<std::pair<int, int> > rdirSim, roddSim;
  for(auto item : this->directSim)
  {
    rdirSim.insert({mpstate[item.first], mpstate[item.second]});
  }
  for(auto item : this->oddRankSim)
  {
    roddSim.insert({mpstate[item.first], mpstate[item.second]});
  }
  ret.setDirectSim(rdirSim);
  ret.setOddRankSim(roddSim);
  ret.setAPPattern(this->apsPattern);
  return ret;
}


template <typename State, typename Symbol>
BuchiAutomaton<int, int> BuchiAutomaton<State, Symbol>::renameAutDict(map<Symbol, int>& mpsymbol, int start)
{
  int stcnt = start;
  //int symcnt = 0;
  std::map<State, int> mpstate;
  //std::map<Symbol, int> mpsymbol;
  std::set<int> rstate;
  Delta<int, int> rtrans;
  std::set<int> rfin;
  std::set<int> rini;
  this->invRenameMap = std::vector<State>(this->states.size() + start);

  for(auto st : this->states)
  {
    auto it = mpstate.find(st);
    this->invRenameMap[stcnt] = st;
    if(it == mpstate.end())
    {
      mpstate[st] = stcnt++;
    }
  }

  rstate = Aux::mapSet(mpstate, this->states);
  rini = Aux::mapSet(mpstate, this->initials);
  rfin = Aux::mapSet(mpstate, this->finals);
  for(auto p : this->trans)
  {
    //auto it = mpsymbol.find(p.first.second);
    int val = mpsymbol[p.first.second];
    // if(it == mpsymbol.end())
    // {
    //   val = symcnt;
    //   mpsymbol[p.first.second] = symcnt++;
    // }
    // else
    // {
    //   val = it->second;
    // }
    std::set<int> to = Aux::mapSet(mpstate, p.second);
    rtrans.insert({std::make_pair(mpstate[p.first.first], val), to});
  }

  auto ret = BuchiAutomaton<int, int>(rstate, rfin, rini, rtrans);
  this->renameStateMap = mpstate;
  this->renameSymbolMap = mpsymbol;

  std::set<std::pair<int, int> > rdirSim, roddSim;
  for(auto item : this->directSim)
  {
    rdirSim.insert({mpstate[item.first], mpstate[item.second]});
  }
  for(auto item : this->oddRankSim)
  {
    roddSim.insert({mpstate[item.first], mpstate[item.second]});
  }
  ret.setDirectSim(rdirSim);
  ret.setOddRankSim(roddSim);
  ret.setAPPattern(this->apsPattern);
  return ret;
}


template <typename State, typename Symbol>
std::string BuchiAutomaton<State, Symbol>::toStringWith(std::function<std::string(State)>& stateStr,
  std::function<std::string(Symbol)>& symStr)
{
  std::string str = "";
  for (auto p : this->initials)
    str += stateStr(p) + "\n";
  for (auto p : this->trans)
  {
    for(auto d : p.second)
      str += symStr(p.first.second) + "," + stateStr(p.first.first)
        + "->" + stateStr(d) + "\n";
  }
  for(auto p : this->finals)
    str += stateStr(p) + "\n";

  if(str.back() == '\n')
    str.pop_back();
  return str;
}


template <typename State, typename Symbol>
std::string BuchiAutomaton<State, Symbol>::toGffWith(std::function<std::string(State)>& stateStr,
  std::function<std::string(Symbol)>& symStr)
{
  int tid = 0;
  std::string str = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\"?>\n";
  str += "<structure label-on=\"transition\" type=\"fa\">\n";
  str += "<alphabet type=\"classical\">\n";
  for(auto s : this->getAlphabet())
    str += "<symbol>" + symStr(s) + "</symbol>\n";
  str += "</alphabet>\n";

  str += "<stateset>\n";
  for(auto st : this->states)
    str += "<state sid=\"" + stateStr(st) +  "\"></state>\n";
  str += "</stateset>\n";

  str += "<acc type=\"buchi\">\n";
  for(auto p : this->finals)
    str += "<stateID>" + stateStr(p) +  "</stateID>\n";
  str += "</acc>\n";

  str += "<initialStateSet>\n";
  for(auto p : this->initials)
    str += "<stateID>" + stateStr(p) +  "</stateID>\n";
  str += "</initialStateSet>\n";

  str += "<transitionset>\n";
  for (auto p : this->trans)
  {
    for(auto d : p.second)
    {
      str += "<transition tid=\"" + std::to_string(tid++) + "\">\n";
      str +=  "<from>" + stateStr(p.first.first) + "</from>\n<to>" + stateStr(d) +
        + "</to>\n<read>" + symStr(p.first.second) + "</read>\n";
      str += "</transition>";
    }
  }
  str += "</transitionset>\n";
  str += "</structure>\n";
  return str;
}


template <>
std::string BuchiAutomaton<std::string, std::string>::toGff()
{
  std::function<std::string(std::string)> f1 = [&] (std::string x) {return x;};
  std::function<std::string(std::string)> f2 = [&] (std::string x) {return x;};
  return toGffWith(f1, f2);
}


template <>
std::string BuchiAutomaton<int, int>::toGff()
{
  std::function<std::string(int)> f1 = [=] (int x) {return std::to_string(x);};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toGffWith(f1, f2);
}

template <>
std::string BuchiAutomaton<int, string>::toHOA()
{
  // TODO: enter correct symbols (now not retained while doing renameAut)
  std::string res;
  size_t alph_size = this->alph.size();
  res += "HOA: v1\n";
  res += "States: " + std::to_string(this->states.size()) + "\n";

  // renumber states to be a continuous sequence
  std::map<int, size_t> state_to_seq;
  size_t state_cnt = 0;
  for (auto st : this->states) {
    state_to_seq.insert({st, state_cnt++});
  }

  // initial states
  for (auto st : this->initials) {
    res += "Start: " + std::to_string(state_to_seq[st]) + "\n";
  }

  res += "acc-name: Buchi\n";
  res += "Acceptance: 1 Inf(0)\n";
  res += "AP: " + std::to_string(alph_size);

  // renumber symbols
  std::map<string, size_t> symb_to_pos;
  size_t symb_cnt = 0;
  for (auto symb : this->alph) {
    res += " \"" + symb + "\"";
    symb_to_pos.insert({symb, symb_cnt++});
  }

  // transitions
  res += "\n--BODY--\n";
  for (auto st : this->states) {
    size_t seq_st = state_to_seq[st];
    res += "State: " + std::to_string(seq_st);
    if (this->finals.find(st) != this->finals.end()) {
      res += " {0}";
    }
    res += "\n";

    for (auto symb : this->alph) {
      auto it = this->trans.find({st, symb});
      if (it == this->trans.end() || it->second.empty()) continue;

      // construct the string for the symbol first (composed of atomic propositions)
      std::string symb_str;
      symb_str += "[";
      bool first = true;
      for (size_t i = 0; i < alph_size; ++i) {
        if (first) first = false;
        else symb_str += " & ";

        if (symb_to_pos[symb] != i) symb_str += "!";
        symb_str += std::to_string(i);
      }
      symb_str += "]";

      for (auto dst : it->second) {
        res += symb_str + " " + std::to_string(state_to_seq[dst]) + "\n";
      }
    }
  }

  res += "--END--\n";

  return res;
}


template <>
std::string BuchiAutomaton<int, APSymbol>::toHOA()
{
  // TODO: enter correct symbols (now not retained while doing renameAut)
  std::string res;
  //size_t alph_size = this->alph.size();
  res += "HOA: v1\n";
  res += "States: " + std::to_string(this->states.size()) + "\n";

  // renumber states to be a continuous sequence
  std::map<int, size_t> state_to_seq;
  size_t state_cnt = 0;
  for (auto st : this->states) {
    state_to_seq.insert({st, state_cnt++});
  }

  // initial states
  for (auto st : this->initials) {
    res += "Start: " + std::to_string(state_to_seq[st]) + "\n";
  }

  res += "acc-name: Buchi\n";
  res += "Acceptance: 1 Inf(0)\n";
  res += "AP: " + std::to_string(this->apsPattern.size());

  for (auto symb : this->apsPattern) {
    res += " \"" +  symb + "\"";
  }

  // transitions
  res += "\n--BODY--\n";
  for (auto st : this->states) {
    size_t seq_st = state_to_seq[st];
    res += "State: " + std::to_string(seq_st);
    if (this->finals.find(st) != this->finals.end()) {
      res += " {0}";
    }
    res += "\n";

    for (auto symb : this->alph) {
      auto it = this->trans.find({st, symb});
      if (it == this->trans.end() || it->second.empty()) continue;

      for (auto dst : it->second) {
        res += "[" + symb.toString() + "] " + std::to_string(state_to_seq[dst]) + "\n";
      }
    }
  }

  res += "--END--\n";

  return res;
}


template <typename State, typename Symbol>
std::string BuchiAutomaton<State, Symbol>::toGraphwizWith(std::function<std::string(State)>& stateStr,
  std::function<std::string(Symbol)>& symStr)
{
  std::string str = "digraph \" Automaton \" { rankdir=LR;\n { rank = LR }\n";
  str += "node [shape = doublecircle];\n";
  for(auto p : this->finals)
    str += "\"" + stateStr(p) + "\"\n";
  str += "node [shape = circle];";
  for(auto st : this->states)
    str += "\"" + stateStr(st) + "\"\n";
  str += "\"init0\";\n";
  for (auto p : this->initials)
    str += "\"init0\" -> \"" + stateStr(p) + "\"\n";
  for (auto p : this->trans)
  {
    for(auto d : p.second)
      str +=  "\"" + stateStr(p.first.first) + "\" -> \"" + stateStr(d) +
        + "\" [label = \"" + symStr(p.first.second) + "\"];\n";
  }
  str += "}\n";
  return str;
}


template <>
std::string BuchiAutomaton<int, int>::toGraphwiz()
{
  std::function<std::string(int)> f1 = [=] (int x) {return std::to_string(x);};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toGraphwizWith(f1, f2);
}

template <>
std::string BuchiAutomaton<StateSch, int>::toGraphwiz()
{
  std::function<std::string(StateSch)> f1 = [&] (StateSch x) {return x.toString();};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toGraphwizWith(f1, f2);
}

template <>
std::string BuchiAutomaton<std::string, std::string>::toGraphwiz()
{
  std::function<std::string(std::string)> f1 = [&] (std::string x) {return x;};
  std::function<std::string(std::string)> f2 = [&] (std::string x) {return x;};
  return toGraphwizWith(f1, f2);
}


template <>
std::string BuchiAutomaton<tuple<int, int, bool>, int>::toGraphwiz()
{
  std::function<std::string(tuple<int, int, bool>)> f1 = [&] (tuple<int, int, bool> x)
  {
    return "(" + std::to_string(std::get<0>(x)) + " " +
      std::to_string(std::get<1>(x)) + " " + std::to_string(std::get<2>(x)) + ")";
  };
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toGraphwizWith(f1, f2);
}


template <>
std::string BuchiAutomaton<int, int>::toString()
{
  std::function<std::string(int)> f1 = [=] (int x) {return std::to_string(x);};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toStringWith(f1, f2);
}


template <>
std::string BuchiAutomaton<std::string, std::string>::toString()
{
  std::function<std::string(std::string)> f1 = [&] (std::string x) {return x;};
  std::function<std::string(std::string)> f2 = [&] (std::string x) {return x;};
  return toStringWith(f1, f2);
}

template <>
std::string BuchiAutomaton<int, std::string>::toString()
{
  std::function<std::string(int)> f1 = [&] (int x) {return std::to_string(x);};
  std::function<std::string(std::string)> f2 = [&] (std::string x) {return x;};
  return toStringWith(f1, f2);
}

template <>
std::string BuchiAutomaton<StateKV, int>::toString()
{
  std::function<std::string(StateKV)> f1 = [&] (StateKV x) {return x.toString();};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toStringWith(f1, f2);
}


template <>
std::string BuchiAutomaton<tuple<int, int, bool>, int>::toString()
{
  std::function<std::string(tuple<int, int, bool>)> f1 = [&] (tuple<int, int, bool> x)
  {
    return "(" + std::to_string(std::get<0>(x)) + " " +
      std::to_string(std::get<1>(x)) + " " + std::to_string(std::get<2>(x)) + ")";
  };
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toStringWith(f1, f2);
}


template <>
std::string BuchiAutomaton<StateSch, int>::toString()
{
  std::function<std::string(StateSch)> f1 = [&] (StateSch x) {return x.toString();};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toStringWith(f1, f2);
}


template <>
std::string BuchiAutomaton<int, APSymbol>::toString()
{
  std::function<std::string(int)> f1 = [=] (int x) {return std::to_string(x);};
  std::function<std::string(APSymbol)> f2 = [&] (APSymbol x) {return x.toString();};
  return toStringWith(f1, f2);
}


template <>
void BuchiAutomaton<int, int>::getAutGraphComponents(AdjList& adjList, Vertices& vrt)
{
  vector<set<int> > adjListSet(this->states.size());
  for(auto st : this->states)
  {
    vrt.push_back({st, -1, -1, false});
    adjListSet[st] = set<int>();
  }
  for(auto tr : this->trans)
  {
    adjListSet[tr.first.first].insert(tr.second.begin(), tr.second.end());
  }
  for(unsigned i = 0; i < adjListSet.size(); i++)
  {
    adjList[i] = vector<int>(adjListSet[i].begin(), adjListSet[i].end());
  }
}


template <>
void BuchiAutomaton<int, int>::removeUseless()
{
  vector<vector<int> > adjList(this->states.size());
  vector<vector<int> > revList(this->states.size());
  vector<VertItem> vrt;

  getAutGraphComponents(adjList, vrt);
  for(unsigned i = 0; i < adjList.size(); i++)
  {
    for(auto dst : adjList[i])
      revList[dst].push_back(i);
  }

  AutGraph gr(adjList, vrt, this->finals);
  gr.computeSCCs();
  set<int> fin;
  for(auto s : gr.getFinalComponents())
  {
    fin.insert(s.begin(), s.end());
  }

  set<int> reach = gr.reachableVertices(this->getInitials());
  set<int> backreach = gr.reachableVertices(revList, fin);
  set<int> ret;

  std::set_intersection(reach.begin(),reach.end(),backreach.begin(),
    backreach.end(), std::inserter(ret, ret.begin()));
  restriction(ret);
}


template <typename State, typename Symbol>
void BuchiAutomaton<State, Symbol>::restriction(set<State>& st)
{
  Transitions newtrans;
  set<State> newfin;
  set<State> newini;
  for(auto tr : this->trans)
  {
    if(st.find(tr.first.first) == st.end())
      continue;

    set<State> dst;
    std::set_intersection(tr.second.begin(),tr.second.end(),st.begin(),
      st.end(), std::inserter(dst, dst.begin()));
    newtrans[tr.first] = dst;
  }

  std::set_intersection(this->finals.begin(),this->finals.end(),st.begin(),
    st.end(), std::inserter(newfin, newfin.begin()));
  std::set_intersection(this->initials.begin(),this->initials.end(),st.begin(),
    st.end(), std::inserter(newini, newini.begin()));
  this->trans = newtrans;
  this->states = st;
  this->finals = newfin;
  this->initials = newini;
}


template <typename State, typename Symbol>
void BuchiAutomaton<State, Symbol>::complete(State trap, bool fin)
{
  bool modif = false;
  set<State> trSet({trap});
  for(State st : this->states)
  {
    for(Symbol s : this->alph)
    {
      auto pr = std::make_pair(st, s);
      if(this->trans.find(pr) == this->trans.end() || this->trans[pr].size() == 0)
      {
        modif = true;
        this->trans[pr] = trSet;
      }
    }
  }
  if(modif)
  {
    for(Symbol s : this->alph)
    {
      auto pr = std::make_pair(trap, s);
      this->trans[pr] = trSet;
    }
    this->states.insert(trap);
    if(fin)
      this->finals.insert(trap);
  }
}


template <>
void BuchiAutomaton<int, APSymbol>::completeAPComplement()
{
  this->complete(this->getStates().size(), false);
  set<APSymbol> allsyms;
  if(this->getAPPattern().size() > 0)
  {
    vector<int> cnum(this->getAPPattern().size());
    std::iota(cnum.begin(), cnum.end(), 0);
    for(const auto& s : Aux::getAllSubsets(cnum))
    {
      APSymbol sym(this->getAPPattern().size());
      for(const int& t : s)
        sym.ap.set(t);
      allsyms.insert(sym);
    }
  }
  this->setAlphabet(allsyms);
  this->complete(this->getStates().size(), true);
}


template <>
vector<set<int> > BuchiAutomaton<int, int>::reachableVector()
{
  vector<set<int> > adjListSet(this->states.size());
  vector<vector<int> > adjList(this->states.size());
  vector<set<int> > ret(this->states.size());
  for(auto st : this->states)
  {
    adjListSet[st] = set<int>();
  }
  for(auto tr : this->trans)
  {
    adjListSet[tr.first.first].insert(tr.second.begin(), tr.second.end());
  }
  for(auto st : this->states)
  {
    adjList[st] = vector<int>(adjListSet[st].begin(), adjListSet[st].end());
  }

  for(auto st : this->states)
  {
    set<int> tmp({st});
    ret[st] = AutGraph::reachableVertices(adjList, tmp);
  }
  return ret;
}


template <typename State, typename Symbol>
void BuchiAutomaton<State, Symbol>::computeRankSim(std::set<State>& cl)
{
  StateRelation rel = this->directSim;
  bool add = false;
  std::map<Symbol, bool> ignore;

  for(Symbol s : this->alph)
  {
    ignore[s] = false;
    for(State st : cl)
    {
      if(this->trans[{st, s}].size() == 0)
        ignore[s] = true;
    }

  }

  do {
    add = false;
    transitiveClosure(rel, cl);
    for(State st1 : this->states)
    {
      if(this->finals.find(st1) != this->finals.end())
        continue;
      for(State st2 : this->states)
      {
        if(this->finals.find(st2) != this->finals.end())
          continue;
        bool der = deriveRankConstr(st1, st2, rel);
        if(der)
          add = true;
      }
    }
  } while(add);
  this->oddRankSim = rel;
}


template <typename State, typename Symbol>
bool BuchiAutomaton<State, Symbol>::containsRankSimEq(std::set<State>& cl)
{
  this->computeRankSim(cl);
  for(auto& item : this->oddRankSim)
  {
    if(item.first == item.second)
      continue;
    if(this->oddRankSim.find({item.second, item.first}) != this->oddRankSim.end())
      return true;
  }
  return false;
}


template <typename State, typename Symbol>
void BuchiAutomaton<State, Symbol>::transitiveClosure(
    BuchiAutomaton<State, Symbol>::StateRelation& rel, std::set<State>& cl)
{
  unsigned s = rel.size();
  do
  {
    s = rel.size();
    for(auto p1 : rel)
    {
      if(cl.find(p1.first) == cl.end() || cl.find(p1.second) == cl.end())
        continue;
      for(auto p2 : rel)
      {
        if(cl.find(p2.first) == cl.end() || cl.find(p2.second) == cl.end())
          continue;
        if(p1.second == p2.first)
          rel.insert({p1.first, p2.second});
      }
    }
  } while(s != rel.size());
}


template <typename State, typename Symbol>
bool BuchiAutomaton<State, Symbol>::deriveRankConstr(State& st1, State& st2,
    BuchiAutomaton<State, Symbol>::StateRelation& rel)
{
  bool leq = true;
  bool geq = true;
  bool ret = false;
  //bool fwdlq = false;
  //bool fwdgq = false;
  StateRelation nw;

  for(Symbol sym : this->alph)
  {
    set<State> dst1 = this->trans[{st1, sym}];
    set<State> dst2 = this->trans[{st2, sym}];
    if(!isRankLeq(dst1, dst2, rel) /*&& !ignore[sym]*/)
      leq = false;
    if(!isRankLeq(dst2, dst1, rel) /*&& !ignore[sym]*/)
      geq = false;
    //propagateFwd(st1, st2, dst1, dst2, rel, nw);
  }

  if(leq) nw.insert({st1, st2});
  if(geq) nw.insert({st2, st1});

  for(auto it : nw)
  {
    if(rel.find(it) == rel.end())
      ret = true;
    rel.insert(it);
  }
  return ret;
}


template <typename State, typename Symbol>
void BuchiAutomaton<State, Symbol>::propagateFwd(State& st1, State& st2,
    std::set<State>& set1, std::set<State>& set2,
    BuchiAutomaton<State, Symbol>::StateRelation& rel,
    BuchiAutomaton<State, Symbol>::StateRelation& nw)
{
  std::set<State> fset1, fset2;
  std::set_difference(set1.begin(), set1.end(), this->finals.begin(), this->finals.end(),
    std::inserter(fset1, fset1.begin()));
  std::set_difference(set2.begin(), set2.end(), this->finals.begin(), this->finals.end(),
    std::inserter(fset2, fset2.begin()));

  if(fset1.size() == 1 && fset2.size() == 1 && rel.find({st1, st2}) != rel.end())
    nw.insert({*(fset1.begin()), *(fset2.begin())});
  if(fset1.size() == 1 && fset2.size() == 1 && rel.find({st2, st1}) != rel.end())
    nw.insert({*(fset2.begin()), *(fset1.begin())});
}



template <typename State, typename Symbol>
bool BuchiAutomaton<State, Symbol>::isRankLeq(std::set<State>& set1, std::set<State>& set2,
    BuchiAutomaton<State, Symbol>::StateRelation& rel)
{
  for(State st1 : set1)
  {
    if(this->finals.find(st1) != this->finals.end())
      continue;
    for(State st2 : set2)
    {
      if(this->finals.find(st2) != this->finals.end())
        continue;
      if(rel.find({st1, st2}) == rel.end())
        return false;
    }
  }
  return true;
}


template <typename State, typename Symbol>
std::map<State, int> BuchiAutomaton<State, Symbol>::propagateGraphValues(
    const std::function<int(LabelState<State>*,VecLabelStatesPtr)>& updFnc, const std::function<int(const State&)>& initFnc)
{
  std::map<State, LabelState<State>*> lst;
  VecLabelStatesPtr active;
  std::map<State, std::vector<LabelState<State>*>> tr;
  for(State st : this->states)
  {
    LabelState<State>* nst = new LabelState<State>;
    nst->label = initFnc(st);
    nst->state = st;

    active.push_back(nst);
    lst[st] = nst;
    tr[st] = std::vector<LabelState<State>*>();
  }

  for(auto t : this->trans)
  {
    for(auto d : t.second)
    {
      tr[t.first.first].push_back(lst[d]);
    }
  }

  bool change = false;
  do {
    change = false;
    for(LabelState<State>* ls : active)
    {
      int nval = updFnc(ls, tr[ls->state]);
      if(nval != ls->label)
        change = true;
      ls->label = nval;
    }
  } while(change);

  map<State, int> activeVal;
  for(unsigned i = 0; i < active.size(); i++)
  {
    activeVal[active[i]->state] = active[i]->label;
    delete active[i];
  }

  return activeVal;
}


template <typename State, typename Symbol>
std::vector<Symbol> BuchiAutomaton<State, Symbol>::containsSelfLoop(State& state)
{
  vector<Symbol> ret;
  auto trans = this->getTransitions();
  for(const auto& a : this->getAlphabet())
  {
    set<State> dst = trans[std::make_pair(state, a)];
    auto it = dst.find(state);
    if(it != dst.end())
      ret.push_back(a);
  }
  return ret;
}


template <typename State, typename Symbol>
set<State> BuchiAutomaton<State, Symbol>::getSelfLoops()
{
  set<State> sl;
  for(const State& st : this->getStates())
  {
    for(const auto& a : this->getAlphabet())
    {
      set<State> dst = trans[std::make_pair(st, a)];
      auto it = dst.find(st);
      if(it != dst.end())
      {
        sl.insert(st);
        break;
      }
    }
  }
  return sl;
}


template <typename State, typename Symbol>
vector<set<State>> BuchiAutomaton<State, Symbol>::getAutGraphSCCs()
{
  BuchiAutomaton<int, int> renAut = this->renameAut();
  vector<vector<int>> adjList(this->states.size());
  vector<VertItem> vrt;
  vector<set<State>> sccs;

  renAut.getAutGraphComponents(adjList, vrt);
  AutGraph gr(adjList, vrt, renAut.getFinals());
  gr.computeSCCs();

  for(auto& scc : gr.getAllComponents())
  {
    set<State> singleScc;
    for(auto &st : scc)
    {
      singleScc.insert(this->invRenameMap[st]);
    }
    sccs.push_back(singleScc);
  }
  return sccs;
}


template <typename State, typename Symbol>
set<State> BuchiAutomaton<State, Symbol>::getEventReachable(set<State>& sls)
{
  BuchiAutomaton<int, int> renAut = this->renameAut();
  vector<vector<int>> adjList(this->states.size());
  std::set<int> ini = renAut.getInitials();
  vector<VertItem> vrt;
  std::stack<int> stack;
  std::set<int> done;
  std::set<State> ret;
  std::vector<int> sccSizeMp(this->states.size());

  for(int i : ini)
    stack.push(i);

  renAut.getAutGraphComponents(adjList, vrt);
  AutGraph gr(adjList, vrt, renAut.getFinals());
  gr.computeSCCs();

  for(auto& scc : gr.getAllComponents())
  {
    for(auto &st : scc)
    {
      if(scc.size() == 1 && sls.find(this->invRenameMap[st]) == sls.end())
        sccSizeMp[st] = 0;
      else
      {
        sccSizeMp[st] = scc.size();
        done.insert(st);
      }
    }
  }

  done = gr.reachableVertices(done);
  for(int st : done)
  {
    ret.insert(this->invRenameMap[st]);
  }
  return ret;
}


template <typename State, typename Symbol>
set<State> BuchiAutomaton<State, Symbol>::getCycleClosingStates(set<State>& slignore)
{
  set<State> ret;
  std::stack<State> stack;
  set<State> done;
  for(const State& in : this->initials)
    stack.push(in);

  while(stack.size() > 0)
  {
    State tst = stack.top();
    stack.pop();
    done.insert(tst);

    for(const Symbol& alp : this->alph)
    {
      for(const State& d : this->trans[{tst, alp}])
      {
        if(d == tst && slignore.find(d) != slignore.end())
          continue;
        if(done.find(d) != done.end())
        {
          if(reachWithRestriction(d, tst, done, ret))
            ret.insert(d);
        }
        else
        {
          stack.push(d);
        }
      }
    }
  }

  return ret;
}


template <typename State, typename Symbol>
bool BuchiAutomaton<State, Symbol>::reachWithRestriction(const State& from, const State& to, set<State>& restr, set<State>& high)
{
  set<State> ret;
  std::stack<State> stack;
  set<State> done;
  stack.push(from);

  while(stack.size() > 0)
  {
    State tst = stack.top();
    stack.pop();
    if(tst == to)
      return true;
    done.insert(tst);

    for(const Symbol& alp : this->alph)
    {
      for(const State& d : this->trans[{tst, alp}])
      {
        if(high.find(d) != high.end())
          continue;
        if(done.find(d) == done.end() && restr.find(d) != restr.end())
        {
          stack.push(d);
        }
      }
    }
  }
  return false;
}


template <>
bool BuchiAutomaton<int, int>::isEmpty()
{
  vector<vector<int> > adjList(this->states.size());
  vector<VertItem> vrt;

  getAutGraphComponents(adjList, vrt);

  AutGraph gr(adjList, vrt, this->finals);
  gr.computeSCCs();
  set<int> fin;
  for(auto s : gr.getFinalComponents())
  {
    fin.insert(s.begin(), s.end());
  }

  set<int> in;
  set<int> tmp = AutGraph::reachableVertices(adjList, this->getInitials());
  set_intersection(fin.begin(), fin.end(), tmp.begin(), tmp.end(),
    std::inserter(in, in.begin()));

  if(in.size() > 0)
    return false;
  return true;
}


template <typename State, typename Symbol>
BuchiAutomaton<tuple<State, int, bool>, Symbol> BuchiAutomaton<State, Symbol>::productBA(BuchiAutomaton<int, Symbol>& other)
{
  typedef tuple<State, int, bool> ProdState;
  set<ProdState> nstates;
  set<ProdState> nini;
  stack<ProdState> stack;
  set<State> fin1 = this->getFinals();
  set<int> fin2 = other.getFinals();
  auto tr1 = this->getTransitions();
  auto tr2 = other.getTransitions();
  map<std::pair<ProdState, Symbol>, set<ProdState>> ntr;
  set<ProdState> nfin;

  for(const State& st1 : this->getInitials())
  {
    for(const int& st2 : other.getInitials())
    {
      stack.push({st1, st2, 0});
      nstates.insert({st1, st2, 0});
      nini.insert({st1, st2, 0});
    }
  }

  while(stack.size() > 0)
  {
    ProdState act = stack.top();
    stack.pop();

    for(const Symbol& sym : this->getAlph())
    {
      set<ProdState> dst;
      for(const State& d1 : tr1[{std::get<0>(act), sym}])
      {
        for(const int& d2 : tr2[{std::get<1>(act), sym}])
        {
          if(!std::get<2>(act) && fin1.find(d1) != fin1.end())
          {
            dst.insert({d1, d2, 1});
          }
          else if(std::get<2>(act) && fin2.find(d2) != fin2.end())
          {
            dst.insert({d1, d2, 0});
          }
          else
          {
            dst.insert({d1, d2, std::get<2>(act)});
          }
        }
      }
      for(auto& item : dst)
      {
        if(nstates.find(item) == nstates.end())
        {
          nstates.insert(item);
          stack.push(item);
        }
      }
      ntr[{act, sym}] = dst;
    }
  }

  for(const State& st1 : this->getStates())
  {
    for(const int& f2 : fin2)
    {
      nstates.insert({st1, f2, 1});
      nfin.insert({st1, f2, 1});
    }
  }

  return BuchiAutomaton<tuple<State, int, bool>, Symbol>(nstates, nfin, nini, ntr);
}


template <typename State, typename Symbol>
BuchiAutomaton<State, Symbol> BuchiAutomaton<State, Symbol>::unionBA(BuchiAutomaton<State, Symbol>& other)
{

  set<State> nstates;
  set<State> nini;
  Transitions ntr(this->getTransitions());
  set<State> nfin;

  set_union(this->getStates().begin(), this->getStates().end(), other.getStates().begin(),
    other.getStates().end(), std::inserter(nstates, nstates.begin()));
  set_union(this->getInitials().begin(), this->getInitials().end(), other.getInitials().begin(),
    other.getInitials().end(), std::inserter(nini, nini.begin()));
  set_union(this->getFinals().begin(), this->getFinals().end(), other.getFinals().begin(),
    other.getFinals().end(), std::inserter(nfin, nfin.begin()));
  ntr.insert(other.getTransitions().begin(), other.getTransitions().end());
  return BuchiAutomaton<State, Symbol>(nstates, nfin, nini, ntr, this->getAlph());
}


template <typename State, typename Symbol>
void BuchiAutomaton<State, Symbol>::singleInitial(State init)
{
  auto& tr = this->getTransitions();
  this->states.insert(init);
  for(const Symbol& s : this->getAlph())
  {
    tr[{init, s}] = set<State>();
    for(const State& st : this->getInitials())
    {
      auto dst = tr[{st, s}];
      tr[{init, s}].insert(dst.begin(), dst.end());
    }
  }
  this->initials = set<State>({init});
}


template <typename State, typename Symbol>
vector<set<State>> BuchiAutomaton<State, Symbol>::getRunTree(vector<Symbol>& word)
{
  set<State> nstates;
  set<State> nini;
  Transitions ntr = this->getTransitions();
  set<State> nfin;
  vector<set<State>> ret;

  set<State> act = this->getInitials();

  ret.push_back(act);
  for(const Symbol& sym : word)
  {
    set<State> dst;
    for(const auto& item : act)
    {
      set<State> tmp = ntr[{item, sym}];
      dst.insert(tmp.begin(), tmp.end());
    }
    act = dst;
    ret.push_back(dst);
  }
  return ret;
}


template <typename State, typename Symbol>
Delta<State, Symbol> BuchiAutomaton<State, Symbol>::getReverseTransitions()
{
  Transitions prev;
  for(const State& s : this->getStates())
  {
    for(const Symbol& a : this->getAlphabet())
      prev[{s,a}] = set<State>();
  }
  for(const auto& t : this->getTransitions())
  {
    for(const auto& d : t.second)
      prev[{d,t.first.second}].insert(t.first.first);
  }
  return prev;
}


template <typename State, typename Symbol>
BuchiAutomaton<State, Symbol> BuchiAutomaton<State, Symbol>::reverseBA()
{
  Transitions rev = this->getReverseTransitions();
  return BuchiAutomaton(this->getStates(), this->getInitials(), this->getFinals(), rev, this->getAlphabet());
}


template class BuchiAutomaton<int, int>;
template class BuchiAutomaton<int, string>;
template class BuchiAutomaton<tuple<int, int, bool>, int>;
template class BuchiAutomaton<std::string, std::string>;
template class BuchiAutomaton<StateKV, int>;
template class BuchiAutomaton<StateSch, int>;
template class BuchiAutomaton<int, APSymbol>;
