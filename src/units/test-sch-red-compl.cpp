
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <chrono>
#include <iomanip>
#include "../BuchiAutomaton.h"
#include "../Simulations.h"
#include "../BuchiAutomatonSpec.h"
#include "../BuchiAutomataParser.h"

using namespace std;

string RABITEXE = "";
string TMPNAME = "tmp124232.ba";

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
    auto t1 = std::chrono::high_resolution_clock::now();

    ba.computeRankSim(cl);
    BuchiAutomaton<int, int> ren = ba.renameAut();
    BuchiAutomatonSpec sp(ren);
    BuchiAutomaton<StateSch, int> comp = sp.complementSchReduced();
    cout << "States: " << comp.getStates().size() << endl;

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    BuchiAutomaton<int, int> renCompl = comp.renameAut();
    BuchiAutomaton<int, int> renComplUn = comp.renameAut(ren.getStates().size());
    renCompl.removeUseless();
    auto prod = renCompl.productBA(ren);
    BuchiAutomaton<int, int> renProd = prod.renameAut();
    BuchiAutomaton<int, int> renUnion = renComplUn.unionBA(ren);
    renUnion.singleInitial(renUnion.getStates().size());

    ofstream ch;
    ch.open(TMPNAME);
    if(!ch)
    {
      cerr << "Opening file error" << endl;
      return 1;
    }
    ch << renUnion.toString();
    ch.close();

    string cmdCheck = "java -jar " + RABITEXE + " all.ba " + TMPNAME;

    cout << std::boolalpha;
    cout << "Check: " << (renProd.isEmpty() && (Simulations::execCmd(cmdCheck) == "Included.\n")) << std::endl;
    cout << std::fixed;
    cout << std::setprecision(2);
    cout << "Time: " << (float)(duration/1000.0) << std::endl;
  }
  os.close();
  return 0;
}
