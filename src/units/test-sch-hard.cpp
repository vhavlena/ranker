
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <algorithm>

#include "units-config.h"
#include "../Automata/BuchiAutomaton.h"
#include "../Algorithms/Simulations.h"
#include "../Complement/BuchiAutomatonSpec.h"
#include "../Automata/BuchiAutomataParser.h"
#include "../Algorithms/AuxFunctions.h"

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
  string filename(argv[1]);
  os.open(argv[1]);

  if(os)
  {
    BuchiAutomaton<string, string> ba = parser.parseBaFormat(os);
    string cmd = "java -jar " + RABITEXE + " " + filename + " " + filename + " -dirsim";
    Simulations sim;
    istringstream strr(Simulations::execCmd(cmd));
    ba.setDirectSim(sim.parseRabitRelation(strr));
    auto cl = set<std::string>();
    ba.computeRankSim(cl);

    BuchiAutomaton<int, int> ren = ba.renameAut();
    BuchiAutomatonSpec sp(ren);
    BuchiAutomaton<StateSch, int> comp = sp.complementSchNFA(sp.getInitials());
    set<StateSch> slIgnore = sp.nfaSlAccept(comp);

    auto maxReach = sp.getMaxReachSize(comp, slIgnore);
    auto minReach = sp.getMinReachSize();
    auto maxReachInd = sp.getMaxReachSizeInd();

    auto slnoAccept = sp.nfaSingleSlNoAccept(comp);
    set<StateSch> ignoreAll;
    for(const auto& t : slnoAccept)
      ignoreAll.insert({t.first, set<int>(), RankFunc(), 0, false});
    ignoreAll.insert(slIgnore.begin(), slIgnore.end());

    auto rankBound = sp.getRankBound(comp, ignoreAll, maxReach, minReach);

    bool suit = true;
    for(auto t : comp.getCycleClosingStates(slIgnore))
    {
      if((t.S.size() >= 13 && rankBound[t.S] >= 3) || (t.S.size() >= 10 && rankBound[t.S] >= 4) ||
        (t.S.size() >= 9 && rankBound[t.S] >= 5) || (t.S.size() >= 8 && rankBound[t.S] >= 6))
      {
        suit = false;
        break;
      }
    }

    cout << std::boolalpha;
    cout << suit << endl;

  }
  os.close();
  return 0;
}
