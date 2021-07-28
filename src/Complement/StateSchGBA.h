
#ifndef _STATE_SCH_GBA_H_
#define _STATE_SCH_GBA_H_

#include <set>
#include <map>
#include <string>

#include "RankFunc.h"

/*
 * State of the Schewe construction
 */
struct StateSchGBA {
  std::set<int> S;
  std::set<int> O;
  RankFunc f;
  int i;
  int j;
  bool tight;

  bool operator <(const StateSchGBA& rhs) const
  {
    if(tight == rhs.tight && tight)
    {
      if(i == rhs.i)
      {
        if(O == rhs.O)
        {
          return f < rhs.f;
        }
        return O < rhs.O;
      }
      else
      return i < rhs.i;
    }
    else if(tight == rhs.tight)
    {
      return S < rhs.S;
    }
    return tight < rhs.tight;
  }

  bool operator ==(const StateSchGBA& rhs) const
  {
    return S == rhs.S && O == rhs.O && f == rhs.f && i == rhs.i && tight == rhs.tight;
  }

  std::string toString() const
  {
    if(!tight)
      return StateSchGBA::printSet(S);
    std::string ret = "(" + StateSchGBA::printSet(S) + "," + StateSchGBA::printSet(O) + ",";
    ret += f.toString();
    ret += "," + std::to_string(i) + ")";
    return ret;
  }

  static std::string printSet(std::set<int> st)
  {
    std::string ret;
    for (auto s : st)
      ret += std::to_string(s) + " ";
    if(ret.back() == ' ')
      ret.pop_back();
    return "{" + ret + "}";
  }
};


#endif
