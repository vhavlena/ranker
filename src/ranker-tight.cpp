
#include <cstdlib>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <chrono>
#include <iomanip>

#include "Ranker-general.h"
#include "Compl-config.h"
#include "Complement/Options.h"
#include "Complement/BuchiAutomatonSpec.h"
#include "Algorithms/AuxFunctions.h"
#include "Automata/BuchiAutomaton.h"
#include "Automata/BuchiAutomataParser.h"
#include "Algorithms/Simulations.h"

using namespace std;

int main(int argc, char *argv[])
{
  Params params = { .output = "", .input = "", .stats = false};
  ifstream os;
  bool delay = false;
  double w = 0.5; 
  delayVersion version;
  bool error = false;

  // without delay
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

  // delay without weight
  else if (argc == 5 and strcmp(argv[1], "--stats") == 0 and strcmp(argv[2], "--delay") == 0){
    params.input = string(argv[4]);
    params.stats = true;
    delay = true;
    if (strcmp(argv[3], "--old") == 0)
      version = oldVersion;
    else if (strcmp(argv[3], "--new") == 0)
      version = newVersion;
    else if (strcmp(argv[3], "--random") == 0)
      version = randomVersion;
    else if (strcmp(argv[3], "--subset") == 0)
      version = subsetVersion;
    else
      error = true;
  }
  else if (argc == 4 and strcmp(argv[1], "--delay") == 0){
    params.input = string(argv[3]);
    params.stats = false;
    delay = true;
    if (strcmp(argv[2], "--old") == 0)
      version = oldVersion;
    else if (strcmp(argv[2], "--new") == 0)
      version = newVersion;
    else if (strcmp(argv[2], "--random") == 0)
      version = randomVersion;
    else if (strcmp(argv[2], "--subset") == 0)
      version = subsetVersion;
    else
      error = true;
  }

  // delay with weight
  else if (argc == 7 and strcmp(argv[1], "--stats") == 0 and strcmp(argv[2], "--delay") == 0 and strcmp(argv[4], "-w") == 0){
    params.input = string(argv[6]);
    params.stats = true;
    delay = true;
    w = stod(argv[5]);
    if (strcmp(argv[3], "--old") == 0)
      version = oldVersion;
    else if (strcmp(argv[3], "--new") == 0)
      version = newVersion;
    else if (strcmp(argv[3], "--random") == 0)
      version = randomVersion;
    else if (strcmp(argv[3], "--subset") == 0)
      version = subsetVersion;
    else
      error = true;
  }
  else if (argc == 6 and strcmp(argv[1], "--delay") == 0 and strcmp(argv[3], "-w") == 0){
    params.input = string(argv[4]);
    params.stats = false;
    delay = true;
    w = stod(argv[4]);
    if (strcmp(argv[2], "--old") == 0)
      version = oldVersion;
    else if (strcmp(argv[2], "--new") == 0)
      version = newVersion;
    else if (strcmp(argv[2], "--random") == 0)
      version = randomVersion;
    else if (strcmp(argv[2], "--subset") == 0)
      version = subsetVersion;
    else
      error = true;
  }
  
  // error
  else
  {
    cerr << "Unrecognized arguments" << endl;
		cerr << "\n";
		cerr << getHelpMsg(argv[0]);
    return 1;
  }
  
  if (error or w < 0.0 or w > 1.0){
    cerr << "Unrecognized arguments" << endl;
		cerr << "\n";
		cerr << getHelpMsg(argv[0]);
    return 1;
  }

  string filename(params.input);
  os.open(filename);
  if(os)
  { // file opened correctly
    InFormat fmt = parseRenamedAutomaton(os);
    auto t1 = std::chrono::high_resolution_clock::now();
    BuchiAutomaton<int, int> renCompl;
    Stat stats;

    if(fmt == BA)
    {
      BuchiAutomaton<string, string> ba;
      BuchiAutomaton<int, int> ren = parseRenameBA(os, &ba);
      try
      {
        complementScheweAutWrap(ren, &renCompl, &stats, delay, w, version);
      }
      catch (const std::bad_alloc&)
      {
        os.close();
        cerr << "Memory error" << endl;
        return 2;
      }
      map<int, string> symDict = Aux::reverseMap(ba.getRenameSymbolMap());
      BuchiAutomaton<int, string> outOrig = renCompl.renameAlphabet<string>(symDict);

      auto t2 = std::chrono::high_resolution_clock::now();
      stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

      if(params.stats)
        printStat(stats);
      cout << outOrig.toHOA() << endl;
    }
    else if(fmt == HOA)
    {
      BuchiAutomaton<int, APSymbol> ba;
      BuchiAutomaton<int, int> ren;
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

      try
      {
        complementScheweAutWrap(ren, &renCompl, &stats, delay, w, version);
      }
      catch (const std::bad_alloc&)
      {
        os.close();
        cerr << "Memory error" << endl;
        return 2;
      }
      map<int, APSymbol> symDict = Aux::reverseMap(ba.getRenameSymbolMap());
      BuchiAutomaton<int, APSymbol> outOrig = renCompl.renameAlphabet<APSymbol>(symDict);
      outOrig.completeAPComplement();
      stats.reachStates = outOrig.getStates().size();
      stats.reachTrans = outOrig.getTransCount();

      auto t2 = std::chrono::high_resolution_clock::now();
      stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

      if(params.stats)
        printStat(stats);
      cout << outOrig.toHOA() << endl;
    }
  }
  else
  { // file cannot be opened
		std::cerr << "Cannot open file \"" + filename + "\"\n";
		return 1;
	}
  os.close();
  return 0;
}
