#include "BuchiAutomatonDebug.h"


/*
 * Create a BA accepting the single word handle.(loop)^omega.
 * @param handle Prefix of the word
 * @param loop Infinitely periodic part (lasso).
 * @return BA
 */
template <typename State, typename Symbol>
BuchiAutomaton<int, Symbol> BuchiAutomatonDebug<State, Symbol>::createWordAutomaton(vector<Symbol>& handle, vector<Symbol>& loop)
{
  Delta<int, Symbol> trans;
  set<int> ini = {0};
  set<int> state = {0};
  int d = handle.size();

  if(loop.empty())
  {
    throw BuchiAutomatonException("Empty lasso of the word");
  }

  for(int i = 0; i < (int)handle.size(); i++)
  {
    trans[{i, handle[i]}] = set<int>({i+1});
    state.insert(i+1);
  }

  for(int i = 0; i < (int)loop.size() - 1; i++)
  {
    trans[{i+d, loop[i]}] = set<int>({d+i+1});
    state.insert(d+i+1);
  }

  state.insert(d+loop.size()-1);
  trans[{d+loop.size()-1, loop.back()}] = set<int>({d});

  return BuchiAutomaton<int, Symbol>(state, state, ini, trans);
}


/*
 * Restrict BA on a single untimately periodic word.
 * @param handle Prefix of the word
 * @param loop Infinitely periodic part (lasso).
 * @return Product of the word and the BA.
 */
template <typename State, typename Symbol>
BuchiAutomaton<pair<State,int>, Symbol> BuchiAutomatonDebug<State, Symbol>::getSubAutomatonWord(vector<Symbol> handle, vector<Symbol> loop)
{
  BuchiAutomaton<int, Symbol> wordBA = createWordAutomaton(handle, loop);
  return this->cartProductBA(wordBA);
}
