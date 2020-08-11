
#include <iostream>
#include <set>
#include <map>
#include <fstream>

#include "units-config.h"
#include "../BuchiAutomaton.h"
#include "../Simulations.h"
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

    BuchiAutomaton<int, int> ren = ba.renameAut();
    BuchiAutomatonSpec sp(ren);
    BuchiAutomaton<StateSch, int> comp = sp.complementSch();

    cout << comp.getStates().size() << endl;
  }
  os.close();
  return 0;
}
