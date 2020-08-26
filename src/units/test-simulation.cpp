
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <chrono>
#include <iomanip>

#include "units-config.h"
#include "../BuchiAutomaton.h"
#include "../Simulations.h"
#include "../BuchiAutomataParser.h"
#include "../Options.h"

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
    
    cout << "Rabit Direct Simulation: " << endl;
    for(const auto& t : sim.parseRabitRelation(strr))
    {
      cout << t.first << " : " << t.second << endl;
    }

    cout << endl << "Direct Simulation: " << endl;
    for(const auto& t : sim.directSimulation<string, string>(ba, "-1"))
    {
      cout << t.first << " : " << t.second << endl;
    }
  }
  os.close();
  return 0;
}
