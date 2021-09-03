#ifndef AP_SYMBOL_H_
#define AP_SYMBOL_H_

#include <boost/dynamic_bitset.hpp>
#include <vector>
#include <string>

using std::string;

/*
 * Type representing an item of AP set
 */
struct APSymbol
{
  //Bitarray representing a set of APs
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
};


class APWord : public std::vector<APSymbol>
{
public:
  string toString() const
  {
    string ret;
    for(const auto& it : *this)
      ret += it.toString() + "; ";
    return ret;
  }

  std::vector<APSymbol> getVector()
  {
    return (std::vector<APSymbol>)(*this);
  }
};

#endif
