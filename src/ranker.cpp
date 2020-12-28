
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
#include "Algorithms/AuxFunctions.h"
#include "Automata/BuchiAutomaton.h"
#include "Automata/BuchiAutomataParser.h"
#include "Algorithms/Simulations.h"

using namespace std;

enum InFormat
{
  HOA,
  BA
};

struct Params
{
  string output;
  string input;
  bool stats;
};

struct Stat
{
  size_t generatedStates;
  size_t generatedTrans;
  size_t reachStates;
  size_t reachTrans;
  long duration;
};


InFormat parseRenamedAutomaton(ifstream& os);
BuchiAutomaton<int, int> parseRenameHOA(ifstream& os, BuchiAutomaton<int, APSymbol>* orig);
BuchiAutomaton<int, int> parseRenameBA(ifstream& os, BuchiAutomaton<string, string>* orig);

void complementAutWrap(BuchiAutomaton<int, int>& ren, BuchiAutomaton<int, int>* complRes, Stat* stats);
void printStat(Stat& st);

/**
 * @brief  Retrieves the help message
 *
 * @param[in]  progName  The name of the executable
 *
 * @returns  The help message
 */
std::string getHelpMsg(const std::string& progName);


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
        complementAutWrap(ren, &renCompl, &stats);
      }
      catch (const std::bad_alloc&)
      {
        os.close();
        cerr << "Memory error" << endl;
        return 2;
      }
      map<int, string> symDict = Aux::reverseMap(ba.getRenameSymbolMap());
      BuchiAutomaton<int, string> outOrig = renCompl.renameAlphabet<string>(symDict);

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

      auto t2 = std::chrono::high_resolution_clock::now();
      stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(t2 - t1).count();

      if(params.stats)
        printStat(stats);
      cout << outOrig.toHOA() << endl;
    }
  } else { // file cannot be opened
		std::cerr << "Cannot open file \"" + filename + "\"\n";
		return 1;
	}
  os.close();
  return 0;
}


InFormat parseRenamedAutomaton(ifstream& os)
{
  string fline;
  getline(os, fline);
  os.seekg(0);
  if(fline.rfind("HOA:", 0) == 0)
  {
    return HOA;
  }
  else
  {
    return BA;
  }
}


BuchiAutomaton<int, int> parseRenameHOA(ifstream& os, BuchiAutomaton<int, APSymbol>* orig)
{
  BuchiAutomataParser parser;
  *orig = parser.parseHoaFormat(os);
  Simulations sim;

  auto ranksim = sim.directSimulation<int, APSymbol>(*orig, -1);
  orig->setDirectSim(ranksim);
  auto cl = set<int>();

  orig->computeRankSim(cl);
  return orig->renameAut();
}


BuchiAutomaton<int, int> parseRenameBA(ifstream& os, BuchiAutomaton<string, string>* orig)
{
  BuchiAutomataParser parser;
  *orig = parser.parseBaFormat(os);
  Simulations sim;

  auto ranksim = sim.directSimulation<string, string>(*orig, "-1");
  orig->setDirectSim(ranksim);
  auto cl = set<std::string>();

  orig->computeRankSim(cl);
  return orig->renameAut();
}


void complementAutWrap(BuchiAutomaton<int, int>& ren, BuchiAutomaton<int, int>* complRes, Stat* stats)
{
  BuchiAutomatonSpec sp(ren);
  ComplOptions opt = { .cutPoint = true, .succEmptyCheck = true, .ROMinState = 8, .ROMinRank = 6, .CacheMaxState = 6, .CacheMaxRank = 8 };
  sp.setComplOptions(opt);
  BuchiAutomaton<StateSch, int> comp;
  comp = sp.complementSchReduced();

  stats->generatedStates = comp.getStates().size();
  stats->generatedTrans = comp.getTransitions().size();

  map<int, int> id;
  for(auto al : comp.getAlphabet())
    id[al] = al;
  BuchiAutomaton<int, int> renCompl = comp.renameAutDict(id);
  renCompl.removeUseless();

  stats->reachStates = renCompl.getStates().size();
  stats->reachTrans = renCompl.getTransitions().size();
  *complRes = renCompl;
}


void printStat(Stat& st)
{
  cerr << "Generated states: " << st.generatedStates << "\nGenerated trans: " << st.generatedTrans << endl;
  cerr << "States: " << st.reachStates << "\nTransitions: " << st.reachTrans << endl;
  cerr << std::fixed;
  cerr << std::setprecision(2);
  cerr << "Time: " << (float)(st.duration/1000.0) << std::endl;
}

std::string getHelpMsg(const std::string& progName)
{
	std::string helpMsg;
	helpMsg += "Usage: " + progName + " [--stats] INPUT\n";
	helpMsg += "\n";
	helpMsg += "Complements a (state-based acceptance condition) Buchi automaton.\n";
	helpMsg += "\n";
	helpMsg += "INPUT is the name of a file in the HOA (Hanoi Omega Automata) format\n";
	helpMsg += "(see https://adl.github.io/hoaf/ ) with the following restrictions:\n";
	helpMsg += "  * only state-based acceptance is supported\n";
	helpMsg += "  * transitions need to have the form of a single conjunction with exactly\n";
	helpMsg += "    one positive atomic proposition\n";
	helpMsg += "  * no aliases or any other fancy features of HOA are supported\n";
	helpMsg += "\n";
	helpMsg += "Flags:\n";
	helpMsg += "  --stats       Print summary statistics\n";

	return helpMsg;
} // getHelpMsg
