#ifndef _AUTOMATON_STRUCT_H_
#define _AUTOMATON_STRUCT_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <algorithm>
#include <tuple>
#include <functional>
#include <numeric>
#include <chrono>

#include "AutGraph.h"
#include "../Complement/StateKV.h"
#include "../Complement/StateSch.h"
#include "../Complement/StateGcoBA.h"
#include "../Algorithms/AuxFunctions.h"
#include "APSymbol.h"

class AutGraph;


/*
 * Single transition
 */
template <typename State, typename Symbol>
struct Transition {
  State from;
  State to;
  Symbol symbol;
};

/*
 * States extended with labels
 */
template <typename State, typename Label>
struct LabelState {
  State state;
  Label label;
};

/*
 * Transition function
 */
template<typename State, typename Symbol> using Delta = std::map<std::pair<State, Symbol>, std::set<State>>;
template<typename State, typename Label> using VecLabelStatesPtr = std::vector<LabelState<State, Label>* >;


template <typename State, typename Symbol>
class AutomatonStruct {

public:
  typedef std::set<State> SetStates;
  typedef std::set<Symbol> SetSymbols;
  typedef Delta<State, Symbol> Transitions;
  typedef std::set<std::pair<State, State> > StateRelation;

protected:
  SetStates states;
  SetStates initials;
  SetSymbols alph;
  Delta<State, Symbol> trans;
  vector<string> apsPattern;


  std::map<State, int> renameStateMap;
  std::map<Symbol, int> renameSymbolMap;
  std::vector<State> invRenameMap;

private:
  std::set<Symbol> getAlph();

public:

  AutomatonStruct(SetStates st, SetStates ini, Transitions trans)
  {
    this->states = st;
    this->trans = trans;
    this->initials = ini;
    this->alph = getAlph();
    this->apsPattern = vector<string>();
  }

  AutomatonStruct(SetStates st, SetStates ini, Transitions trans, SetSymbols alp)
  {
    this->states = st;
    this->trans = trans;
    this->initials = ini;
    this->alph = alp;
    this->apsPattern = vector<string>();
  }

  AutomatonStruct(SetStates st, SetStates ini, Transitions trans, SetSymbols alp, vector<string> aps)
  {
    this->states = st;
    this->trans = trans;
    this->initials = ini;
    if(alp.size() == 0)
      this->alph = getAlph();
    else
      this->alph = alp;
    this->apsPattern = aps;
  }

  AutomatonStruct() : AutomatonStruct({}, {}, {}) {};

  AutomatonStruct(AutomatonStruct<State, Symbol>& other)
  {
    this->states = other.states;
    this->trans = other.trans;
    this->initials = other.initials;
    this->alph = other.alph;
    this->renameStateMap = other.renameStateMap;
    this->renameSymbolMap = other.renameSymbolMap;
    this->invRenameMap = other.invRenameMap;
    this->apsPattern = other.apsPattern;
  }

  bool isReachDeterministic(set<State>& start);

  /*
   * Get automaton states.
   * @return Set of states
   */
  SetStates& getStates()
  {
    return this->states;
  }

  /*
   * Get automaton initial states.
   * @return Set of initial states
   */
  SetStates& getInitials()
  {
    return this->initials;
  }

  /*
   * Get automaton transitions.
   * @return Transitions: map<pair<State, Symbol>, Set<States>>
   */
  Transitions& getTransitions()
  {
    return this->trans;
  }

  /*
   * Get automaton alphabet.
   * @return Set of symbols
   */
  SetSymbols& getAlphabet()
  {
    return this->alph;
  }

  /*
   * Set automaton alphabet.
   * @params st New set of symbols
   */
  void setAlphabet(SetSymbols st)
  {
    this->alph = st;
  }

  /*
   * Get mapping used for renaming states of the automaton (created by calling
   * of renameAut(dict) method)
   * @return Mapping of states to int
   */
  std::map<State, int>& getRenameStateMap()
  {
    return this->renameStateMap;
  }

  /*
   * Get mapping used for renaming symbols of the automaton (created by calling
   * of renameAut method)
   * @return Mapping of symbols to int
   */
  std::map<Symbol, int>& getRenameSymbolMap()
  {
    return this->renameSymbolMap;
  }

  /*
   * Get inverse mapping used for renaming states of the automaton (created by calling
   * of renameAut method)
   * @return Mapping of ints to states
   */
  std::vector<State>& getInvRenameSymbolMap()
  {
    return this->invRenameMap;
  }

  /*
   * Set mapping used for renaming states of the automaton.
   * @param mp Mapping of states to int
   */
  void setRenameStateMap(std::map<State, int> mp)
  {
    this->renameStateMap = mp;
  }

  /*
   * Get atomic propositions
   * @return Vector of atomic propositions
   */
  vector<string> getAPPattern()
  {
    return this->apsPattern;
  }

  /*
   * Set atomic propositions
   * @param aps Vector of atomic propositions
   */
  void setAPPattern(vector<string> aps)
  {
    this->apsPattern = aps;
  }

  /*
   * Get the number of simple transitions
   * @return Transitions count
   */
  int getTransCount() const
  {
    int cnt = 0;
    for(const auto& t : this->trans)
    {
      cnt += t.second.size();
    }
    return cnt;
  }

  vector<set<State> > reachableVector();
  vector<Symbol> containsSelfLoop(State& state);

  void getAutGraphComponents(AdjList& adjList, Vertices& vrt);
  set<State> getSelfLoops();
  set<State> getAllSuccessors(State state);

  /*
   * Implementation of a simple data flow analysis. The values are iteratively
   * propagated through graph of the automaton.
   * @param updFnc Function updating values of the states
   * @param initFnc Function assigning initial values to states
   * @return Values assigned to each state after fixpoint
   */
  template <typename Label>
  std::map<State, Label> propagateGraphValues(const std::function<Label(LabelState<State, Label>*,VecLabelStatesPtr<State, Label>)>& updFnc,
    const std::function<Label(const State&)>& initFnc)
  {
    std::map<State, LabelState<State, Label>*> lst;
    VecLabelStatesPtr<State, Label> active;
    std::map<State, std::vector<LabelState<State, Label>*>> tr;
    for(State st : this->states)
    {
      LabelState<State, Label>* nst = new LabelState<State, Label>;
      nst->label = initFnc(st);
      nst->state = st;

      active.push_back(nst);
      lst[st] = nst;
      tr[st] = std::vector<LabelState<State, Label>*>();
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
      for(LabelState<State, Label>* ls : active)
      {
        Label nval = updFnc(ls, tr[ls->state]);
        if(nval != ls->label)
          change = true;
        ls->label = nval;
      }
    } while(change);

    map<State, Label> activeVal;
    for(unsigned i = 0; i < active.size(); i++)
    {
      activeVal[active[i]->state] = active[i]->label;
      delete active[i];
    }

    return activeVal;
  }

  /*
   * Is the automaton deterministic
   * @return True deterministic, false otherwise
   */
  bool isDeterministic()
  {
    return this->initials.size() <= 1 && isReachDeterministic(this->initials);
  }

  bool reachWithRestriction(const State& from, const State& to, SetStates& restr, SetStates& high);
  void singleInitial(State init);
  Delta<State, Symbol> getReverseTransitions();
  vector<set<State>> getRunTree(vector<Symbol>& word);

};

#endif
