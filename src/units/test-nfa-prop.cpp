
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <algorithm>
#include "../BuchiAutomaton.h"
#include "../BuchiAutomatonSpec.h"
#include "../BuchiAutomataParser.h"

using namespace std;

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
    BuchiAutomaton<int, int> ren = ba.renameAut();
    BuchiAutomatonSpec sp(ren);
    BuchiAutomaton<StateSch, int> comp = sp.complementSchNFA(sp.getInitials());
    set<StateSch> slIgnore = sp.nfaSlAccept(comp);

    cout << "Rank bound: " << endl;
    for(auto t : sp.getRankBound(comp, slIgnore))
    {
      cout << t.first.toString() << " : " << t.second << endl;
    }

    cout << endl << "Max reach size: " << endl;
    for(auto t : sp.getMaxReachSize(comp, slIgnore))
    {
      cout << t.first.toString() << " : " << t.second << endl;
    }

    cout << endl << "Min reach size: " << endl;
    for(auto t : sp.getMinReachSize())
    {
      cout << t.first.toString() << " : " << t.second << endl;
    }

    cout << endl << "Cycle closing states: " << endl;
    for(auto t : comp.getCycleClosingStates(slIgnore))
    {
      cout << t.toString() << endl;
    }
  }
  os.close();
  return 0;
}
