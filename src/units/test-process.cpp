
#include <iostream>
#include <set>
#include <map>
#include <fstream>

#include "../BuchiAutomaton.h"
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
  os.open(argv[1]);

  if(os)
  {
    BuchiAutomaton<string, string> ba = parser.parseBaFormat(os);
    auto ren = ba.renameAut();
    ren.removeUseless();
    //ren.complete(-1);

    cout << ren.toGff() << endl << endl;
    cout << ren.toGraphwiz() << endl;
  }
  os.close();
  return 0;
}
