
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
using std::pair;

/*
 * Vertex with additional information for Tarjan
 */
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
  map<int, set<int>> finalsGBA;
  Vertices vertices;
  SCCs finalComponents;
  SCCs allComponents;

  int index;
  stack<int> S;

protected:
  void strongConnect(int v);
  void strongConnect(int v, map<int, set<int>> finals, bool coBuchi, std::vector<std::vector<int>> allCycles);

public:
  AutGraph(AdjList& adj, Vertices& vert, set<int>& finals)
  {
    this->adjList = adj;
    this->vertices = vert;
    this->finals = finals;
  }

  AutGraph(AdjList &adj, Vertices &vert, map<int, set<int>> finalsGBA)
  {
    this->adjList = adj;
    this->vertices = vert;
    this->finalsGBA = finalsGBA;
  }

  void computeSCCs();
  void computeSCCs(set<pair<int, int> >& accTrans);
  void computeSCCs(map<int, set<int>> finals, bool coBuchi, std::vector<std::vector<int>> alCycles);
  set<int> reachableVertices(set<int>& from);
  static set<int> reachableVertices(AdjList &lst, set<int>& from);

  /*
   * Get SCCs containing at least one final state
   * @return Vector of SCCs (set of vertices)
   */
  SCCs& getFinalComponents()
  {
    return this->finalComponents;
  }

  /*
   * Get all SCCs
   * @return Vector of SCCs (set of vertices)
   */
  SCCs& getAllComponents()
  {
    return this->allComponents;
  }
};

#endif
