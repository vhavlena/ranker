
#include "AuxFunctions.h"

namespace Aux
{

int countEqClasses(int n, set<int>& st, set<pair<int, int>>& rel)
{
  vector<int> cl(n);
  set<pair<int, int>> relprime;
  for(int i = 0; i < cl.size(); i++)
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
      for(int i = 0; i < cl.size(); i++)
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


vector< vector<int> > getAllSubsets(vector<int> set)
{
  vector< vector<int> > subset;
  vector<int> empty;
  subset.push_back( empty );

  for (int i = 0; i < set.size(); i++)
  {
    vector< vector<int> > subsetTemp = subset;
    for (int j = 0; j < subsetTemp.size(); j++)
      subsetTemp[j].push_back( set[i] );

    for (int j = 0; j < subsetTemp.size(); j++)
      subset.push_back( subsetTemp[j] );
  }
  return subset;
}

}
