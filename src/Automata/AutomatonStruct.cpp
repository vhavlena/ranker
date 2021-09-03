#include "AutomatonStruct.h"

/*
 * Get all symbols occuring within the transitions (requires iterating over all
 * transitions).
 * @return Set of symbols
 */
template <typename State, typename Symbol>
std::set<Symbol> AutomatonStruct<State, Symbol>::getAlph()
{
  SetSymbols sym;
  for (auto p : this->trans)
    sym.insert(p.first.second);
  return sym;
}

/*
 * Function converting the automaton <int, int> to graph representation using
 * list of adjacent vertices. Assumes automaton to have states numbered from 0
 * (no gaps).
 * @return adjList List of adjacent vertices
 * @return vrt Vector of vertices
 */
template <>
void AutomatonStruct<int, int>::getAutGraphComponents(AdjList& adjList, Vertices& vrt)
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
 * For each state compute a set of reachable states (assumes numbered states
 * from 0 with no gaps).
 * @return Vector of sets of reachable states
 */
template <>
vector<set<int> > AutomatonStruct<int, int>::reachableVector()
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
 * Get self-loops symbols for a given state
 * @param state State for getting sl symbols
 * @return Set of self-loop symbols
 */
template <typename State, typename Symbol>
std::vector<Symbol> AutomatonStruct<State, Symbol>::containsSelfLoop(State& state)
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
set<State> AutomatonStruct<State, Symbol>::getSelfLoops()
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


template<typename State, typename Symbol>
std::set<State> AutomatonStruct<State, Symbol> :: getAllSuccessors(State state)
{
  std::set<State> successors;
  std::set<State> tmp;
  for (auto symbol : this->alph)
  {
    tmp = this->trans[std::pair<State, Symbol>(state, symbol)];
    successors.insert(tmp.begin(), tmp.end());
  }
  return successors;
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
bool AutomatonStruct<State, Symbol>::reachWithRestriction(const State& from, const State& to, set<State>& restr, set<State>& high)
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
 * Convert the automaton into automaton with a single initial state (in
 * place modification)
 * @param init New unique initial state
 */
template <typename State, typename Symbol>
void AutomatonStruct<State, Symbol>::singleInitial(State init)
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
vector<set<State>> AutomatonStruct<State, Symbol>::getRunTree(vector<Symbol>& word)
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
Delta<State, Symbol> AutomatonStruct<State, Symbol>::getReverseTransitions()
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
 * Is the automaton reachable from start deterministic?
 * @param start Set of initial states
 * @return Deterministic
 */
template <typename State, typename Symbol>
bool AutomatonStruct<State, Symbol>::isReachDeterministic(set<State>& start)
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


template class AutomatonStruct<int, int>;
template class AutomatonStruct<int, string>;
template class AutomatonStruct<tuple<int, int, bool>, int>;
template class AutomatonStruct<tuple<StateSch, int, bool>, int>;
template class AutomatonStruct<tuple<StateSch, int, bool>, APSymbol>;
template class AutomatonStruct<tuple<StateKV, int, bool>, int>;
template class AutomatonStruct<pair<int, int>, APSymbol>;
template class AutomatonStruct<pair<int, int>, int>;
template class AutomatonStruct<tuple<int, int, bool>, APSymbol>;
template class AutomatonStruct<tuple<string, int, bool>, string>;
template class AutomatonStruct<std::string, std::string>;
template class AutomatonStruct<pair<std::string, std::string>, std::string>;
template class AutomatonStruct<pair<std::string, int>, std::string>;
template class AutomatonStruct<StateKV, int>;
template class AutomatonStruct<StateSch, int>;
template class AutomatonStruct<int, APSymbol>;
template class AutomatonStruct<pair<StateSch, int>, APSymbol>;
template class AutomatonStruct<pair<StateSch, int>, int>;
template class AutomatonStruct<pair<StateKV, int>, int>;
template class AutomatonStruct<StateSch, APSymbol>;

template class AutomatonStruct<tuple<StateSch, int>, int>;
template class AutomatonStruct<tuple<StateSch, int>, APSymbol>;
template class AutomatonStruct<tuple<StateKV, int>, int>;
template class AutomatonStruct<tuple<int, int>, int>;
template class AutomatonStruct<tuple<int, int>, APSymbol>;
template class AutomatonStruct<tuple<string, int>, string>;

template class AutomatonStruct<StateGcoBA, int>;
template class AutomatonStruct<tuple<StateGcoBA, int, bool>, int>;
template class AutomatonStruct<pair<StateGcoBA, int>, int>;
//
