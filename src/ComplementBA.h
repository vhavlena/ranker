
#ifndef _COMPLEMENT_BA_H_
#define _COMPLEMENT_BA_H_

#include <set>
#include <map>
#include <string>

template <typename State>
struct StateKV {
  std::set<State> S;
  std::set<State> O;
  std::map<State, int> f;

  bool operator <(const StateKV<State>& rhs) const
  {
    return S < rhs.S && O < rhs.O && f < rhs.f;
  }

  std::string toString()
  {
    std::string ret = "({" + printSet(S) + "}, {";
    ret += printSet(O) + "}, {";
    for (auto p : f)
      ret += std::to_string(p.first) + ":" + std::to_string(p.second) + " ";
    if(ret.back() == ' ')
      ret.pop_back();
    ret += "})";
    return ret;
  }

  std::string printSet(std::set<State> st)
  {
    std::string ret;
    for (auto s : st)
      ret += std::to_string(s) + " ";
    if(ret.back() == ' ')
      ret.pop_back();
    return ret;
  }
};



#endif
