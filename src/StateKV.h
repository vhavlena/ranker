
#ifndef _STATE_KV_H_
#define _STATE_KV_H_

#include <set>
#include <map>
#include <string>

#include "RankFunc.h"

struct StateKV {
  std::set<int> S;
  std::set<int> O;
  RankFunc f;

  bool operator <(const StateKV& rhs) const
  {
    if(S == rhs.S)
    {
      if(O == rhs.O)
      {
        return f < rhs.f;
      }
      return O < rhs.O;
    }
    else
    return S < rhs.S;
  }

  bool operator ==(const StateKV& rhs) const
  {
    return S == rhs.S && O == rhs.O && f == rhs.f;
  }

  std::string toString()
  {
    std::string ret = "({" + printSet(S) + "},{";
    ret += printSet(O) + "},";
    ret += f.toString();
    ret += ")";
    return ret;
  }

  std::string printSet(std::set<int> st)
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
