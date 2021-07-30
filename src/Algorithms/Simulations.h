#ifndef _BUCHI_SIMULATION_H_
#define _BUCHI_SIMULATION_H_

#include <iostream>
#include <fstream>
#include <string>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <array>
#include <exception>
#include <queue>
#include <map>
#include <set>
#include <string>

#include <boost/algorithm/string.hpp>
#include "../Automata/BuchiAutomaton.h"
#include "../Automata/GeneralizedBuchiAutomaton.h"

using namespace std;

/*
 * Relation type
 */
template<typename State> using Relation = set<pair<State, State>>;

/*
 * Timeout expired exception
 */
class TimeoutException: public exception
{
  virtual const char* what() const throw()
  {
    return "Timeout expired";
  }
};

/*
 * Rabit relation element
 */
struct RabitSimLine
{
  pair<string, string> rel;
  pair<string, string> aut;
};

class Simulations {

private:
  static RabitSimLine parseRabitRelLine(string& line);
  static void splitString(string& line, vector<string>& split);

public:
  BuchiAutomaton<std::string, std::string>::StateRelation parseRabitRelation(istringstream & os);
  static string execCmdTO(string& cmd, int timeout = 1000);
  static string execCmd(string& cmd);

  /*
   * Compute direct simulation
   * @param ba Buchi automaton
   * @param sink Sink state
   * @return Direct simulation
   */
  template<typename State, typename Symbol>
  Relation<State> directSimulation(BuchiAutomaton<State, Symbol>& ba, State sink)
  {
    BuchiAutomaton<State, Symbol> baTmp(ba);
    baTmp.complete(sink);

    Relation<State> dir;
    Relation<State> comp = computeDirectCompl(baTmp);
    for(const State& s1 : baTmp.getStates())
    {
      for(const State& s2 : baTmp.getStates())
      {
        if(comp.find({s1, s2}) == comp.end() && s1 != sink && s2 != sink)
          dir.insert({s1, s2});
      }
    }
    return dir;
  };

protected:

  /*
   * Compute complement of direct simulation
   * @param ba Buchi automaton
   * @return Complement of direct simulation
   */
  template<typename State, typename Symbol>
  Relation<State> computeDirectCompl(BuchiAutomaton<State, Symbol>& ba)
  {
    Delta<State, Symbol> revTr = ba.getReverseTransitions();
    map<tuple<Symbol, State, State>, unsigned> counter;
    for(const Symbol& sym : ba.getAlphabet())
    {
      for(const State& s1 : ba.getStates())
      {
        for(const State& s2 : ba.getStates())
          counter[{sym, s1, s2}] = 0;
      }
    }

    queue<pair<State, State>> proc;
    Relation<State> ret;

    set<State> nofin;
    set<State> fin = ba.getFinals();
    auto trans = ba.getTransitions();

    std::set_difference(ba.getStates().begin(), ba.getStates().end(), fin.begin(),
      fin.end(), std::inserter(nofin, nofin.begin()));
    for(const State& f : fin)
    {
      for(const State& nf : nofin)
      {
        ret.insert({f, nf});
        proc.push({f, nf});
      }
    }

    while(proc.size() > 0)
    {
      auto item = proc.front();
      proc.pop();
      for(const Symbol& a : ba.getAlphabet())
      {
        for(const State& k : revTr[{item.second, a}])
        {
          counter[{a, item.first, k}] += 1;
          if(counter[{a, item.first, k}] == trans[{k, a}].size())
          {
            for(const State& m : revTr[{item.first, a}])
            {
              if(ret.find({m,k}) == ret.end())
              {
                ret.insert({m,k});
                proc.push({m,k});
              }
            }
          }
        }
      }
    }
    return ret;
  };
};

#endif
