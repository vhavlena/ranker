
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

    cout << comp.getStates().size() << endl;
    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    cout << std::fixed;
    cout << std::setprecision(2);
    cout << "Time: " << (float)(duration/1000.0) << std::endl;
  }
  os.close();
  return 0;
}
