
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
  return 0;
}
