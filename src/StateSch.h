
#ifndef _STATE_SCH_H_
#define _STATE_SCH_H_

#include <set>
#include <map>
#include <string>

template <typename State>
struct StateSch {
  std::set<State> S;
  std::set<State> O;
  std::map<State, int> f;
  int i;
  int maxRank;
  bool tight;

  bool operator <(const StateSch<State>& rhs) const
  {
    if(tight == rhs.tight && tight)
    {
      if(S == rhs.S)
      {
        if(O == rhs.O)
        {
          if(f == rhs.f)
          {
            return i < rhs.i;
          }
          return f < rhs.f;
        }
        return O < rhs.O;
      }
      else
      return S < rhs.S;
    }
    else if(tight == rhs.tight)
    {
      return S < rhs.S;
    }
    return tight < rhs.tight;
  }

  bool operator ==(const StateSch<State>& rhs) const
  {
    return S == rhs.S && O == rhs.O && f == rhs.f && i == rhs.i;
  }

  std::string toString()
  {
    std::string ret = "({" + printSet(S) + "}, {";
    ret += printSet(O) + "}, {";
    for (auto p : f)
      ret += std::to_string(p.first) + ":" + std::to_string(p.second) + " ";
    if(ret.back() == ' ')
      ret.pop_back();
    ret += "}" + std::to_string(i) + ")";
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
