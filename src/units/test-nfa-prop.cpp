
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include "../BuchiAutomaton.h"
#include "../BuchiAutomatonSpec.h"
#include "../BuchiAutomataParser.h"

using namespace std;

int updMaxFnc(LabelState<StateSch>& a, std::set<LabelState<StateSch>*>& sts)
{
  int m = a.label;
  //std::cout << a.state.toString() << endl;
  for(LabelState<StateSch>* tmp : sts)
  {
    cout << tmp->state.toString() << std::endl;
    // if(tmp->state.S == a.state.S)
    //   continue;
    m = std::max(m, tmp->label);
  }
  return std::min(a.label, m);
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
