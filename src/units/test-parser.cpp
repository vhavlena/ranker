
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>

#include "units-config.h"
#include "../BuchiAutomaton.h"
#include "../BuchiAutomataParser.h"
#include "../Simulations.h"

using namespace std;

int main()
{
  BuchiAutomataParser parser;
  ifstream os;
  os.open("../../examples/A3.ba");
  cout << "../../examples/A3.ba" << endl;

  if(os)
  {
    auto ba = parser.parseBaFormat(os);
    cout << ba.toString() << endl;
  }
  os.close();

  os.open("../../examples/A4.ba");
  cout << endl << "../../examples/A4.ba" << endl;
  if(os)
  {
    auto ba = parser.parseBaFormat(os);
    cout << ba.toString() << endl;
  }
  os.close();

  os.open("../../examples/A6.ba");
  cout << endl << "../../examples/A6.ba" << endl;
  if(os)
  {
    auto ba = parser.parseBaFormat(os);
    cout << ba.toString() << endl;
  }
  os.close();

  cout << endl << "Simulation parsing: " << endl;
  string cmd = "java -jar " + RABITEXE + " ../../examples/2.5-0.1/A6.ba ../../examples/2.5-0.1/A6.ba -dirsim";
  Simulations sim;
  istringstream strr(Simulations::execCmd(cmd));
  try
  {
    for(const auto& item : sim.parseRabitRelation(strr))
    {
      cout << item.first << " " << item.second << endl;
    }
  }
  catch(string& msg)
  {
    cout << msg << endl;
  }

  return 0;
}
