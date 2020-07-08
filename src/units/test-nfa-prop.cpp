
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <algorithm>
#include "../BuchiAutomaton.h"
#include "../BuchiAutomatonSpec.h"
#include "../BuchiAutomataParser.h"

using namespace std;

set<StateSch> slIgnore;
BuchiAutomaton<int, int> ren;

int updMaxFnc(LabelState<StateSch>* a, const std::vector<LabelState<StateSch>*> sts)
{
  int m = 0;
  //std::cout << a->state.toString() << endl;
  for(const LabelState<StateSch>* tmp : sts)
  {
    //cout << ": " << tmp->state.toString() << std::endl;
    if(tmp->state.S == a->state.S && slIgnore.find(a->state) != slIgnore.end())
      continue;
    m = std::max(m, tmp->label);
  }
  return std::min(a->label, m);
}


int initMaxFnc(const StateSch& act)
{
  set<int> ret;
  set<int> fin = ren.getFinals();
  std::set_difference(act.S.begin(),act.S.end(),fin.begin(),
    fin.end(), std::inserter(ret, ret.begin()));
  if(ren.containsRankSimEq(ret))
    return std::max((int)ret.size() - 1, 0);
  return ret.size();
}


int main(int argc, char *argv[])
{
  BuchiAutomataParser parser;
  ifstream os;

  if(argc != 2)
  {
    cerr << "Bad arguments" << endl;
    return 1;
  }
  os.open(argv[1]);
  cout << argv[1] << endl;

  if(os)
  {
    BuchiAutomaton<string, string> ba = parser.parseBaFormat(os);
    ba.setDirectSim({{"[10]", "[8]"}, {"[5]", "[3]"}, {"[11]", "[8]"}, {"[11]", "[7]"}, {"[11]", "[6]"}, {"[12]", "[3]"}, {"[3]", "[3]"}});
    ren = ba.renameAut();
    BuchiAutomatonSpec sp(ren);
    BuchiAutomaton<StateSch, int> comp = sp.complementSchNFA(sp.getInitials());

    for(auto t : comp.getAutGraphSCCs())
    {
      cout << "-------" << endl;
      for(auto s : t)
      {
        cout << " " << s.toString() << endl;
      }
    }

    slIgnore = sp.nfaSlAccept(comp);
    auto prval = comp.propagateGraphValues(updMaxFnc, initMaxFnc);

    auto sls = comp.getSelfLoops();
    for(auto t : comp.getEventReachable(sls))
    {
      cout << " : " << t.toString() << " : " << prval[t] << endl;
    }

    //cout << comp.toGraphwiz() << endl;


    //auto prval = comp.propagateGraphValues(updMaxFnc, initMaxFnc);
    for(auto& tmp : prval)
    {
      cout << tmp.first.toString() << ": " << tmp.second << std::endl;
    }

    // cout << comp.toString() << endl;
    //cout << comp.toGraphwiz() << endl;
    cout << comp.getStates().size() << endl;
  }
  os.close();
  return 0;
}
