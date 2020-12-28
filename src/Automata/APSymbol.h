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

  string toString() const
  {
    string ret;
    for(unsigned int i = 0; i < ap.size(); i++)
    {
      if(ap[i])
        ret += std::to_string(i);
      else
        ret += "!" + std::to_string(i);
      if(i + 1 < ap.size())
        ret += "&";
    }
    return ret;
  }

  friend std::ostream& operator<<(std::ostream& os, const APSymbol& symb)
  {
    os << symb.toString();
    return os;
  }
};

#endif
