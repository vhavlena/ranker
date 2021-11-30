
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

int main(int argc, char *argv[])
{
  map<string, int> mp = {{"a0", 0}, {"a1", 1}};
  string word = "a0&a1; cycle{!a0&!a1; a0&!a1; !a0&a1}";
  pair<APWord, APWord> inf = BuchiAutomataParser::parseHoaInfWord(word, mp);
  cout << inf.first.toString() << " cycle{ " << inf.second.toString() << "}" << endl;

  return 0;
}
