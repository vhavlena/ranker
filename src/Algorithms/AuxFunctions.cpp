
#include "AuxFunctions.h"

namespace Aux
{
/*
 * Count equivalence classes in the maximal equivalence fragment of a give relation.
 * @param n Number of elements of a set
 * @param st Set of states
 * @return Number of classes
 */
int countEqClasses(int n, set<int>& st, set<pair<int, int>>& rel)
{
  vector<int> cl(n);
  set<pair<int, int>> relprime;
  for(unsigned i = 0; i < cl.size(); i++)
  {
    cl[i] = i;
  }

  for(auto& t : rel)
  {
    if(st.find(t.first) != st.end() && st.find(t.second) != st.end())
      relprime.insert(t);
  }

  for(const auto& t : relprime)
  {
    if(relprime.find({t.second, t.first}) != relprime.end())
    {
      int fnd = cl[t.second];
      int rpl = cl[t.first];
      for(unsigned i = 0; i < cl.size(); i++)
      {
        if(cl[i] == fnd)
          cl[i] = rpl;
      }
    }
  }

  set<int> ret;
  for(auto s : st)
  {
    ret.insert(cl[s]);
  }
  return ret.size();
}


/*
 * Get all subsets of a given vector
 * @param set Set represented as a vector
 * @return All subsets
 */
vector< vector<int> > getAllSubsets(vector<int> set)
{
  vector< vector<int> > subset;
  vector<int> empty;
  subset.push_back( empty );

  for (unsigned i = 0; i < set.size(); i++)
  {
    vector< vector<int> > subsetTemp = subset;
    for (unsigned j = 0; j < subsetTemp.size(); j++)
      subsetTemp[j].push_back( set[i] );

    for (unsigned j = 0; j < subsetTemp.size(); j++)
      subset.push_back( subsetTemp[j] );
  }
  return subset;
}


/*
 * Convert to string the contents of a given vector
 * @param st Vector to be converted to string
 * @return String representation
 */
string printVector(vector<int> st)
{
  string ret;
  for (auto s : st)
    ret += std::to_string(s) + " ";
  if(ret.back() == ' ')
    ret.pop_back();
  return "{" + ret + "}";
}

}
