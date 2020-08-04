
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
#include "../Options.h"

using namespace std;

string RABITEXE = "";
string TMPNAME = "tmp124232.ba";

int main(int argc, char *argv[])
{
  BuchiAutomataParser parser;
  ifstream os1;
  ifstream os2;

  if(argc != 3)
  {
    cerr << "Bad arguments" << endl;
    return 1;
  }
  string filename1(argv[1]);
  string filename2(argv[2]);
  os1.open(argv[1]);
  os2.open(argv[2]);

  if(os1 && os2)
  {
    BuchiAutomaton<string, string> ba1 = parser.parseBaFormat(os1);
    BuchiAutomaton<string, string> ba2 = parser.parseBaFormat(os2);

    map<string, int> id1 = {{"a0", 0}, {"a1", 1}};
    map<string, int> id2 = {{"0", 0}, {"1", 1}};
    BuchiAutomaton<int, int> ren = ba1.renameAutDict(id1);
    BuchiAutomaton<int, int> renCompl = ba2.renameAutDict(id1, ren.getStates().size());

    BuchiAutomaton<int, int> renUnion = renCompl.unionBA(ren);
    renUnion.singleInitial(renUnion.getStates().size());

    ofstream ch;
    ch.open(TMPNAME);
    if(!ch)
    {
      cerr << "Opening file error" << endl;
      return 1;
    }
    ch << renUnion.toString();
    ch.close();

    string cmdCheck = "java -jar " + RABITEXE + " all.ba " + TMPNAME + " -de";
    string checkRes = "";
    try
    {
      checkRes = Simulations::execCmd(cmdCheck, 25);
      checkRes = checkRes == "Included.\n" ? "true" : "false";
    }
    catch(const TimeoutException &)
    {
      checkRes = "TO";
    }

    cout << std::boolalpha;
    cout << "Check: " << checkRes << std::endl;
  }
  os1.close();
  os2.close();
  return 0;
}
