
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
BuchiAutomaton<int, int> BuchiAutomaton<State, Symbol>::renameAut()
{
  int stcnt = 0;
  int symcnt = 0;
  std::map<State, int> mpstate;
  std::map<Symbol, int> mpsymbol;
  std::set<int> rstate;
  std::map< std::pair<int, int>, std::set<int> > rtrans;
  std::set<int> rfin;
  std::set<int> rini;

  for(auto st : this->states)
  {
    auto it = mpstate.find(st);
    if(it == mpstate.end())
    {
      mpstate[st] = stcnt++;
    }
  }

  rstate = mapSet(mpstate, this->states);
  rini = mapSet(mpstate, this->initials);
  rfin = mapSet(mpstate, this->finals);
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
    std::set<int> to = mapSet(mpstate, p.second);
    rtrans.insert({std::make_pair(mpstate[p.first.first], val), to});
  }

  auto ret = BuchiAutomaton<int, int>(rstate, rfin, rini, rtrans);
  this->renameStateMap = mpstate;

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
std::string BuchiAutomaton<StateKV, int>::toString()
{
  std::function<std::string(StateKV)> f1 = [&] (StateKV x) {return x.toString();};
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
void BuchiAutomaton<int, int>::removeUseless()
{
  vector<set<int> > adjListSet(this->states.size());
  vector<vector<int> > adjList(this->states.size());
  vector<vector<int> > revList(this->states.size());
  vector<VertItem> vrt;
  for(auto st : this->states)
  {
    vrt.push_back({st, -1, -1, false});
    adjListSet[st] = set<int>();
  }
  for(auto tr : this->trans)
  {
    adjListSet[tr.first.first].insert(tr.second.begin(), tr.second.end());
  }
  for(int i = 0; i < adjListSet.size(); i++)
  {
    for(auto dst : adjListSet[i])
      revList[dst].push_back(i);
    adjList[i] = vector<int>(adjListSet[i].begin(), adjListSet[i].end());
  }

  AutGraph gr(adjList, vrt, this->finals);
  gr.computeFinalSCCs();
  set<int> fin;
  for(auto s : gr.getFinalComponents())
  {
    fin.insert(s.begin(), s.end());
  }

  set<int> reach = gr.reachableVertices(fin);
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
void BuchiAutomaton<State, Symbol>::complete(State trap)
{
  bool modif = false;
  set<State> trSet({trap});
  for(State st : this->states)
  {
    for(Symbol s : this->alph)
    {
      auto pr = std::make_pair(st, s);
      if(this->trans.find(pr) == this->trans.end())
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
  }
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
        bool der = deriveRankConstr(st1, st2, rel, ignore);
        if(der)
          add = true;
      }
    }
  } while(add);
  this->oddRankSim = rel;
}


template <typename State, typename Symbol>
void BuchiAutomaton<State, Symbol>::transitiveClosure(
    BuchiAutomaton<State, Symbol>::StateRelation& rel, std::set<State>& cl)
{
  int s = rel.size();
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
    BuchiAutomaton<State, Symbol>::StateRelation& rel,
    std::map<Symbol, bool>& ignore)
{
  bool leq = true;
  bool geq = true;
  bool ret = false;
  bool fwdlq = false;
  bool fwdgq = false;
  StateRelation nw;

  for(Symbol sym : this->alph)
  {
    set<State> dst1 = this->trans[{st1, sym}];
    set<State> dst2 = this->trans[{st2, sym}];
    if(!isRankLeq(dst1, dst2, rel) && !ignore[sym])
      leq = false;
    if(!isRankLeq(dst2, dst1, rel) && !ignore[sym])
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
  State fw1, fw2;
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



template class BuchiAutomaton<int, int>;
template class BuchiAutomaton<std::string, std::string>;
template class BuchiAutomaton<StateKV, int>;
template class BuchiAutomaton<StateSch, int>;
