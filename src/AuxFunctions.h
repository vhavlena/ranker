
#ifndef AUX_FNC_H_
#define AUX_FNC_H_

#include <set>
#include <vector>

using namespace std;

namespace Aux
{
  int countEqClasses(int n, set<int>& st, set<pair<int, int>>& rel);
  vector< vector<int> > getAllSubsets(vector<int> set);

  template <typename T>
  std::set<std::vector<T> > cartProduct(std::set<std::vector<T> > s1, std::set<std::vector<T> > s2)
  {
    std::set<std::vector<T> > ret;
    for(auto v1 : s1)
    {
      for(auto v2 : s2)
      {
        std::vector<T> tmp(v1);
        tmp.insert(tmp.end(), v2.begin(), v2.end());
        ret.insert(tmp);
      }
    }
    return ret;
  }


  template <typename T>
  std::set<std::vector<T> > cartProductList(std::vector<std::set<std::vector<T> > > slist)
  {
    std::set<std::vector<T> > ret;
    if(slist.size() == 0)
      return ret;

    ret = slist[0];
    for(int i = 1; i < slist.size(); i++)
    {
      ret = cartProduct<T>(ret, slist[i]);
    }
    return ret;
  }

}

#endif
