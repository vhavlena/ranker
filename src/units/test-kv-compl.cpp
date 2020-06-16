
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include "../BuchiAutomaton.h"
#include "../BuchiAutomatonSpec.h"
#include "../BuchiAutomataParser.h"

using namespace std;

int main()
{
  BuchiAutomataParser parser;
  ifstream os;
  os.open("../../examples/A02.ba");
  cout << "../../examples/A02.ba" << endl;

  if(os)
  {
    BuchiAutomaton<string, string> ba = parser.parseBaFormat(os);
    BuchiAutomaton<int, int> ren = ba.renameAut();
    BuchiAutomatonSpec sp(ren);
    BuchiAutomaton<StateKV, int> comp = sp.complementKV();
    cout << comp.toString() << endl;
  }
  os.close();
  return 0;
}
