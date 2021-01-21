
#include "AutGraph.h"

/*
 * Strongly connected components from a given vertex number (implementing
 * the Tarjan's algorithm)
 */
void AutGraph::strongConnect(int v)
{
  this->vertices[v].index = this->index;
  this->vertices[v].lowLink = this->index;
  this->index++;
  this->S.push(v);
  this->vertices[v].onStack = true;

  for(int w : this->adjList[v])
  {
    if(this->vertices[w].index == -1)
    {
      strongConnect(w);
      this->vertices[v].lowLink = std::min(this->vertices[v].lowLink,
          this->vertices[w].lowLink);
    }
    else if(this->vertices[w].onStack)
    {
      this->vertices[v].lowLink = std::min(this->vertices[v].lowLink, this->vertices[w].index);
    }
  }

  if(this->vertices[v].lowLink == this->vertices[v].index)
  {
    set<int> scc;
    bool final = false;
    int w;
    do {
      w = this->S.top();
      this->S.pop();
      this->vertices[w].onStack = false;
      scc.insert(this->vertices[w].label);
      if(!final && this->finals.find(this->vertices[w].label) != this->finals.end())
      {
        final = true;
      }
    } while(v != w);
    if(final && scc.size() >= 1)
    {
      auto item = scc.begin();
      if(scc.size() == 1)
      {
        auto it = std::find(this->adjList[*item].begin(), this->adjList[*item].end(), *item);
        if(it != this->adjList[*item].end())
          this->finalComponents.push_back(scc);
      }
      else
      {
        this->finalComponents.push_back(scc);
      }
    }
    this->allComponents.push_back(scc);
  }
}


/*
 * Compute all strongly connected components (SCCs)
 */
void AutGraph::computeSCCs()
{
  this->index = 0;
  this->S = stack<int>();
  this->finalComponents.clear();
  this->allComponents.clear();
  for(VertItem v : this->vertices)
  {
    if(v.index == -1)
    {
      this->strongConnect(v.label);
    }
  }
}


/*
 * Get all reachable vertices from a set of vertices
 * @param from Set of starting vertices
 * @return Set of reachable vertices
 */
set<int> AutGraph::reachableVertices(set<int>& from)
{
  return reachableVertices(this->adjList, from);
}


/*
 * Get all reachable vertices from a set of vertices
 * @param lst List of adjacent vertices
 * @param from Set of starting vertices
 * @return Set of reachable vertices
 */
set<int> AutGraph::reachableVertices(AdjList &lst, set<int>& from)
{
  set<int> all(from);
  stack<int> stack;
  int item;
  for(int it : all)
    stack.push(it);


  while(stack.size() > 0)
  {
    item = stack.top();
    stack.pop();
    for(int dst : lst[item])
    {
      if(all.find(dst) == all.end())
      {
        stack.push(dst);
        all.insert(dst);
      }
    }
  }
  return all;
}
