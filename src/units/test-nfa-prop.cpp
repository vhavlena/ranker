
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include "../BuchiAutomaton.h"
#include "../BuchiAutomatonSpec.h"
#include "../BuchiAutomataParser.h"

using namespace std;

set<StateSch> slIgnore;

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
  return act.S.size();
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
    BuchiAutomaton<int, int> ren = ba.renameAut();
    BuchiAutomatonSpec sp(ren);
    BuchiAutomaton<StateSch, int> comp = sp.complementSchNFA(sp.getInitials());

    //cout << comp.toGraphwiz() << endl;
    slIgnore = sp.nfaSlAccept(comp);

    auto prval = comp.propagateGraphValues(updMaxFnc, initMaxFnc);
    for(auto& tmp : prval)
    {
      cout << tmp.state.toString() << ": " << tmp.label << std::endl;
    }

    // cout << comp.toString() << endl;
    cout << comp.toGraphwiz() << endl;
    cout << comp.getStates().size() << endl;
  }
  os.close();
  return 0;
}
