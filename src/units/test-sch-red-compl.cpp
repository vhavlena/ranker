
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

string printRTMacrostate(const set<StateSch>& m)
{
  string ret = "<";
  for(auto& item : m)
  {
    ret += item.toString() + " : ";
  }
  ret += ">";
  return ret;
}

string printRunTree(vector<set<StateSch>> &rt, vector<int>& word)
{
  string str = "digraph \" Automaton \" { rankdir=LR;\n { rank = LR }\n";
  str += "node [shape = circle];";
  for(const auto& item : rt)
  {
    str += "\"" + printRTMacrostate(item) + "\"\n";
  }
  for(int i = 0; i < rt.size() - 1; i++)
  {
    str += "\"" + printRTMacrostate(rt[i]) + "\" -> \"" + printRTMacrostate(rt[i+1]) + "\" [label=" + to_string(word[i]) + "]\n";
  }
  return str;
}

string RABITEXE = "";
string TMPNAME = "tmp124232.ba";

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
  cout << argv[1] << endl;

  if(os)
  {
    BuchiAutomaton<string, string> ba = parser.parseBaFormat(os);
    string cmd = "java -jar " + RABITEXE + " " + filename + " " + filename + " -dirsim";
    Simulations sim;
    istringstream strr(Simulations::execCmd(cmd));

    ba.setDirectSim(sim.parseRabitRelation(strr));
    auto cl = set<std::string>();
    auto t1 = std::chrono::high_resolution_clock::now();

    ba.computeRankSim(cl);
    BuchiAutomaton<int, int> ren = ba.renameAut();
    BuchiAutomatonSpec sp(ren);
    ComplOptions opt = { .cutPoint = false };
    sp.setComplOptions(opt);
    BuchiAutomaton<StateSch, int> comp = sp.complementSchReduced();

    cout << "Generated states: " << comp.getStates().size() << " Generated trans: " << comp.getTransitions().size() << endl;

    auto t2 = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

    map<int, int> id;
    for(auto al : comp.getAlphabet())
      id[al] = al;
    BuchiAutomaton<int, int> renCompl = comp.renameAutDict(id);
    BuchiAutomaton<int, int> renComplUn = comp.renameAutDict(id, ren.getStates().size());

    renCompl.removeUseless();
    cout << "States: " << renCompl.getStates().size() << " Transitions: " << renCompl.getTransitions().size() << endl;

    auto prod = renCompl.productBA(ren);
    BuchiAutomaton<int, int> renProd = prod.renameAut();
    BuchiAutomaton<int, int> renUnion = renComplUn.unionBA(ren);
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
      checkRes = Simulations::execCmd(cmdCheck, 10);
      checkRes = renProd.isEmpty() && checkRes == "Included.\n" ? "true" : "false";
    }
    catch(TimeoutException)
    {
      checkRes = "TO";
    }

    cout << std::boolalpha;
    cout << "Check: " << checkRes << std::endl;
    cout << std::fixed;
    cout << std::setprecision(2);
    cout << "Time: " << (float)(duration/1000.0) << std::endl;
  }
  os.close();
  return 0;
}
