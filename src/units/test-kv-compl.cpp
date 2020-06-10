
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include "../BuchiAutomaton.h"
#include "../BuchiAutomataParser.h"

using namespace std;

int main()
{
  BuchiAutomataParser parser;
  ifstream os;
  os.open("../../examples/A3.ba");
  cout << "../../examples/A3.ba" << endl;

  if(os)
  {
    BuchiAutomaton<string, string> ba = parser.parseBaFormat(os);
    BuchiAutomaton<int, int> ren = ba.renameAut();
    BuchiAutomaton<StateKV<int>, int> comp = ren.complementKV();
    cout << comp.toString() << endl;
  }
  os.close();
  return 0;
}
