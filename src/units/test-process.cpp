
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
  cout << argv[1] << endl;

  if(os)
  {
    BuchiAutomaton<string, string> ba = parser.parseBaFormat(os);
    BuchiAutomaton<int, int> ren = ba.renameAut();
    ren.removeUseless();
    //ren.complete(-1);
    ren = ren.renameAut();
    cout << ren.toGraphwiz() << endl;

    auto vec = ren.reachableVector();
    for(int i = 0; i < vec.size(); i++)
    {
      cout << i << ": ";
      for(auto s : vec[i])
        cout << s << ", ";
      cout << endl;
    }

    cout << ren.toGraphwiz() << endl;
  }
  os.close();
  return 0;
}