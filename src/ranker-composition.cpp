
#include <cstdlib>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <string>

#include "Compl-config.h"
#include "Ranker-general.h"
#include "Complement/Options.h"
#include "Complement/BuchiAutomatonSpec.h"
#include "Automata/BuchiAutomaton.h"
#include "Automata/BuchiAutomataParser.h"
#include "Algorithms/Simulations.h"

using namespace std;

bool suitCase(BuchiAutomatonSpec& sp);

int main(int argc, char *argv[])
{
  Params params = { .output = "", .input = "", .stats = false};
  ifstream os;

  if(argc == 2)
  {
		if ((std::string(argv[1]) == "--help") || (std::string(argv[1]) == "-h")) {
			cerr << getHelpMsg(argv[0]);
			return 0;
		} else {
			params.input = string(argv[1]);
		}
  }
  else if(argc == 3 && strcmp(argv[2], "--stats") == 0)
  {
    params.input = string(argv[1]);
    params.stats = true;
  }
  else if(argc == 3 && strcmp(argv[1], "--stats") == 0)
  {
    params.input = string(argv[2]);
    params.stats = true;
  }
  else
  {
    cerr << "Unrecognized arguments" << endl;
		cerr << "\n";
		cerr << getHelpMsg(argv[0]);
    return 1;
  }

  //string filename = params.input;
  os.open(params.input);
  if(!os)
  {
    std::cerr << "Cannot open file " << params.input << "\n";
    return EXIT_FAILURE;
  }
  else
  {
    // const char* rabitpath_cstr = std::getenv("RABITEXE");
    // std::string rabitpath = (nullptr == rabitpath_cstr)? RABITEXE : rabitpath_cstr;
    const char* goalpath_cstr = std::getenv("GOALEXE");
    std::string goalpath = (nullptr == goalpath_cstr)? GOALEXE : goalpath_cstr;

    BuchiAutomaton<int, APSymbol> ba;
    BuchiAutomaton<int, int> ren;
    BuchiAutomaton<int, int> renCompl;
    Stat stats;
    try
    {
      ren = parseRenameHOA(os, &ba);
    }
    catch(const ParserException& e)
    {
      os.close();
      cerr << "Parser error:" << endl;
      cerr << "line " << e.getLine() << ": " << e.what() << endl;
      return 2;
    }

    BuchiAutomatonSpec sp(ren);
    if(!suitCase(sp))
    {
      const char* tmpf_name = nullptr;
      std::FILE* tmpf = nullptr;

      // open for writing and fail if already exists (the "wx",
      // see https://en.cppreference.com/w/cpp/io/c/fopen).
      // This is used to atomically create a file and get its name.
      while (nullptr == (tmpf = std::fopen(tmpf_name, "wx"))) {
        #pragma GCC diagnostic push   // tmpnam is deprecated
        #pragma GCC diagnostic ignored "-Wdeprecated-declarations"
        tmpf_name = std::tmpnam(nullptr);
        #pragma GCC diagnostic pop
      }

      std::fputs(ren.toGff().c_str(), tmpf);
      std::fflush(tmpf);

      BuchiAutomataParser parser;
      string cmd = goalpath + " complement -m piterman -r " + tmpf_name;
      string ret = Simulations::execCmd(cmd);

      map<int, APSymbol> symDict = Aux::reverseMap(ba.getRenameSymbolMap());
      BuchiAutomaton<string, string> bagff = parser.parseGffFormat(ret);
      bagff.setAPPattern(ba.getAPPattern());

      map<string, int> symDictStrInt;
      for(const auto &t : symDict)
      {
        symDictStrInt.insert({to_string(t.first), t.first});
      }
      BuchiAutomaton<int, APSymbol> outOrig = bagff.renameAutDict(symDictStrInt).renameAlphabet(symDict);

      stats.engine = "GOAL";
      stats.duration = NAN;
      stats.generatedStates = NAN;
      stats.generatedTrans = NAN;
      stats.reachStates = outOrig.getStates().size();
      stats.reachTrans = outOrig.getTransCount();
      if(params.stats)
        printStat(stats);

      cout << outOrig.toHOA() << endl;
    }
    else
    {
      try
      {
        complementAutWrap(ren, &renCompl, &stats);
      }
      catch (const std::bad_alloc&)
      {
        os.close();
        cerr << "Memory error" << endl;
        return 2;
      }
      map<int, APSymbol> symDict = Aux::reverseMap(ba.getRenameSymbolMap());
      BuchiAutomaton<int, APSymbol> outOrig = renCompl.renameAlphabet<APSymbol>(symDict);

      stats.duration = NAN;
      if(params.stats)
        printStat(stats);
      cout << outOrig.toHOA() << endl;
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
