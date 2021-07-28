#include "GeneralizedBuchiAutomaton.h"

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
  for (auto s : this->finals){
    for(auto p : s)
        str += stateStr(p) + "\n";
    str += "---\n"; // divider between accepting sets
  }

  if(str.back() == '\n')
    str.pop_back();
  return str;
}

template<typename State, typename Symbol>
std::string GeneralizedBuchiAutomaton<State, Symbol>::toGraphwizWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr){
  //TODO
  std::string str = "digraph \" Automaton \" { rankdir=LR;\n { rank = LR }\n";
  str += "node [shape = doublecircle];\n";
  for(auto p : this->finals) //TODO
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

template<typename State, typename Symbol>
std::string GeneralizedBuchiAutomaton<State,Symbol>::toGffWith(std::function<std::string(State)>& stateStr,  std::function<std::string(Symbol)>& symStr){
    //TODO
}

template<typename State, typename Symbol>
std::string GeneralizedBuchiAutomaton<State,Symbol>::toString(){
    //TODO
}

template<typename State, typename Symbol>
std::string GeneralizedBuchiAutomaton<State,Symbol>::toGraphwiz(){
    //TODO
}

template<typename State, typename Symbol>
std::string GeneralizedBuchiAutomaton<State,Symbol>::toGff(){
    //TODO
}

template<typename State, typename Symbol>
std::string GeneralizedBuchiAutomaton<State,Symbol>::toHOA(){
    //TODO
}

/*template<typename State, typename Symbol>
GeneralizedBuchiAutomaton<int, int>* GeneralizedBuchiAutomaton<State,Symbol>::renameAut(int start){
    //TODO
}*/

template<typename State, typename Symbol>
GeneralizedBuchiAutomaton<int, int> GeneralizedBuchiAutomaton<State,Symbol>::renameAutDict(map<Symbol, int>& mpsymbol, int start){
    //TODO
}

template<typename State, typename Symbol>
void GeneralizedBuchiAutomaton<State,Symbol>::removeUseless(){
    //TODO
}

template<typename State, typename Symbol>
bool GeneralizedBuchiAutomaton<State,Symbol>::isEmpty(){
    //TODO
}

template<typename State, typename Symbol>
GeneralizedBuchiAutomaton<tuple<State, int, bool>, Symbol> GeneralizedBuchiAutomaton<State,Symbol>::productGBA(GeneralizedBuchiAutomaton<int, Symbol>& other){
    //TODO
}

template<typename State, typename Symbol>
GeneralizedBuchiAutomaton<pair<State, int>, Symbol> GeneralizedBuchiAutomaton<State,Symbol>::cartProductGBA(GeneralizedBuchiAutomaton<int, Symbol>& other){
    //TODO
}

template<typename State, typename Symbol>
GeneralizedBuchiAutomaton<State, Symbol> GeneralizedBuchiAutomaton<State,Symbol>::unionGBA(GeneralizedBuchiAutomaton<State, Symbol>& other){
    //TODO
}

template<typename State, typename Symbol>
GeneralizedBuchiAutomaton<State, Symbol> GeneralizedBuchiAutomaton<State,Symbol>::reverseGBA(){
    //TODO
}