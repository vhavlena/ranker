
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
  return BuchiAutomaton<int, int>(rstate, rfin, rini, rtrans);
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



template class BuchiAutomaton<int, int>;
template class BuchiAutomaton<std::string, std::string>;
template class BuchiAutomaton<StateKV, int>;
template class BuchiAutomaton<StateSch, int>;
