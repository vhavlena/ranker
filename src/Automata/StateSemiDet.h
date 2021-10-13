
#ifndef _STATE_SEMIDET_H_
#define _STATE_SEMIDET_H_

#include <set>
#include <map>
#include <string>

#include "../Algorithms/AuxFunctions.h"


/*
 * State of the Semideterministic automaton
 */
struct StateSemiDet {
  int waiting;
  std::pair<set<int>, set<int>> tight;
  bool isWaiting;

  std::string toString() const
  {
    if(isWaiting)
      return std::to_string(waiting);
    std::string ret = "(" + Aux::printIntSet(tight.first) + "," + Aux::printIntSet(tight.second) + ")";
    return ret;
  }

  bool operator<(const StateSemiDet& rhs) const
  {
    if(isWaiting == rhs.isWaiting && !isWaiting)
    {
      if(tight.first == rhs.tight.first)
      {
        return tight.second < rhs.tight.second;
      }
      else
      return tight.first < rhs.tight.first;
    }
    else if(isWaiting == rhs.isWaiting)
    {
      return waiting < rhs.waiting;
    }
    return isWaiting < rhs.isWaiting;
  }

  bool operator ==(const StateSemiDet& rhs) const
  {
    return tight == rhs.tight && waiting == rhs.waiting && isWaiting == rhs.isWaiting;
  }
};



#endif
