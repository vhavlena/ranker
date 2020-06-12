
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
std::string BuchiAutomaton<StateKV<int>, int>::toString()
{
  std::function<std::string(StateKV<int>)> f1 = [&] (StateKV<int> x) {return x.toString();};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toStringWith(f1, f2);
}


template class BuchiAutomaton<int, int>;
template class BuchiAutomaton<std::string, std::string>;
template class BuchiAutomaton<StateKV<int>, int>;
