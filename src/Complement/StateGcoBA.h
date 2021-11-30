
#ifndef _STATE_GCOBA_H_
#define _STATE_GCOBA_H_

#include <set>
#include <map>
#include <string>

/*
 * State of the KV construction
 */
struct StateGcoBA {
  std::set<int> S;
  std::set<int> B;
  int i;

  bool operator <(const StateGcoBA& rhs) const
  {
    if (i != rhs.i)
      return i < rhs.i;
    else {
      if (S != rhs.S)
        return S < rhs.S;
      else
        return B < rhs.B;
    }
  }
  
  bool operator ==(const StateGcoBA& rhs) const
  {
    return S == rhs.S && B == rhs.B && i == rhs.i;
  }

  std::string toString()
  {
    std::string ret = "({" + printSet(S) + "},{";
    ret += printSet(B) + "},";
    ret += std::to_string(i);
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
