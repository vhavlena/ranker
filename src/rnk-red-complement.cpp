
#include <cstdlib>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <chrono>
#include <iomanip>

#include "Compl-config.h"
#include "BuchiAutomaton.h"
#include "Simulations.h"
#include "BuchiAutomatonSpec.h"
#include "BuchiAutomataParser.h"
#include "Options.h"

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
    const char* rabitpath_cstr = std::getenv("RABITEXE");
    std::string rabitpath = (nullptr == rabitpath_cstr)? RABITEXE : rabitpath_cstr;

    BuchiAutomaton<string, string> ba = parser.parseBaFormat(os);
    string cmd = "java -jar " + rabitpath + " " + filename + " " + filename + " -dirsim";
    Simulations sim;
    istringstream strr(Simulations::execCmd(cmd));

    ba.setDirectSim(sim.parseRabitRelation(strr));
    auto cl = set<std::string>();
    auto t1 = std::chrono::high_resolution_clock::now();

    ba.computeRankSim(cl);
    BuchiAutomaton<int, int> ren = ba.renameAut();
    BuchiAutomatonSpec sp(ren);
    ComplOptions opt = { .cutPoint = false, .succEmptyCheck = true };
    sp.setComplOptions(opt);
    BuchiAutomaton<StateSch, int> comp;
    try
    {
      comp = sp.complementSchReduced();
    }
    catch (const std::bad_alloc&)
    {
      os.close();
      cerr << "Memory error" << endl;
      return 2;
    }

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    cout << "Generated states: " << comp.getStates().size() << "\nGenerated trans: " << comp.getTransitions().size() << endl;

    map<int, int> id;
    for(auto al : comp.getAlphabet())
      id[al] = al;

    BuchiAutomaton<int, int> renCompl = comp.renameAutDict(id);
    renCompl.removeUseless();
    cout << "States: " << renCompl.getStates().size() << "\nTransitions: " << renCompl.getTransitions().size() << endl;

    cout << std::fixed;
    cout << std::setprecision(2);
    cout << "Time: " << (float)(duration/1000.0) << std::endl;
  }
  os.close();
  return 0;
}
