
#ifndef _AUT_GRAPH_H_
#define _AUT_GRAPH_H_

#include <set>
#include <map>
#include <string>
#include <vector>
#include <stack>
#include <iostream>
#include <algorithm>

using std::set;
using std::map;
using std::vector;
using std::string;
using std::stack;

struct VertItem {
  int label;
  int index;
  int lowLink;
  bool onStack;
};

typedef set<int> SCC;
typedef vector<SCC> SCCs;
typedef vector<vector<int> > AdjList;
typedef vector<VertItem> Vertices;

class AutGraph
{

private:
  AdjList adjList;
  set<int> finals;
  Vertices vertices;
  SCCs finalComponents;
  SCCs allComponents;

  int index;
  stack<int> S;

protected:
  void strongConnect(int v);

public:
  AutGraph(AdjList& adj, Vertices& vert, set<int>& finals)
  {
    this->adjList = adj;
    this->vertices = vert;
    this->finals = finals;
  }

  void computeSCCs();
  set<int> reachableVertices(set<int>& from);
  static set<int> reachableVertices(AdjList &lst, set<int>& from);

  SCCs& getFinalComponents()
  {
    return this->finalComponents;
  }

  SCCs& getAllComponents()
  {
    return this->allComponents;
  }
};

#endif
