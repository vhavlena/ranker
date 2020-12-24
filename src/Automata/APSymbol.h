#ifndef AP_SYMBOL_H_
#define AP_SYMBOL_H_

#include <boost/dynamic_bitset.hpp>

//Bitarray representing a set of APs
struct APSymbol
{
  boost::dynamic_bitset<> ap;

  APSymbol() : ap() {};
  APSymbol(int cnt) : ap(cnt) {};

  bool operator==(const APSymbol other) const
  {
    return ap == other.ap;
  }

  bool operator<(const APSymbol other) const
  {
    return ap < other.ap;
  }
};

#endif
