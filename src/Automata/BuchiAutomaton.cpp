#include "BuchiAutomaton.h"
#include <boost/math/special_functions/factorials.hpp>

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
  VecTrans<int, int> ftrans;
  this->invRenameMap = std::vector<State>(this->states.size() + start);

  for(const auto& st : this->states)
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
  for(auto& p : this->trans)
  {
    int val = mpsymbol[p.first.second];
    std::set<int> to = Aux::mapSet(mpstate, p.second);
    rtrans.insert({std::make_pair(mpstate[p.first.first], val), to});
  }
  for(unsigned i = 0; i < this->accTrans.size(); i++)
  {
    ftrans.push_back({ .from = mpstate[this->accTrans[i].from],
        .to = mpstate[this->accTrans[i].to],
        .symbol = mpsymbol[this->accTrans[i].symbol] });
  }

  auto ret = BuchiAutomaton<int, int>(rstate, rfin, rini, rtrans, ftrans, rsym);
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

  for(const auto& tr : this->accTrans)
    str += symStr(tr.symbol) + "," + stateStr(tr.from)
      + "->" + stateStr(tr.to) + "\n";

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
  /*
  TODO: add support for accepting transitions
  */
  assert(this->accTrans.size() == 0);

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
std::string BuchiAutomaton<std::string, std::string>::toGff(){
  std::function<std::string(std::string)> f1 = [&] (std::string x) {return x;};
  std::function<std::string(std::string)> f2 = [&] (std::string x) {return x;};
  return toGffWith(f1, f2);
}

template<>
std::string BuchiAutomaton<int, int>::toGff(){
  std::function<std::string(int)> f1 = [&] (int x) {return std::to_string(x);};
  std::function<std::string(int)> f2 = [&] (int x) {return std::to_string(x);};
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
  res += "properties: trans-labels explicit-labels\n";
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
      if (this->apsPattern.size() == 0)
      {
        symb_str += "t";
      }
      else 
      {
        bool first = true;
        for (size_t i = 0; i < alph_size; ++i) {
          if (first) first = false;
          else symb_str += " & ";

          if (symb_to_pos[symb] != i) symb_str += "!";
          symb_str += std::to_string(i);
        }
      }
      symb_str += "]";

      for (auto dst : it->second) {
        Transition<int, string> tr = { .from = st, .to = dst, .symbol = symb};
        if(std::find(this->accTrans.begin(), this->accTrans.end(), tr) != this->accTrans.end())
          res += symb_str + " " + std::to_string(state_to_seq[dst]) + " {0} \n";
        else
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
  res += "properties: trans-labels explicit-labels";
  if (!this->isTBA()) res += " state-acc\n";
  else res += "\n";
  res += "AP: " + std::to_string(this->apsPattern.size());

  for (const auto& item : this->apsPattern){
    res += " \"" + item + "\"";
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
        Transition<int, APSymbol> tr = { .from = st, .to = dst, .symbol = symb};
        if(std::find(this->accTrans.begin(), this->accTrans.end(), tr) != this->accTrans.end())
          res += "[" + (this->apsPattern.size() > 0 ? symb.toString() : "t") + "] " + std::to_string(state_to_seq[dst]) + " {0} \n";
        else
          res += "[" + (this->apsPattern.size() > 0 ? symb.toString() : "t") + "] " + std::to_string(state_to_seq[dst]) + "\n";
      }
    }
  }

  res += "--END--\n";

  return res;
}

template <>
std::string BuchiAutomaton<int, int>::toHOA(std::map<int, int> sccs)
{
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
  res += "properties: trans-labels explicit-labels";
  if (!this->isTBA()) res += " state-acc\n";
  else res += "\n";
  res += "AP: " + std::to_string(alph_size);

  // renumber symbols
  std::map<string, size_t> symb_to_pos;
  size_t symb_cnt = 0;
  for (auto symb : this->alph) {
    res += " \"" + std::to_string(symb) + "\"";
    symb_to_pos.insert({std::to_string(symb), symb_cnt++});
  }

  // transitions
  res += "\n--BODY--\n";
  for (auto st : this->states) {
    size_t seq_st = state_to_seq[st];
    res += "State: " + std::to_string(seq_st);

    // state label
    res += " \"" + std::to_string(sccs[st]) + "\"";

    /*for (auto scc : sccs){
      if (scc.states.find(st) != scc.states.end()){
        res += " \"" + std::to_string(scc.rank) + "\"";
        break;
      }
    }*/

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
      if (this->apsPattern.size() == 0)
      {
        symb_str += "t";
      }
      else {
        bool first = true;
        for (size_t i = 0; i < alph_size; ++i) {
          if (first) first = false;
          else symb_str += " & ";
  
          if (symb_to_pos[std::to_string(symb)] != i) symb_str += "!";
          symb_str += std::to_string(i);
        }
      }
      symb_str += "]";

      for (auto dst : it->second) {
        Transition<int, int> tr = { .from = st, .to = dst, .symbol = symb };
        if(std::find(this->accTrans.begin(), this->accTrans.end(), tr) != this->accTrans.end())
          res += symb_str + " " + std::to_string(state_to_seq[dst]) + " {0} \n";
        else
          res += symb_str + " " + std::to_string(state_to_seq[dst]) + "\n";
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
    {
      std::string isAcc = "";
      Transition<State, Symbol> tr = { .from = p.first.first, .to = d, .symbol = p.first.second };
      if(std::find(this->accTrans.begin(), this->accTrans.end(), tr) != this->accTrans.end())
        isAcc = "\\n{0}";
      str +=  "\"" + stateStr(p.first.first) + "\" -> \"" + stateStr(d) +
        + "\" [label = \"" + symStr(p.first.second) + isAcc + "\"];\n";
    }
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
 * Function converting the automaton <StateSemiDet, int> to graphwiz format.
 * @return Graphwiz representation of the automaton
 */
template <>
std::string BuchiAutomaton<StateSemiDet, int>::toGraphwiz()
{
  std::function<std::string(StateSemiDet)> f1 = [&] (StateSemiDet x) {return x.toString();};
  std::function<std::string(int)> f2 = [=] (int x) {return std::to_string(x);};
  return toGraphwizWith(f1, f2);
}

/*
 * Function converting the automaton <StateSemiDet, APSymbol> to graphwiz format.
 * @return Graphwiz representation of the automaton
 */
template <>
std::string BuchiAutomaton<StateSemiDet, APSymbol>::toGraphwiz()
{
  std::function<std::string(StateSemiDet)> f1 = [&] (StateSemiDet x) {return x.toString();};
  std::function<std::string(APSymbol)> f2 = [&] (APSymbol x) {return x.toString();};
  return toGraphwizWith(f1, f2);
}


/*
 * Function converting the automaton <pair<StateGcoBA, int>, APSymbol> to graphwiz format.
 * @return Graphwiz representation of the automaton
 */
template <>
std::string BuchiAutomaton<pair<StateGcoBA, int>, APSymbol>::toGraphwiz()
{
  std::function<std::string(pair<StateGcoBA, int>)> f1 = [&] (pair<StateGcoBA, int> x)
  {
    return "(" + x.first.toString() + " " + std::to_string(x.second) + ")";
  };
  std::function<std::string(APSymbol)> f2 = [=] (APSymbol x) {return x.toString();};
  return toGraphwizWith(f1, f2);
}


/*
 * Function converting the automaton <StateGcoBA, int>> to graphwiz format.
 * @return Graphwiz representation of the automaton
 */
template <>
std::string BuchiAutomaton<StateGcoBA, int>::toGraphwiz()
{
  std::function<std::string(StateGcoBA)> f1 = [&] (StateGcoBA x)
  {
    return x.toString();
  };
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
 * Function converting the automaton <pair<StateSch, int>, APSymbol> to graphwiz format.
 * @return Graphwiz representation of the automaton
 */
template <>
std::string BuchiAutomaton<pair<StateSch, int>, APSymbol>::toGraphwiz()
{
  std::function<std::string(pair<StateSch, int>)> f1 = [&] (pair<StateSch, int> x)
  {
    return "(" + x.first.toString() + " " + std::to_string(x.second) + ")";
  };
  std::function<std::string(APSymbol)> f2 = [=] (APSymbol x) {return x.toString();};
  return toGraphwizWith(f1, f2);
}

template <>
std::string BuchiAutomaton<StateSD, int>::toGraphwiz()
{
  std::function<std::string(StateSD)> f1 = [&] (StateSD x) {return x.toString();};
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
 * Function converting the automaton <StateKV, int> to string.
 * @return String representation of the automaton
 */
template <>
std::string BuchiAutomaton<StateGcoBA, int>::toString()
{
  std::function<std::string(StateGcoBA)> f1 = [&] (StateGcoBA x) {return x.toString();};
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

template <>
std::string BuchiAutomaton<StateSD, int>::toString()
{
  std::function<std::string(StateSD)> f1 = [&] (StateSD x) {return x.toString();};
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
 * Remove unreachable and nonaccepting states from the automaton (in place
 * modification, assumes states numbered from 0 with no gaps).
 */
template <>
void BuchiAutomaton<int, int>::removeUseless()
{
  vector<vector<int> > adjList(this->states.size());
  vector<vector<int> > revList(this->states.size());
  vector<VertItem> vrt;

  set<pair<int, int> > accTrSet;
  for(const auto& tr : this->accTrans)
  {
    accTrSet.insert({tr.from, tr.to});
  }

  getAutGraphComponents(adjList, vrt);
  for(unsigned i = 0; i < adjList.size(); i++)
  {
    for(auto dst : adjList[i])
      revList[dst].push_back(i);
  }

  AutGraph gr(adjList, vrt, this->finals);
  gr.computeSCCs(accTrSet);
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
  VecTrans<State, Symbol> ftrans;
  for(const auto& tr : this->trans)
  {
    if(st.find(tr.first.first) == st.end())
      continue;

    set<State> dst;
    std::set_intersection(tr.second.begin(),tr.second.end(),st.begin(),
      st.end(), std::inserter(dst, dst.begin()));
    newtrans[tr.first] = dst;
  }

  for(const auto& tr : this->accTrans)
  {
    if(st.find(tr.to) == st.end() || st.find(tr.from) == st.end())
      continue;
    ftrans.push_back(tr);
  }

  std::set_intersection(this->finals.begin(),this->finals.end(),st.begin(),
    st.end(), std::inserter(newfin, newfin.begin()));
  std::set_intersection(this->initials.begin(),this->initials.end(),st.begin(),
    st.end(), std::inserter(newini, newini.begin()));
  this->trans = newtrans;
  this->states = st;
  this->finals = newfin;
  this->initials = newini;
  this->accTrans = ftrans;
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

  APSymbol tmp(this->getAPPattern().size());
  bool first = true;
  set<int> diff;
  for(const APSymbol& s : this->getAlphabet())
  {
    if(first)
    {
      tmp = s;
      first = false;
    }
    else
    {
      for(unsigned i = 0; i < this->getAPPattern().size(); i++)
      {
        if(tmp.ap[i] != s.ap[i])
        {
          diff.insert(i);
          tmp.ap[i] = 2;
        }
      }
    }
  }
  
  for(unsigned i = 0; i < this->getAPPattern().size(); i++)
  {
    if(tmp.ap[i] != 2)
    {
      APSymbol sym(this->getAPPattern().size(), 2);
      sym.ap[i] = !tmp.ap[i];
      allsyms.insert(sym);
    }
  }

  if(this->getAlphabet().size() <= 1)
  {
    allsyms.insert(tmp);
  }

  if(diff.size() > 0)
  {
    vector<int> cnum;
    for(const int & t : diff)
      cnum.push_back(t);

    for(const auto& s : Aux::getAllSubsets(cnum))
    {
      APSymbol sym = tmp;
      for(const int& t : cnum)
        sym.ap[t] = 0;
      for(const int& t : s)
        sym.ap[t] = 1;
      allsyms.insert(sym);
    }
  }

  this->setAlphabet(allsyms);
  this->complete(this->getStates().size(), true);

  // if(this->getAPPattern().size() > 0)
  // {
  //   vector<int> cnum(this->getAPPattern().size());
  //   std::iota(cnum.begin(), cnum.end(), 0);
  //   for(const auto& s : Aux::getAllSubsets(cnum))
  //   {
  //     APSymbol sym(this->getAPPattern().size());
  //     for(const int& t : s)
  //       sym.ap[t] = 1;
  //     allsyms.insert(sym);
  //   }
  // }
  // this->setAlphabet(allsyms);
  // this->complete(this->getStates().size(), true);
}

/*
 * Compute the odd rank simulation (stores in this->oddRankSim)
 * @param cl Closure of states
 */
template <typename State, typename Symbol>
void BuchiAutomaton<State, Symbol>::computeRankSim(std::set<State>& cl)
{
  /*
  TODO: add support for accepting transitions
  */
  assert(this->accTrans.size() == 0);

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
  /*
  TODO: add support for accepting transitions
  */
  assert(this->accTrans.size() == 0);

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
  /*
  TODO: add support for accepting transitions
  */
  assert(this->accTrans.size() == 0);

  unsigned s = rel.size();
  do
  {
    s = rel.size();
    for(const auto& p1 : rel)
    {
      if(cl.find(p1.first) == cl.end() || cl.find(p1.second) == cl.end())
        continue;
      for(const auto& p2 : rel)
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
  /*
  TODO: add support for accepting transitions
  */
  assert(this->accTrans.size() == 0);

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
  /*
  TODO: add support for accepting transitions
  */
  assert(this->accTrans.size() == 0);

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
  /*
  TODO: add support for accepting transitions
  */
  assert(this->accTrans.size() == 0);

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
 * Get eventually reachable states
 * @params sls States containing self-loops over the whole alphabet
 * @return Set of eventually reachable states
 */
template <typename State, typename Symbol>
set<State> BuchiAutomaton<State, Symbol>::getEventReachable(set<State>& sls)
{
  BuchiAutomaton<int, int> renAutBA = this->renameAut();

  vector<vector<int>> adjList(this->states.size());
  std::set<int> ini = renAutBA.getInitials();
  vector<VertItem> vrt;
  std::stack<int> stack;
  std::set<int> done;
  std::set<State> ret;
  std::vector<int> sccSizeMp(this->states.size());

  for(int i : ini)
    stack.push(i);

  renAutBA.getAutGraphComponents(adjList, vrt);
  AutGraph gr(adjList, vrt, renAutBA.getFinals());
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
          if(this->reachWithRestriction(d, tst, done, ret))
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

  set<pair<int, int> > accTrSet;
  for(const auto& tr : this->accTrans)
  {
    accTrSet.insert({tr.from, tr.to});
  }

  AutGraph gr(adjList, vrt, this->finals);
  gr.computeSCCs(accTrSet);
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
  /*
  TODO: add support for accepting transitions
  */
  assert(this->accTrans.size() == 0);

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

    for(const Symbol& sym : this->getAlphabet())
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
 * Get cartesian product with another BA (assuming the second BA has all states final)
 * @param other Other BA
 * @return cartesian product of two BAs
 */
template <typename State, typename Symbol>
BuchiAutomaton<pair<State, int>, Symbol> BuchiAutomaton<State, Symbol>::cartProductBA(BuchiAutomaton<int, Symbol>& other)
{
  /*
  TODO: add support for accepting transitions
  */
  assert(this->accTrans.size() == 0);

  typedef pair<State, int> ProdState;
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
      stack.push({st1, st2});
      nstates.insert({st1, st2});
      nini.insert({st1, st2});
    }
  }

  while(stack.size() > 0)
  {
    ProdState act = stack.top();
    stack.pop();

    for(const Symbol& sym : this->getAlphabet())
    {
      set<ProdState> dst;
      for(const State& d1 : tr1[{act.first, sym}])
      {
        for(const int& d2 : tr2[{act.second, sym}])
        {
          dst.insert({d1, d2});
        }
      }
      for(auto& item : dst)
      {
        if(nstates.find(item) == nstates.end())
        {
          if(fin1.find(item.first) != fin1.end())
            nfin.insert(item);
          nstates.insert(item);
          stack.push(item);
        }
      }
      ntr[{act, sym}] = dst;
    }
  }

  return BuchiAutomaton<pair<State, int>, Symbol>(nstates, nfin, nini, ntr);
}


/*
 * Compute union (nondeterministic) with another BA
 * @param other Other BA
 * @return BA accepting intersection of both languages
 */
template <typename State, typename Symbol>
BuchiAutomaton<State, Symbol> BuchiAutomaton<State, Symbol>::unionBA(BuchiAutomaton<State, Symbol>& other)
{
  /*
  TODO: add support for accepting transitions
  */
  //assert(this->accTrans.size() == 0);

  set<State> nstates;
  set<State> nini;
  Transitions ntr(this->getTransitions());
  set<State> nfin;
  VecTransG finTrans(this->getFinTrans());

  set_union(this->getStates().begin(), this->getStates().end(), other.getStates().begin(),
    other.getStates().end(), std::inserter(nstates, nstates.begin()));
  set_union(this->getInitials().begin(), this->getInitials().end(), other.getInitials().begin(),
    other.getInitials().end(), std::inserter(nini, nini.begin()));
  set_union(this->getFinals().begin(), this->getFinals().end(), other.getFinals().begin(),
    other.getFinals().end(), std::inserter(nfin, nfin.begin()));
  finTrans.insert(finTrans.end(), other.getFinTrans().begin(), other.getFinTrans().end());


  // merge transitions
  for (auto it = other.getTransitions().begin(); it != other.getTransitions().end(); it++){
      if (ntr.find(it->first) != ntr.end())
        ntr[it->first].insert(it->second.begin(), it->second.end());
      else
        ntr.insert({it->first, it->second});
  }

  return BuchiAutomaton<State, Symbol>(nstates, nfin, nini, ntr, finTrans, this->getAlphabet(), this->apsPattern);
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
  VecTrans<State, Symbol> revAccTr;
  for(const auto& tr : this->accTrans)
  {
    revAccTr.push_back({ .from = tr.to, .to = tr.from, .symbol = tr.symbol });
  }

  return BuchiAutomaton(this->getStates(), this->getInitials(), this->getFinals(), rev, revAccTr, this->getAlphabet());
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
  for(auto& p : this->trans)
  {
    std::set<StateSch> to = Aux::mapSet(mpst, p.second);
    rtrans.insert({std::make_pair(mpst[p.first.first], p.first.second), to});
  }

  auto ret = BuchiAutomaton<StateSch, int>(rstate, rfin, rini, rtrans);
  return ret;
}


template <typename State, typename Symbol>
map<State, set<Symbol> > BuchiAutomaton<State, Symbol>::getPredSymbolMap()
{
  auto trans = this->getTransitions();
  map<State, set<Symbol>> ret;
  for(const State& s : this->getStates())
  {
    ret[s] = set<Symbol>();
    for(const Symbol& sym : this->getAlphabet())
    {
      if(trans[{s, sym}].size() > 0)
        ret[s].insert(sym);
    }
  }
  return ret;
}

template <typename State, typename Symbol>
map<State, set<Symbol>> BuchiAutomaton<State, Symbol>::getReverseSymbolMap(){
  auto trans = this->getReverseTransitions();
  map<State, set<Symbol>> ret;
  for(const State& s : this->getStates())
  {
    ret[s] = set<Symbol>();
    for(const Symbol& sym : this->getAlphabet())
    {
      if(trans[{s, sym}].size() > 0)
        ret[s].insert(sym);
    }
  }
  return ret;
}


template <>
BuchiAutomaton<int, int> BuchiAutomaton<int, int>::copyStateAcc(int start)
{
  auto origTrans = this->getTransitions();
  Delta<int,int> trans(origTrans.begin(), origTrans.end());
  map<int,int> stMap;
  set<int> fins;

  set<int> orig = this->getStates();
  set<int> states(orig.begin(), orig.end());

  for(const int& s : this->getStates())
  {
    stMap[s] = start;
    states.insert(start);
    start++;
  }
  for(auto &p : this->getTransitions())
  {
    set<int> dst = Aux::mapSet(stMap, p.second);
    trans.insert({std::make_pair(stMap[p.first.first], p.first.second), dst});
  }

  for(auto &p : this->getTransitions())
  {
    set<int> dst = Aux::mapSet(stMap, p.second);
    if(dst.size() > 0)
    {
      trans[p.first].insert(dst.begin(), dst.end());
    }
  }
  for(const auto& f : this->getFinals())
  {
    fins.insert(stMap[f]);
  }

  return BuchiAutomaton(states, fins, this->getInitials(), trans, this->getAlphabet(), this->getAPPattern());
}


/*
 * Set of all successors.
 * @param states Set of states to get successors
 * @param symbol Symbol
 * @return Set of successors over symbol
 */
template <typename State, typename Symbol>
set<State> BuchiAutomaton<State, Symbol>::succSet(const set<State>& states, const Symbol& symbol)
{
  set<State> ret;
  for(const State& st : states)
  {
    set<State> dst = this->getTransitions()[std::make_pair(st, symbol)];
    ret.insert(dst.begin(), dst.end());
  }
  return ret;
}


/*
 * Semideterminize the given automaton
 * @return Modified structure with equivalent language
 */
template <>
BuchiAutomaton<StateSemiDet, APSymbol> BuchiAutomaton<int, APSymbol>::semideterminize()
{
  /*
  TODO: add support for accepting transitions
  */
  assert(this->getFinTrans().size() == 0);

  std::stack<StateSemiDet> stack;
  set<StateSemiDet> comst;
  set<StateSemiDet> initials;
  set<StateSemiDet> finals;
  //set<StateSch> succ;
  set<APSymbol> alph = getAlphabet();
  map<std::pair<StateSemiDet, APSymbol>, set<StateSemiDet> > mp;
  map<std::pair<StateSemiDet, APSymbol>, set<StateSemiDet> >::iterator it;

  for(const int& i : this->getInitials())
  {
    StateSemiDet init = {i, {set<int>(), set<int>()}, true};
    stack.push(init);
    comst.insert(init);
    initials.insert(init);
  }

  set<int> fins = this->getFinals();

  set<int> ignore;
  set<int> closingStates = this->getCycleClosingStates(ignore);

  while(stack.size() > 0)
  {
    StateSemiDet st = stack.top();
    stack.pop();

    if(!st.isWaiting && st.tight.first == st.tight.second && st.tight.first.size() > 0)
    {
      finals.insert(st);
    }

    for(APSymbol const & sym : alph)
    {
      set<StateSemiDet> dst;
      if(st.isWaiting)
      {
        auto pr = std::make_pair(st.waiting, sym);
        set<int> tmpDst = this->getTransitions()[pr];
        for(const int& d : tmpDst)
        {
          StateSemiDet s1 = { d, {set<int>(), set<int>()}, true };
          dst.insert(s1);

          //if(closingStates.find(st.waiting) != closingStates.end())
          {
            StateSemiDet s2 = { -1, {set<int>({d}), set<int>()}, false };
            dst.insert(s2);
          }
        }
      }
      else
      {
        if(st.tight.first != st.tight.second)
        {
          set<int> succ = succSet(st.tight.first, sym);
          set<int> succ2;
          std::set_intersection(succ.begin(), succ.end(), fins.begin(), fins.end(),
            std::inserter(succ2, succ2.begin()));
          set<int> tmpsucc = succSet(st.tight.second, sym);
          succ2.insert(tmpsucc.begin(), tmpsucc.end());
          StateSemiDet s = { -1, {succ, succ2}, false };
          dst.insert(s);
        }
        else
        {
          set<int> succ = succSet(st.tight.first, sym);
          set<int> succ2;
          std::set_intersection(succ.begin(), succ.end(), fins.begin(), fins.end(),
            std::inserter(succ2, succ2.begin()));
          StateSemiDet s = { -1, {succ, succ2}, false };
          dst.insert(s);
        }
      }

      for(const StateSemiDet& d : dst)
      {
        if(comst.find(d) == comst.end())
        {
          stack.push(d);
          comst.insert(d);
        }
      }
      auto pr = std::make_pair(st, sym);
      mp[pr] = dst;
    }
  }

  return BuchiAutomaton<StateSemiDet, APSymbol>(comst, finals,
    initials, mp, alph, this->getAPPattern());
}


template <>
BuchiAutomaton<int, int> BuchiAutomaton<int, int>::removeUselessRename()
{
  map<int, int> id;
  for(auto al : this->getAlphabet())
    id[al] = al;
  BuchiAutomaton<int, int> renptr = this->renameAutDict(id);
  renptr.removeUseless();
  return renptr.renameAutDict(id);
}


template<typename State, typename Symbol>
BuchiAutomaton<int, Symbol> BuchiAutomaton<State,Symbol>::reduce()
{
  //assert(!this->isTBA() && "Reduce not supported for TBAs");
  assert(this->directSim.size() > 0 && "Simulation is not computed");

  map<State, int> stmap;
  set<set<State>> eqcl = Aux::getEqClasses(this->directSim, this->getStates());

  set<int> nst;
  set<int> nini;
  set<int> nfin;
  Delta<int, Symbol> ntr;

  int i = 0;
  for(const auto& cl : eqcl)
  {
    nst.insert(i);
    for(const State& st : cl)
      stmap[st] = i;
    i++;
  }

  nini = Aux::mapSet(stmap, this->initials);
  nfin = Aux::mapSet(stmap, this->finals);

  for(auto& p : this->trans)
  {
    Symbol val = p.first.second;
    std::set<int> to = Aux::mapSet(stmap, p.second);
    ntr[{ stmap[p.first.first], val} ].insert(to.begin(), to.end());
  }

  BuchiAutomaton<int, Symbol> ret(nst, nfin, nini, ntr, this->getAlphabet(), this->getAPPattern());
  VecTrans<int, Symbol> newTrans;
  for (auto tr : this->getFinTrans()){
    Transition<int, Symbol> tmp = {.from = stmap[tr.from], .to = stmap[tr.to], .symbol = tr.symbol};
    newTrans.push_back(tmp);
  }
  ret.setFinTrans(newTrans);
  return ret;
}


template<typename State, typename Symbol>
BuchiAutomaton<int, Symbol> BuchiAutomaton<State,Symbol>::toTBA()
{
  //assert(!this->isTBA() && "Reduce not supported for TBAs");

  map<State, int> stmap;
  map<State, set<Symbol>> predSyms = this->getPredSymbolMap();
  map<State, set<Symbol>> predSymsRev = this->getReverseSymbolMap();
  Delta<State, Symbol> revTr = this->getReverseTransitions();

  set<pair<State,State>> eqRel;
  for(const State& p : this->getStates())
  {
    for(const State& q : this->getStates())
    {
      if(predSyms[p] != predSyms[q])
        continue;

      bool add = true;
      for(const Symbol& s : predSyms[p])
      {
        if(this->trans[{p,s}] != this->trans[{q,s}])
        {
          add = false;
          break;
        }
      }
      if(add) eqRel.insert({p,q});
    }
  }

  set<set<State>> eqcl = Aux::getEqClasses(eqRel, this->getStates());

  set<int> nst;
  set<int> nini;
  set<int> nfin;
  set<State> generateFinTrans;
  Delta<int, Symbol> ntr;
  VecTrans<int, Symbol> accTrans;

  int i = 0;
  for(const auto& cl : eqcl)
  {
    nst.insert(i);
    for(const State& st : cl)
      stmap[st] = i;
    if(std::includes(this->finals.begin(), this->finals.end(), cl.begin(), cl.end()))
    {
      nfin.insert(i);
    }
    else
    {
      for(const State& st : cl)
      {
        if(this->finals.find(st) != this->finals.end())
          generateFinTrans.insert(st);
      }
    }
    i++;
  }

  for(const State& src : generateFinTrans)
  {
    for(const Symbol& s : predSymsRev[src])
    {
      for(const State& dest : revTr[{src, s}])
      {
        Transition<int, Symbol> tr = {.from = stmap[dest], .to = stmap[src], .symbol = s};
        accTrans.push_back(tr);
      }
    }
  }

  nini = Aux::mapSet(stmap, this->initials);

  for(auto& p : this->trans)
  {
    Symbol val = p.first.second;
    std::set<int> to = Aux::mapSet(stmap, p.second);
    ntr[{ stmap[p.first.first], val} ].insert(to.begin(), to.end());
  }

  return BuchiAutomaton<int, Symbol>(nst, nfin, nini, ntr, accTrans, this->getAlphabet(), this->getAPPattern());
}


template<>
BuchiAutomaton<int, APSymbol> BuchiAutomaton<int,int>::toAPBA(map<int, string>& symName)
{
  map<int, APSymbol> symToPos;
  set<APSymbol> newAlph;
  vector<string> apPat;

  int cnt = 0;
  int num = this->getAlphabet().size();
  for(int sym : this->getAlphabet())
  {
    APSymbol ap(num);
    ap.ap[cnt] = 1;
    symToPos[sym] = ap;
    newAlph.insert(ap);
    apPat.push_back(symName[sym]);
    cnt++;
  }

  auto ret = renameAlphabet(symToPos);
  ret.setAPPattern(apPat);

  return ret;
}




template class BuchiAutomaton<int, int>;
template class BuchiAutomaton<std::string, std::string>;
template class BuchiAutomaton<StateSch, int>;
template class BuchiAutomaton<int, APSymbol>;
template class BuchiAutomaton<StateSch, APSymbol>;
template class BuchiAutomaton<StateGcoBA, int>;
template class BuchiAutomaton<StateGcoBA, APSymbol>;
template class BuchiAutomaton<StateSemiDet, int>;
template class BuchiAutomaton<StateSemiDet, APSymbol>;
template class BuchiAutomaton<StateSD, int>;
template class BuchiAutomaton<StateSD, APSymbol>;
