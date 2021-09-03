
#include "GenBuchiAutomaton.h"

/*
 * Abstract function converting the automaton to string.
 * @param stateStr Function converting a state to string
 * @param symStr Function converting a symbol to string
 * @return String representation of the automaton
 */
template<typename State, typename Symbol>
std::string GeneralizedBuchiAutomaton<State,Symbol>::toStringWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr){
  std::string str = "";
  for (auto p : this->initials)
    str += stateStr(p) + "\n";
  for (auto p : this->trans)
  {
    for(auto d : p.second)
      str += symStr(p.first.second) + "," + stateStr(p.first.first)
        + "->" + stateStr(d) + "\n";
  }
  for (auto it = this->finals.begin(); it != this->finals.end(); it++){
    for(auto p : it->second)
        str += stateStr(p) + "\n";
    str += "---\n"; // divider between accepting sets
  }

  if(str.back() == '\n')
    str.pop_back();
  return str;
}

template<typename State, typename Symbol>
std::string GeneralizedBuchiAutomaton<State, Symbol>::toGraphwizWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr){
  std::string str = "digraph \" Automaton \" { rankdir=LR;\n { rank = LR }\n";
  str += "node [shape = doublecircle];\n";
  for (auto st : this->states){
      std::string acc = "";
      for (auto it = this->finals.begin(); it != this->finals.end(); it++){
          if (it->second.find(st) != it->second.end()){
            if (acc != "")
                acc += ",";
            acc += stateStr(st);   
          }
      }
      if (acc != "")
        str += "\"" + stateStr(st) + "\" [label = \"" + stateStr(st) + " {" + acc + "}\"] " + "\n";
  }
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
std::string GeneralizedBuchiAutomaton<int, int>::toGraphwiz()
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
std::string GeneralizedBuchiAutomaton<StateSch, int>::toGraphwiz()
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
std::string GeneralizedBuchiAutomaton<std::string, std::string>::toGraphwiz()
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
std::string GeneralizedBuchiAutomaton<tuple<int, int, bool>, int>::toGraphwiz()
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
std::string GeneralizedBuchiAutomaton<tuple<StateSch, int, bool>, int>::toGraphwiz()
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
std::string GeneralizedBuchiAutomaton<pair<StateSch, int>, APSymbol>::toGraphwiz()
{
  std::function<std::string(pair<StateSch, int>)> f1 = [&] (pair<StateSch, int> x)
  {
    return "(" + x.first.toString() + " " + std::to_string(x.second) + ")";
  };
  std::function<std::string(APSymbol)> f2 = [=] (APSymbol x) {return x.toString();};
  return toGraphwizWith(f1, f2);
}

template<typename State, typename Symbol>
std::string GeneralizedBuchiAutomaton<State,Symbol>::toGffWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr){
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
  
    str += "<acc type=\"Generalized Buchi\">\n";
    for (auto it = this->finals.begin(); it != this->finals.end(); it++){
      str += "<AccSet>\n";
      for(auto p : it->second)
          str += "<stateID>" + stateStr(p) +  "</stateID>\n";
      str += "</AccSet>\n";
    }
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
 * Function converting the automaton <int, int> to string.
 * @return String representation of the automaton
 */
template <>
std::string GeneralizedBuchiAutomaton<int, int>::toString()
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
std::string GeneralizedBuchiAutomaton<std::string, std::string>::toString()
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
std::string GeneralizedBuchiAutomaton<StateKV, int>::toString()
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
std::string GeneralizedBuchiAutomaton<tuple<int, int, bool>, int>::toString()
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
std::string GeneralizedBuchiAutomaton<StateSch, int>::toString()
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
std::string GeneralizedBuchiAutomaton<int, APSymbol>::toString()
{
  std::function<std::string(int)> f1 = [=] (int x) {return std::to_string(x);};
  std::function<std::string(APSymbol)> f2 = [&] (APSymbol x) {return x.toString();};
  return toStringWith(f1, f2);
}

/*
 * Function converting the automaton <string, string> to gff format.
 * @return Gff representation of the automaton
 */
template <>
std::string GeneralizedBuchiAutomaton<std::string, std::string>::toGff(){
  std::function<std::string(std::string)> f1 = [&] (std::string x) {return x;};
  std::function<std::string(std::string)> f2 = [&] (std::string x) {return x;};
  return toGffWith(f1, f2);
}

template<>
std::string GeneralizedBuchiAutomaton<int, int>::toGff(){
  std::function<std::string(int)> f1 = [&] (int x) {return std::to_string(x);};
  std::function<std::string(int)> f2 = [&] (int x) {return std::to_string(x);};
  return toGffWith(f1, f2);
}

template<>
std::string GeneralizedBuchiAutomaton<int,std::string>::toHOA(){
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

  res += "acc-name: generalized-Buchi " + std::to_string(this->finals.size()) + "\n";
  res += "Acceptance: " + std::to_string(this->finals.size()) + " ";
  for (unsigned i=0; i<this->finals.size(); i++){
      if (i != 0)
        res += "&";
    res += "Inf(" + std::to_string(i) + ")";
  }
  res += "\n";
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
    bool empty = true;
    bool end = false;
    for (auto it = this->finals.begin(); it != this->finals.end(); it++){
        if (it->second.find(st) != it->second.end()){
            if (empty)
                res += "{";
            empty = false;
            end = true;
            res += std::to_string(it->first) + " ";
        }
    }
    if (end)
        res += "}";
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

template<typename State, typename Symbol>
GeneralizedBuchiAutomaton<int, int> GeneralizedBuchiAutomaton<State,Symbol>::renameAutDict(map<Symbol, int>& mpsymbol, int start){
    int stcnt = start;
    std::map<State, int> mpstate;
    std::set<int> rstate;
    Delta<int, int> rtrans;
    std::map<int, std::set<int>> rfin;
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
    rfin = Aux::mapMap(mpstate, this->finals);
    for(auto p : this->trans)
    {
      int val = mpsymbol[p.first.second];
      std::set<int> to = Aux::mapSet(mpstate, p.second);
      rtrans.insert({std::make_pair(mpstate[p.first.first], val), to});
    }

    auto ret = GeneralizedBuchiAutomaton<int, int>(rstate, rfin, rini, rtrans, rsym);
    this->renameStateMap = mpstate;
    this->renameSymbolMap = mpsymbol;

    ret.setAPPattern(this->apsPattern);
    return ret;
}

template<>
void GeneralizedBuchiAutomaton<int,int>::removeUseless(){
    vector<vector<int> > adjList(this->states.size());
    vector<vector<int> > revList(this->states.size());
    vector<VertItem> vrt;

    getAutGraphComponents(adjList, vrt);
    for(unsigned i = 0; i < adjList.size(); i++)
    {
      for(auto dst : adjList[i])
        revList[dst].push_back(i);
    }

    std::map<int, std::set<int>> allFinals = this->finals;
    AutGraph gr(adjList, vrt, allFinals);
    gr.computeSCCs(this->finals);
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
void GeneralizedBuchiAutomaton<State, Symbol>::restriction(set<State>& st)
{
    Transitions newtrans;
    map<int,set<State>> newfin;
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

template<>
bool GeneralizedBuchiAutomaton<int,int>::isEmpty(){
    vector<vector<int> > adjList(this->states.size());
    vector<VertItem> vrt;

    getAutGraphComponents(adjList, vrt);

    std::map<int, std::set<int>> allFinals = this->finals;
    AutGraph gr(adjList, vrt, allFinals);
    gr.computeSCCs(this->finals);
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

template<typename State, typename Symbol>
GeneralizedBuchiAutomaton<tuple<State, int>, Symbol> GeneralizedBuchiAutomaton<State,Symbol>::productGBA(GeneralizedBuchiAutomaton<int, Symbol>& other){
  typedef tuple<State, int> ProdState;
  set<ProdState> nstates;
  set<ProdState> nini;
  stack<ProdState> stack;
  map<int,set<State>> fin1 = this->getFinals();
  map<int,set<int>> fin2 = other.getFinals();
  auto tr1 = this->getTransitions();
  auto tr2 = other.getTransitions();
  map<std::pair<ProdState, Symbol>, set<ProdState>> ntr;
  map<int,set<ProdState>> nfin;

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
      for(const State& d1 : tr1[{std::get<0>(act), sym}])
      {
        for(const int& d2 : tr2[{std::get<1>(act), sym}])
        {
          dst.insert({d1,d2});
        }
      }
      for(auto& item : dst)
      {
        if(nstates.find(item) == nstates.end())
        {
          nstates.insert(item);
          stack.push(item);
          for (auto it = this->getFinals().begin(); it != this->getFinals().end(); it++){
              if (it->second.find(std::get<0>(item)) != it->second.end())
                nfin[it->first].insert(item);
          }
          for (auto it = other.getFinals().begin(); it != other.getFinals().end(); it++){
              if (it->second.find(std::get<1>(item)) != it->second.end())
                nfin[(it->first)+this->getFinals().size()].insert(item);
          }
        }
      }
      ntr[{act, sym}] = dst;
    }
  }

  return GeneralizedBuchiAutomaton<tuple<State, int>, Symbol>(nstates, nfin, nini, ntr);
}

template<typename State, typename Symbol>
GeneralizedBuchiAutomaton<tuple<State, int>, Symbol> GeneralizedBuchiAutomaton<State,Symbol>::cartProductGBA(GeneralizedBuchiAutomaton<int, Symbol>& other){
  typedef tuple<State, int> ProdState;
  set<ProdState> nstates;
  set<ProdState> nini;
  stack<ProdState> stack;
  map<int,set<State>> fin1 = this->getFinals();
  map<int,set<int>> fin2 = other.getFinals();
  auto tr1 = this->getTransitions();
  auto tr2 = other.getTransitions();
  map<std::pair<ProdState, Symbol>, set<ProdState>> ntr;
  map<int,set<ProdState>> nfin;

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
      for(const State& d1 : tr1[{std::get<0>(act), sym}])
      {
        for(const int& d2 : tr2[{std::get<1>(act), sym}])
        {
          dst.insert({d1,d2});
        }
      }
      for(auto& item : dst)
      {
        if(nstates.find(item) == nstates.end())
        {
          nstates.insert(item);
          stack.push(item);
          for (auto it = this->getFinals().begin(); it != this->getFinals().end(); it++){
              if (it->second.find(std::get<0>(item)) != it->second.end())
                nfin[it->first].insert(item);
          }
          for (auto it = other.getFinals().begin(); it != other.getFinals().end(); it++){
              if (it->second.find(std::get<1>(item)) != it->second.end())
                nfin[(it->first)+this->getFinals().size()].insert(item);
          }
        }
      }
      ntr[{act, sym}] = dst;
    }
  }

  return GeneralizedBuchiAutomaton<tuple<State, int>, Symbol>(nstates, nfin, nini, ntr);
}

template<typename State, typename Symbol>
GeneralizedBuchiAutomaton<State, Symbol> GeneralizedBuchiAutomaton<State,Symbol>::unionGBA(GeneralizedBuchiAutomaton<State, Symbol>& other){
  set<State> nstates;
  set<State> nini;
  Transitions ntr(this->getTransitions());
  map<int,set<State>> nfin;

  set_union(this->getStates().begin(), this->getStates().end(), other.getStates().begin(),
    other.getStates().end(), std::inserter(nstates, nstates.begin()));
  set_union(this->getInitials().begin(), this->getInitials().end(), other.getInitials().begin(),
    other.getInitials().end(), std::inserter(nini, nini.begin()));
  nfin = this->getFinals();

  // acceptance sets union
  unsigned i=0;
  for (auto it = other.getFinals().begin(); it != other.getFinals().end(); it++){
      if (i <= nfin.size())
        nfin[i].insert(it->second.begin(), it->second.end());
      else
        nfin.insert(i, it->second});
      i++;
  }

  // merge transitions
  for (auto it = other.getTransitions().begin(); it != other.getTransitions().end(); it++){
      if (ntr.find(it->first) != ntr.end())
        ntr[it->first].insert(it->second.begin(), it->second.end());
      else
        ntr.insert({it->first, it->second});
  }
  return GeneralizedBuchiAutomaton<State, Symbol>(nstates, nfin, nini, ntr, this->getAlphabet());
}

template<typename State, typename Symbol>
GeneralizedBuchiAutomaton<State, Symbol> GeneralizedBuchiAutomaton<State,Symbol>::reverseGBA(){
    //TODO
}

template class GeneralizedBuchiAutomaton<int, int>;
template class GeneralizedBuchiAutomaton<std::string, std::string>;
template class GeneralizedBuchiAutomaton<StateSch, int>;
template class GeneralizedBuchiAutomaton<int, APSymbol>;
template class GeneralizedBuchiAutomaton<StateSch, APSymbol>;