
#ifndef AUX_FNC_H_
#define AUX_FNC_H_

#include <set>
#include <map>
#include <vector>
#include <string>
#include <algorithm>
#include <cassert>

using namespace std;

namespace Aux
{
  int countEqClasses(int n, set<int>& st, set<pair<int, int>>& rel);
  vector< vector<int> > getAllSubsets(vector<int> set);
  vector< vector<int> > getAllSubsets(vector<int> set, unsigned max);
  string printVector(vector<int> st);

  /*
   * Cartesian product of two n-ary relations
   * @param s1 First vector
   * @param s2 Second vector
   * @return Cartesian product of s1 and s2
   */
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


  /*
   * Cartesian product of a vector of n-ary relations
   * @param slist Vector of relations
   * @return Cartesian product of s1 and s2
   */
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


  /*
   * Apply mapping on all items of a set
   * @param mp Mapping to be applied
   * @param st Set of elements to be transformed
   * @return mp(st)
   */
  template <typename T, typename S>
  std::set<S> mapSet(std::map<T, S>& mp, std::set<T>& st)
  {
    std::set<S> ret;
    for(const auto& p : st)
      ret.insert(mp[p]);
    return ret;
  }

  template<typename T, typename S>
  std::map<int, std::set<S>> mapMap(std::map<T, S> &mp, std::map<int, std::set<T>> &st){
    std::map<int, std::set<S>> ret;
    for (unsigned i = 0; i < st.size(); i++){
      std::set<int> emptySet;
      ret.insert({i, emptySet});
      auto it = ret.find(i);
      for (const auto& p : st[i]){
        it->second.insert(mp[p]);
      }
    }
    return ret;
  }


  /*
   * Reverse a given mapping (assume it is a bijection)
   * @param mp Mapping to be reversed
   * @return Reversed map (switched key, value pairs)
   */
  template<typename K, typename V>
  map<V, K> reverseMap(map<K, V>& mp)
  {
    map<V, K> rev;
    for(const auto &t : mp)
    {
      rev.insert({t.second, t.first});
    }
    return rev;
  }


  template<typename K>
  int maxValue(map<K, int>& mp)
  {
    assert(mp.size() > 0);

    auto pr = std::max_element(mp.begin(), mp.end(),
      [] (const auto & p1, const auto & p2) {
          return p1.second < p2.second;
      }
    );
    return pr->second;
  }

  std::string printIntSet(std::set<int> st);


  template<typename State>
  set<pair<State,State>> inverseRelation(set<pair<State,State>>& rel)
  {
    set<pair<State,State>> rt;
    for(const auto& item : rel)
    {
      rt.insert({item.second, item.first});
    }
    return rt;
  }


  template<typename State>
  set<set<State>> getEqClasses(set<pair<State,State>>& rel, set<State>& univ)
  {
    map<State, int> clId;
    map<State, int*> cl;
    int i = 0;
    for(const State& st : univ)
    {
      clId[st] = i++;
      cl[st] = &(clId[st]);
    }

    set<pair<State,State>> inv = Aux::inverseRelation(rel);
    set<pair<State,State>> intersect;
    set_intersection(inv.begin(), inv.end(), rel.begin(), rel.end(),
      std::inserter(intersect, intersect.begin()));
    for(const auto& pr : intersect)
    {
      cl[pr.first] = cl[pr.second];
      clId[pr.second] = clId[pr.first];
    }

    vector<set<State>> vc(univ.size());
    set<set<State>> ret;
    for(const auto& c : cl)
    {
      vc[*(c.second)].insert(c.first);
    }
    for(const set<State>& sts : vc)
    {
      if(sts.size() > 0)
        ret.insert(sts);
    }

    return ret;
  }

}

#endif
