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
  for (auto it = this->finals.begin(); it != this->finals.end(); it++){
    for(State p : it->second)
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

template class GeneralizedBuchiAutomaton<int, int>;
template class GeneralizedBuchiAutomaton<std::string, std::string>;
template class GeneralizedBuchiAutomaton<StateSch, int>;
template class GeneralizedBuchiAutomaton<int, APSymbol>;
template class GeneralizedBuchiAutomaton<StateSch, APSymbol>;