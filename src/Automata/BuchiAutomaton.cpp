#include "BuchiAutomaton.h"
#include <boost/math/special_functions/factorials.hpp>

/*
 * Get all symbols occuring within the transitions (requires iterating over all
 * transitions).
 * @return Set of symbols
 */
template <typename State, typename Symbol>
std::set<Symbol> BuchiAutomaton<State, Symbol>::getAlph()
{
  SetSymbols sym;
  for (auto p : this->trans)
    sym.insert(p.first.second);
  return sym;
}


/*
 * Rename states and symbols of the automaton (to consecutive numbers).
 * @param start Starting number for states
 * @return Renamed automaton
 */
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
  set<int> rsym;
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
  for(const auto& a : this->alph)
  {
    rsym.insert(symcnt);
    mpsymbol[a] = symcnt++;
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

  auto ret = BuchiAutomaton<int, int>(rstate, rfin, rini, rtrans, rsym);
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


/*
 * Rename states and symbols of the automaton to numbers (symbols are renamed
 * by the explicit map).
 * @param mpsymbol Explicit map assigning numbers to original symbols
 * @param start Starting number for states
 * @return Renamed automaton
 */
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
  set<int> rsym;
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
  for(const auto& t : this->alph)
  {
    rsym.insert(mpsymbol[t]);
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

  auto ret = BuchiAutomaton<int, int>(rstate, rfin, rini, rtrans, rsym);
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


/*
 * Abstract function converting the automaton to string.
 * @param stateStr Function converting a state to string
 * @param symStr Function converting a symbol to string
 * @return String representation of the automaton
 */
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


/*
 * Abstract function converting the automaton to gff format.
 * @param stateStr Function converting a state to string
 * @param symStr Function converting a symbol to string
 * @return Gff representation of the automaton
 */
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


/*
 * Function converting the automaton <string, string> to gff format.
 * @return Gff representation of the automaton
 */
template <>
std::string BuchiAutomaton<std::string, std::string>::toGff()
{
  std::function<std::string(std::string)> f1 = [&] (std::string x) {return x;};
  std::function<std::string(std::string)> f2 = [&] (std::string x) {return x;};
  return toGffWith(f1, f2);
}


/*
 * Function converting the automaton <int, int> to gff format.
 * @return Gff representation of the automaton
 */
template <>
std::string BuchiAutomaton<int, int>::toGff()
{
  std::function<std::string(int)> f1 = [=] (int x) {return std::to_string(x);};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toGffWith(f1, f2);
}


/*
 * Function converting the automaton <int, string> to hoa format.
 * @return Hoa representation of the automaton
 */
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
  res += "properties: trans-labels explicit-labels state-acc\n";
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


/*
 * Function converting the automaton <int, APSymbol> to hoa format.
 * @return Hoa representation of the automaton
 */
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
  res += "properties: trans-labels explicit-labels state-acc\n";
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


/*
 * Abstract function converting the automaton to graphwiz format.
 * @param stateStr Function converting a state to string
 * @param symStr Function converting a symbol to string
 * @return Graphwiz representation of the automaton
 */
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


/*
 * Function converting the automaton <int, int> to graphwiz format.
 * @return Graphwiz representation of the automaton
 */
template <>
std::string BuchiAutomaton<int, int>::toGraphwiz()
{
  std::function<std::string(int)> f1 = [=] (int x) {return std::to_string(x);};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toGraphwizWith(f1, f2);
}


/*
 * Function converting the automaton <StateSch, int> to graphwiz format.
 * @return Graphwiz representation of the automaton
 */
template <>
std::string BuchiAutomaton<StateSch, int>::toGraphwiz()
{
  std::function<std::string(StateSch)> f1 = [&] (StateSch x) {return x.toString();};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toGraphwizWith(f1, f2);
}


/*
 * Function converting the automaton <string, string> to graphwiz format.
 * @return Graphwiz representation of the automaton
 */
template <>
std::string BuchiAutomaton<std::string, std::string>::toGraphwiz()
{
  std::function<std::string(std::string)> f1 = [&] (std::string x) {return x;};
  std::function<std::string(std::string)> f2 = [&] (std::string x) {return x;};
  return toGraphwizWith(f1, f2);
}


/*
 * Function converting the automaton <tuple<int, int, bool>, int> to graphwiz format.
 * @return Graphwiz representation of the automaton
 */
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


/*
 * Function converting the automaton <tuple<StateSch, int, bool>, int> to graphwiz format.
 * @return Graphwiz representation of the automaton
 */
template <>
std::string BuchiAutomaton<tuple<StateSch, int, bool>, int>::toGraphwiz()
{
  std::function<std::string(tuple<StateSch, int, bool>)> f1 = [&] (tuple<StateSch, int, bool> x)
  {
    return "(" + std::get<0>(x).toString() + " " +
      std::to_string(std::get<1>(x)) + " " + std::to_string(std::get<2>(x)) + ")";
  };
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toGraphwizWith(f1, f2);
}


/*
 * Function converting the automaton <int, int> to string.
 * @return String representation of the automaton
 */
template <>
std::string BuchiAutomaton<int, int>::toString()
{
  std::function<std::string(int)> f1 = [=] (int x) {return std::to_string(x);};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toStringWith(f1, f2);
}


/*
 * Function converting the automaton <string, string> to string.
 * @return String representation of the automaton
 */
template <>
std::string BuchiAutomaton<std::string, std::string>::toString()
{
  std::function<std::string(std::string)> f1 = [&] (std::string x) {return x;};
  std::function<std::string(std::string)> f2 = [&] (std::string x) {return x;};
  return toStringWith(f1, f2);
}


/*
 * Function converting the automaton <StateKV, int> to string.
 * @return String representation of the automaton
 */
template <>
std::string BuchiAutomaton<StateKV, int>::toString()
{
  std::function<std::string(StateKV)> f1 = [&] (StateKV x) {return x.toString();};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toStringWith(f1, f2);
}


/*
 * Function converting the automaton <tuple<int, int, bool>, int> to string.
 * @return String representation of the automaton
 */
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


/*
 * Function converting the automaton <StateSch, int> to string.
 * @return String representation of the automaton
 */
template <>
std::string BuchiAutomaton<StateSch, int>::toString()
{
  std::function<std::string(StateSch)> f1 = [&] (StateSch x) {return x.toString();};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toStringWith(f1, f2);
}


/*
 * Function converting the automaton <int, APSymbol> to string.
 * @return String representation of the automaton
 */
template <>
std::string BuchiAutomaton<int, APSymbol>::toString()
{
  std::function<std::string(int)> f1 = [=] (int x) {return std::to_string(x);};
  std::function<std::string(APSymbol)> f2 = [&] (APSymbol x) {return x.toString();};
  return toStringWith(f1, f2);
}


/*
 * Function converting the automaton <int, int> to graph representation using
 * list of adjacent vertices. Assumes automaton to have states numbered from 0
 * (no gaps).
 * @return adjList List of adjacent vertices
 * @return vrt Vector of vertices
 */
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


/*
 * Remove unreachable and nonaccepting states from the automaton (in place
 * modification, assumes states numbered from 0 with no gaps).
 */
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


/*
 * Restrict the automaton wrt given states (in place modification).
 * @param st States that remain in the automaton
 */
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


/*
 * Complete  the automaton (in place modification)
 * @param trap New (possibly) trap state
 * @param fin Should be the trap state final?
 */
template <typename State, typename Symbol>
void BuchiAutomaton<State, Symbol>::complete(State trap, bool fin)
{
  bool modif = false;
  set<State> trSet({trap});

  if(this->states.empty())
  {

    modif = true;
    this->initials.insert(trap);
  }

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


/*
 * Complete  the automaton wrt all subsets of atomic propositions (in
 * place modification)
 */
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


/*
 * For each state compute a set of reachable states (assumes numbered states
 * from 0 with no gaps).
 * @return Vector of sets of reachable states
 */
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


/*
 * Compute the odd rank simulation (stores in this->oddRankSim)
 * @param cl Closure of states
 */
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


/*
 * Function checking wheather odd rank sim contains equivalent pairs.
 * @param cl Closure of states
 */
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


/*
 * Function computing transitive closure of give relation
 * @param rel Relation (out parameter)
 * @param cl Closure of states (for computation of odd rank simulation)
 */
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


/*
 * Get constraints for two states wrt odd rank simulation
 * @param st1 First state
 * @param st2 Second state
 * @param rel odd rank simulation
 */
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


/*
 * Propagate odd rank sim values to successor states (obsolete)
 */
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


/*
 * Are all states of set1 bigger than all states of set2?
 * @param set1 First set of states
 * @param set2 Second set of states
 * @param rel Rank simulation (or relation in general)
 * @return set1 >=(forall, forall) sett2
 */
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


/*
 * Implementation of a simple data flow analysis. The values are iteratively
 * propagated through graph of the automaton.
 * @param updFnc Function updating values of the states
 * @param initFnc Function assigning initial values to states
 * @return Values assigned to each state after fixpoint
 */
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


/*
 * Get self-loops symbols for a given state
 * @param state State for getting sl symbols
 * @return Set of self-loop symbols
 */
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


/*
 * Get all states containing self-loops
 * @return Set of all self-loop
 */
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


/*
 * Get SCCs of the automaton
 * @return Vector of SCCs (represented as a set of states)
 */
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


/*
 * Get eventually reachable states
 * @params sls States containing self-loops over the whole alphabet
 * @return Set of eventually reachable states
 */
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


/*
 * Get states closing a cycle in the automaton graph
 * @params slignore States containing self-loops to be ignored
 * @return Set of states closing a cycle
 */
template <typename State, typename Symbol>
set<State> BuchiAutomaton<State, Symbol>::getCycleClosingStates(set<State>& slignore)
{
  this->getAllCycles(); // test
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

template<typename State, typename Symbol>
void BuchiAutomaton<State, Symbol> :: unblock(int state, std::set<int> &blockedSet, std::map<int, std::set<int>> &blockedMap){
  for (auto w : blockedMap[state]){
    if (blockedSet.find(w) != blockedSet.end()){ // w is in blockedSet
      blockedSet.erase(w);
    }
    blockedMap[state].erase(w); // delete w from blocked maps
  }
}

template<typename State, typename Symbol>
bool BuchiAutomaton<State, Symbol> :: circuit(int state, std::vector<int> &stack, std::set<int> &blockedSet, std::map<int, 
  std::set<int>> &blockedMap, std::set<int> scc, AdjList adjlist, int startState, std::vector<std::vector<int>> &allCyclesRenamed) {
  bool flag = false;
  stack.push_back(state);
  blockedSet.insert(state);

  // for every successor
  for (auto succ : adjlist[state]){
    if (succ == startState){
      // cycle was closed -> states on stack
      flag = true;
      std::vector<int> cycle;
      for (auto state : stack){
        cycle.push_back(state);
      }
      if (not cycle.empty())
        cycle.push_back(startState);
      if (not std::any_of(cycle.begin(), cycle.end(), [startState](int i){return i < startState;}))
        allCyclesRenamed.push_back(cycle); // add new cycle to the vector of cycles
    }
    else if (blockedSet.find(succ) == blockedSet.end()){ // succ is not in blockedSet
      // successor is not in the blocked set
      if (this->circuit(succ, stack, blockedSet, blockedMap, scc, adjlist, startState, allCyclesRenamed)){
        flag = true;
      }
    }
  }

  if (flag){
    // unblock ...
    this->unblock(state, blockedSet, blockedMap);
  } else {
    // for every successor
    for (auto succ : adjlist[state]){
      if (blockedMap[state].find(succ) == blockedMap[state].end()){ // succ is not in blockedMap[state]
        blockedMap[state].insert(succ);
      }
    }
  }

  stack.pop_back(); // remove last element from stack
  return flag;
}

template<typename State, typename Symbol>
std::vector<std::vector<State>> BuchiAutomaton<State, Symbol> :: getAllCycles(){
  BuchiAutomaton<int, int> renAut = this->renameAut();
  vector<vector<int>> adjList(this->states.size());
  vector<VertItem> vrt;
  vector<set<State>> sccs;

  renAut.getAutGraphComponents(adjList, vrt);
  AutGraph gr(adjList, vrt, renAut.getFinals());
  gr.computeSCCs(); // all sccs

  std::vector<std::vector<int>> allCyclesRenamed;
  std::vector<std::vector<State>> allCycles;
  const std::set<int> emptySet;
  std::vector<std::set<int>> tmpVector;

  for(auto& scc : gr.getAllComponents()){ // for every scc
    for (auto &state : scc){ // for every state in scc
      std::vector<int> stack;
      std::set<int> blockedSet;
      std::map<int, std::set<int>> blockedMap;

      // insert all states in scc to blockedMap
      for(auto &state : scc){
        blockedMap.insert(std::pair<int, std::set<int>>(state, emptySet));
      }

      // circuit method: returns all cycles in allCyclesRenamed
      this->circuit(state, stack, blockedSet, blockedMap, scc, adjList, state, allCyclesRenamed);
    }
  }

  for (auto &cycle : allCyclesRenamed){
    //std::cout << "Cycle: ";
    std::vector<State> oneCycle;
    for (auto &state : cycle){
      oneCycle.push_back(this->invRenameMap[state]);
      //std::cout << state << " ";
    }
    allCycles.push_back(oneCycle);
    //std::cout << std::endl;
  }

  return allCycles;
}

template<typename State, typename Symbol>
std::set<State> BuchiAutomaton<State, Symbol> :: getAllSuccessors(State state){
  std::set<State> successors;
  std::set<State> tmp;
  for (auto symbol : this->alph){
    tmp = this->trans[std::pair<State, Symbol>(state, symbol)];
    successors.insert(tmp.begin(), tmp.end());
  }
  return successors;
}

int fact(int n)
{
    int res = 1;
    for (int i = 2; i <= n; i++)
        res = res * i;
    return res;
}

int nCr(int n, int r)
{
    return fact(n) / (fact(r) * fact(n - r));
}

template<typename State, typename Symbol>
unsigned BuchiAutomaton<State, Symbol> :: getAllPossibleRankings(unsigned maxRank, unsigned accStates, unsigned nonAccStates){
  unsigned even = std::pow(((maxRank+1)/2), accStates);

  unsigned odd = 0;
  unsigned tmpSum = 1;
  unsigned tmp;
  unsigned upperBound = nonAccStates - (maxRank-1)/2;
  unsigned innerUpperBound;
  for (unsigned r=0; r<=(maxRank-1)/2; r++){
    // every rank
    //FIXME
    innerUpperBound = nonAccStates - (maxRank-1)/2 + 1 + r;

    tmp = [](unsigned upperBound, unsigned innerUpperBound, unsigned tmpSum){
      unsigned tmpVar = 0;
      for (unsigned i=1; i<=upperBound; i++){
        //std::cout << innerUpperBound << " " << i << " " << tmpSum << std::endl;
        tmpVar += (nCr(innerUpperBound, i) * tmpSum);
      }
      return tmpVar;
    }(upperBound, innerUpperBound, tmpSum); 
    tmpSum = tmp;
    odd = tmp;   
  }

  return odd*even; //FIXME
}

/*
 * Get states closing a cycle in the automaton graph
 * @params slignore States containing self-loops to be ignored
 * @params dmap Delay map (mapping assigning information about macrostates for the Delay optimization)
 * @return Set of states and symbols for which transitions to the tight part should be generated
 */
template <typename State, typename Symbol>
std::map<State, std::set<Symbol>> BuchiAutomaton<State, Symbol> :: getCycleClosingStates(SetStates& slignore, DelayMap<State>& dmap) {
  std::map<State, std::set<Symbol>> statesToGenerate;
  //std::set<State> statesToGenerate;
  std::vector<std::vector<State>> allCycles;
  std::map<State, double> mapping;
  std::set<State> successors;
  std::set<State> allStates = this->states;
  std::set<State> tmpStates;
  std::vector<std::vector<State>> tmpCycles;

  allCycles = this->getAllCycles(); // get all cycles
  while (not allStates.empty()){
    mapping.clear(); //!!
    while(not allStates.empty() and std::any_of(allStates.begin(), allStates.end(), [&](State i){return mapping[i]==0.0;})){  
    // number for every state
    for (auto state : allStates){
      successors = this->getAllSuccessors(state); // all successors
      std::set<State> tmpSucc = successors;
      
      for (auto succ : successors){
        // only successors in cycles that are not covered yet
        if (not std::any_of(allCycles.begin(), allCycles.end(), [state, succ](std::vector<State> item){
          return [state, succ, item](){
            for (unsigned i=0; i<item.size()-1; i++){
              if (item[i] == state and item[i+1] == succ)
                return true;
            }
            return false;
          }();
        })){tmpSucc.erase(succ);}
      }
      successors = tmpSucc;

      unsigned rankings = [successors, this](DelayMap<State> &dmap){
          unsigned result = 0;
          for (auto succ : successors){
            if (dmap[succ].maxRank != 0)
              result += this->getAllPossibleRankings(dmap[succ].maxRank, dmap[succ].macrostateSize - dmap[succ].nonAccStates, dmap[succ].nonAccStates);
          }
          return result; 
        }(dmap);

      // number of cycles with this state that are not covered yet
      unsigned cycles = [allCycles, state](){
        unsigned tmp = 0;
        for (std::vector<State> cycle : allCycles){
          if (std::find(cycle.begin(), cycle.end(), state) != cycle.end())
            tmp++;
        }
        return tmp;
      }();

      mapping.insert(std::pair<State, double>(state, cycles!=0 ? ((double)rankings)/cycles : 0.0));
    }

    // remove states with 0.0
    tmpStates = allStates;
    for (auto state : allStates){
      if (mapping[state] == 0.0){
        // add them to statesToGenerate with no symbol
        std::set<Symbol> tmpSymbols;
        statesToGenerate.insert(std::pair<State, std::set<Symbol>>(state, tmpSymbols)); 
        tmpStates.erase(state);
        // remove all cycles with this state
        tmpCycles.clear();
        for (std::vector<State> cycle : allCycles){
          if (std::find(cycle.begin(), cycle.end(), state) == cycle.end())
            tmpCycles.push_back(cycle);
        }
        allCycles = tmpCycles;
      }
    }
    allStates = tmpStates;
    }
    
    // pick min
    State minState;
    double min = -1.0;
    bool first = true;
    if (allStates.size() == 0)
      break;
    for (auto state : allStates){
      if (first or mapping[state] < min){
        minState = state;
        min = mapping[state];
        first = false;
      }
    }

    // which transitions should be generated
    std::set<Symbol> symbols;
    std::set<State> cycleSucc;
    for (auto cycle : allCycles){
      for (unsigned i = 0; i < cycle.size()-1; i++){
        if (cycle[i] == minState){
          cycleSucc.insert(cycle[i+1]); 
          break;
        }
      }
    }
    
    for (auto succ : cycleSucc){
      for (auto a : this->alph){
        std::set<State> reachStates = this->trans[std::pair<State, Symbol>(minState, a)];
        if (reachStates.find(succ) != reachStates.end())
          symbols.insert(a);
      }
    }

    statesToGenerate.insert(std::pair<State, std::set<Symbol>>(minState, symbols)); 
    // remove all cycles with this state
    tmpCycles.clear();
    for (std::vector<State> cycle : allCycles){
      if (std::find(cycle.begin(), cycle.end(), minState) == cycle.end())
        tmpCycles.push_back(cycle);
    }
    allCycles = tmpCycles;
    allStates.erase(minState);
  }

  return statesToGenerate;
}

/*
 * Get reachable states wrt given restrictions
 * @params from Initial state
 * @params to Final state
 * @params restr States to be taken into account
 * @params high States closing cycle
 * @return Is there is a path between from and to?
 */
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


/*
 * Accept the automaton empty language (assumes states numbered from 0
 * with no gaps).
 */
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


/*
 * Compute intersection (product) with another BA
 * @param other Other BA
 * @return BA accepting intersection of both languages
 */
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


/*
 * Compute union (nondeterministic) with another BA
 * @param other Other BA
 * @return BA accepting intersection of both languages
 */
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


/*
 * Convert the automaton into automaton with a single initial state (in
 * place modification)
 * @param init New unique initial state
 */
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


/*
 * Get part of the determinized automaton wrt a single word
 * @param word Word for determinization
 * @return Vector of macrostates (set of states).
 */
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


/*
 * Get reversed transition function (reverse directions of transitions)
 * @return Reversed transition function
 */
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


/*
 * Get reversed BA (reverse directions of transitions, initials became
 * finals and vice versa)
 * @return Reversed BA
 */
template <typename State, typename Symbol>
BuchiAutomaton<State, Symbol> BuchiAutomaton<State, Symbol>::reverseBA()
{
  Transitions rev = this->getReverseTransitions();
  return BuchiAutomaton(this->getStates(), this->getInitials(), this->getFinals(), rev, this->getAlphabet());
}


/*
 * Get automaton with renamed states to StateSch (debug only)
 * @param mpst Map assigning original (int) states back to StateSch
 * @return Renamed BA
 */
template <>
BuchiAutomaton<StateSch, int> BuchiAutomaton<int, int>::getComplStructure(std::map<int, StateSch>& mpst)
{
  std::set<StateSch> rstate;
  Delta<StateSch, int> rtrans;
  std::set<StateSch> rfin;
  std::set<StateSch> rini;

  rstate = Aux::mapSet(mpst, this->states);
  rini = Aux::mapSet(mpst, this->initials);
  rfin = Aux::mapSet(mpst, this->finals);
  for(auto p : this->trans)
  {
    std::set<StateSch> to = Aux::mapSet(mpst, p.second);
    rtrans.insert({std::make_pair(mpst[p.first.first], p.first.second), to});
  }

  auto ret = BuchiAutomaton<StateSch, int>(rstate, rfin, rini, rtrans);
  return ret;
}


/*
 * Is the automaton reachable from start deterministic?
 * @param start Set of initial states
 * @return Deterministic
 */
template <typename State, typename Symbol>
bool BuchiAutomaton<State, Symbol>::isReachDeterministic(set<State>& start)
{
  set<State> visited(start.begin(), start.end());
  stack<State> stack;
  for(const auto& t : start)
    stack.push(t);

  while(stack.size() > 0)
  {
    State act = stack.top();
    stack.pop();

    for(const Symbol& sym : this->alph)
    {
      set<State> dest = this->trans[{act, sym}];
      if(dest.size() > 1)
        return false;
      if(dest.size() == 1)
      {
        auto item = dest.begin();
        if(visited.find(*item) == visited.end())
        {
          stack.push(*item);
          visited.insert(*item);
        }
      }
    }
  }
  return true;
}


template class BuchiAutomaton<int, int>;
template class BuchiAutomaton<int, string>;
template class BuchiAutomaton<tuple<int, int, bool>, int>;
template class BuchiAutomaton<tuple<StateSch, int, bool>, int>;
template class BuchiAutomaton<tuple<int, int, bool>, APSymbol>;
template class BuchiAutomaton<std::string, std::string>;
template class BuchiAutomaton<StateKV, int>;
template class BuchiAutomaton<StateSch, int>;
template class BuchiAutomaton<int, APSymbol>;
