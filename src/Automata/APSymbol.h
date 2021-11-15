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
  // //Bitarray representing a set of APs
  // boost::dynamic_bitset<> ap;

  std::vector<char> ap;

  APSymbol() : ap() {};
  APSymbol(int cnt) : ap(cnt, 0) {};
  APSymbol(int cnt, char val) : ap(cnt, val) {};

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
      if(ap[i] == 1)
      {
        ret += std::to_string(i);
        ret += "&";
      }
      else if(ap[i] == 0)
      {
        ret += "!" + std::to_string(i);
        ret += "&";
      }

      // if(i + 1 < ap.size())
      //   ret += "&";
    }
    if(ret.back() == '&')
      ret.pop_back();

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
