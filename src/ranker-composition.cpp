
#include <cstdlib>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <chrono>
#include <iomanip>

#include "Compl-config.h"
#include "Complement/Options.h"
#include "Complement/BuchiAutomatonSpec.h"
#include "Automata/BuchiAutomaton.h"
#include "Automata/BuchiAutomataParser.h"
#include "Algorithms/Simulations.h"

using namespace std;

struct Params
{
  string output;
  string input;
};

bool suitCase(BuchiAutomatonSpec& sp);

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
    const char* rabitpath_cstr = std::getenv("RABITEXE");
    std::string rabitpath = (nullptr == rabitpath_cstr)? RABITEXE : rabitpath_cstr;
    const char* goalpath_cstr = std::getenv("GOALEXE");
    std::string goalpath = (nullptr == goalpath_cstr)? GOALEXE : goalpath_cstr;

    BuchiAutomaton<string, string> ba = parser.parseBaFormat(os);
    string cmd = "java -jar " + rabitpath + " " + filename + " " + filename + " -dirsim";
    Simulations sim;
    istringstream strr(Simulations::execCmd(cmd));

    ba.setDirectSim(sim.parseRabitRelation(strr));
    auto cl = set<std::string>();

    ba.computeRankSim(cl);
    BuchiAutomaton<int, int> ren = ba.renameAut();
    BuchiAutomatonSpec sp(ren);
    BuchiAutomaton<int, int> renCompl;

    if(!suitCase(sp))
    {
      ofstream ch;
      ch.open(GFFTMPNAME);
      if(!ch)
      {
        cerr << "Opening file error" << endl;
        os.close();
        ch.close();
        return 1;
      }
      ch << ren.toGff();
      ch.close();

      cmd = goalpath + " complement -m piterman -r " + GFFTMPNAME;
      string ret = Simulations::execCmd(cmd);
      BuchiAutomaton<string, string> bagff = parser.parseGffFormat(ret);
      renCompl = bagff.renameAut();
    }
    else
    {
      ComplOptions opt = { .cutPoint = true, .succEmptyCheck = true, .ROMinState = 8, .ROMinRank = 6, .CacheMaxState = 6, .CacheMaxRank = 8 };
      sp.setComplOptions(opt);
      BuchiAutomaton<StateSch, int> comp;
      try
      {
        comp = sp.complementSchReduced();
      }
      catch (const std::bad_alloc&)
      {
        os.close();
        cerr << "Memory error" << endl;
        return 2;
      }

      map<int, int> id;
      for(auto al : ren.getAlphabet())
        id[al] = al;
      renCompl = comp.renameAutDict(id);
      renCompl.removeUseless();
    }

    cout << "States: " << renCompl.getStates().size() << "\nTransitions: " << renCompl.getTransitions().size() << endl;

    if(params.output != "")
    {
      ofstream ch;
      ch.open(params.output);
      if(!ch)
      {
        cerr << "Cannot open the output file" << endl;
        return 1;
      }
      ch << renCompl.toString();
      ch.close();
    }
  }
  os.close();
  return 0;
}


bool suitCase(BuchiAutomatonSpec& sp)
{
  BuchiAutomaton<StateSch, int> comp = sp.complementSchNFA(sp.getInitials());
  set<StateSch> slIgnore = sp.nfaSlAccept(comp);

  auto maxReach = sp.getMaxReachSize(comp, slIgnore);
  auto minReach = sp.getMinReachSize();
  auto maxReachInd = sp.getMaxReachSizeInd();

  auto slnoAccept = sp.nfaSingleSlNoAccept(comp);
  set<StateSch> ignoreAll;
  for(const auto& t : slnoAccept)
    ignoreAll.insert({t.first, set<int>(), RankFunc(), 0, false});
  ignoreAll.insert(slIgnore.begin(), slIgnore.end());

  auto rankBound = sp.getRankBound(comp, ignoreAll, maxReach, minReach);

  for(auto t : comp.getCycleClosingStates(slIgnore))
  {
    if((t.S.size() >= 9 && rankBound[t.S] >= 5) || (t.S.size() >= 8 && rankBound[t.S] >= 6))
    {
      return false;
    }
  }
  return true;
}
