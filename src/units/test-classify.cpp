
#include <cstdlib>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <chrono>
#include <iomanip>

#include "../Compl-config.h"
#include "../Complement/Options.h"
#include "../Complement/BuchiAutomatonSpec.h"
#include "../Automata/BuchiAutomaton.h"
#include "../Automata/BuchiAutomataParser.h"
#include "../Algorithms/Simulations.h"

using namespace std;

struct Params
{
  string output;
  string input;
};

int main(int argc, char *argv[])
{
  BuchiAutomataParser parser;
  Params params = { .output = "", .input = ""};
  ifstream os;

  if(argc == 2)
  {
    params.input = string(argv[1]);
  }
  else if(argc == 4 && strcmp(argv[2], "-o") == 0)
  {
    params.input = string(argv[1]);
    params.output = string(argv[3]);
  }
  else
  {
    cerr << "Unrecognized arguments" << endl;
    return 1;
  }

  string filename(params.input);
  os.open(params.input);
  if(os)
  {
    try
    {
      BuchiAutomaton<int, APSymbol> ba = parser.parseHoaFormat(os);
      cout << "Deterministic: " << ba.isDeterministic() << endl;
      cout << "Semideterministic: " << ba.isSemiDeterministic() << endl;
    }
    catch(const ParserException& ex)
    {
      cout << ex.what() << endl;
    }
  }
  os.close();
  return 0;
}
