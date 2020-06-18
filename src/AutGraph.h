
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


class AutGraph
{
public:
  typedef set<int> SCC;
  typedef vector<vector<int> > AdjList;
  typedef vector<VertItem> Vertices;

private:
  AdjList adjList;
  set<int> finals;
  Vertices vertices;
  vector<SCC> finalComponents;

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

  void computeFinalSCCs();
  set<int> reachableVertices(set<int>& from);
  set<int> reachableVertices(AdjList &lst, set<int>& from);

  vector<SCC>& getFinalComponents()
  {
    return this->finalComponents;
  }
};

#endif
