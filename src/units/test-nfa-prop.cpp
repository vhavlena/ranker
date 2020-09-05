
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
  cout << argv[1] << endl;

  if(os)
  {
    BuchiAutomaton<string, string> ba = parser.parseBaFormat(os);
    string cmd = "java -jar " + RABITEXE + " " + filename + " " + filename + " -dirsim";
    Simulations sim;
    istringstream strr(Simulations::execCmd(cmd));
    ba.setDirectSim(sim.parseRabitRelation(strr));
    auto cl = set<std::string>();
    ba.computeRankSim(cl);

    cout << "State rename map " << endl;
    BuchiAutomaton<int, int> ren = ba.renameAut();
    for(auto t : ba.getRenameStateMap())
      std::cout << t.first << " = " << t.second << std::endl;

    cout << "Original BA (renamed): " << endl;
    cout << ren.toGraphwiz() << endl;
    BuchiAutomatonSpec sp(ren);
    BuchiAutomaton<StateSch, int> comp = sp.complementSchNFA(sp.getInitials());
    set<StateSch> slIgnore = sp.nfaSlAccept(comp);

    auto maxReach = sp.getMaxReachSize(comp, slIgnore);
    auto minReach = sp.getMinReachSize();
    auto maxReachInd = sp.getMaxReachSizeInd();

    cout << "Rank bound: " << endl;
    for(auto t : sp.getRankBound(comp, slIgnore, maxReach, minReach))
    {
      cout << StateSch::printSet(t.first) << " : " << t.second << endl;
    }

    auto slnoAccept = sp.nfaSingleSlNoAccept(comp);
    set<StateSch> ignoreAll;
    for(const auto& t : slnoAccept)
      ignoreAll.insert({t.first, set<int>(), RankFunc(), 0, false});
    ignoreAll.insert(slIgnore.begin(), slIgnore.end());
    cout << endl << "Rank bound (without a/n single-letter self-loops): " << endl;
    for(auto t : sp.getRankBound(comp, ignoreAll, maxReach, minReach))
    {
      cout << StateSch::printSet(t.first) << " : " << t.second << endl;
    }

    cout << endl << "Max reach size: " << endl;
    for(auto t : maxReach)
    {
      cout << StateSch::printSet(t.first) << " : " << t.second << endl;
    }

    cout << endl << "Min reach size: " << endl;
    for(auto s : sp.getStates())
    {
      cout << s << " : " << minReach[s] << " : " << maxReachInd[s] << endl;
    }

    cout << endl << "Cycle closing states: " << endl;
    for(auto t : comp.getCycleClosingStates(slIgnore))
    {
      cout << t.toString() << endl;
    }

    auto cl2 = set<int>();
    sp.computeRankSim(cl2);
    cout << endl << "Rank simulation: " << endl;
    for(auto t : sp.getOddRankSim())
    {
      cout << t.first << " " << t.second << endl;
    }

    cout << endl << "Direct simulation: " << endl;
    for(auto t : sp.getDirectSim())
    {
      cout << t.first << " " << t.second << endl;
    }

    cout << endl << "Final states: " << endl;
    for(auto t : sp.getFinals())
    {
      cout << t << endl;
    }
  }
  os.close();
  return 0;
}
