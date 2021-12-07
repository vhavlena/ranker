
#ifndef _STATE_SD_H_
#define _STATE_SD_H_

#include <set>
#include <map>
#include <string>

/*
 * State of the Schewe construction
 */
struct StateSD {
  std::set<int> N;
  std::set<int> C;
  std::set<int> S;
  std::set<int> B;

  bool operator <(const StateSD& rhs) const
  {
      return N < rhs.N;
  }

  bool operator ==(const StateSD& rhs) const
  {
    return N == rhs.N and C == rhs.C and S == rhs.S and B == rhs.B;
  }

  std::string toString() const
  {
    std::string ret = "(" + StateSD::printSet(N) + "," + StateSD::printSet(C) + "," + StateSD::printSet(S) + "," + StateSD::printSet(B) + ")";
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
