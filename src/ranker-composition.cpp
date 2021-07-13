
#include <cstdlib>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <string>
#include "args.hxx"

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
  bool delay = false;
  double w = 0.5;
  delayVersion version;
  //bool error = false;
  bool elevatorTest = false;
  bool elevatorRank = false;
  bool eta4 = false;

 args::ArgumentParser parser("Program complementing a (state-based acceptance condition) Buchi automaton.\n", "");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});

  args::Positional<std::string> inputFile(parser, "INPUT", "The name of a file in the HOA (Hanoi Omega Automata) format with the following restrictions:\n* only state-based acceptance is supported\n* transitions need to have the form of a single conjunction with exactly one positive atomic proposition\n* no aliases or any other fancy features of HOA are supported\n");
  args::Flag statsFlag(parser, "", "Print summary statistics", {"stats"});
  args::ValueFlag<std::string> delayFlag(parser, "version", "Use delay optimization, versions: old, new, random, subset, stirling", {"delay"});
  args::ValueFlag<double> weightFlag(parser, "value", "Weight parameter for delay - value in <0,1>", {'w', "weight"});
  args::Flag elevatorFlag(parser, "elevator rank", "Update rank upper bound of each macrostate based on elevator automaton structure", {"elevator-rank"});
  args::Flag eta4Flag(parser, "eta4", "Max rank optimization - eta 4 only when going from some accepting state", {"eta4"});
  args::Flag elevatorTestFlag(parser, "elevator test", "Test if INPUT is an elevator automaton", {"elevator-test"});

  try
  {
      parser.ParseCLI(argc, argv);
  }
  catch (args::Help)
  {
      std::cout << parser;
      return 0;
  }
  catch (args::ParseError e)
  {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }
  catch (args::ValidationError e)
  {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }

  // input file
  if (inputFile){
    params.input = args::get(inputFile);
  }

  // print statistics
  if (statsFlag){
    params.stats = true;
  }

  // delay version
  if (delayFlag){
    std::string v = args::get(delayFlag);
    if (v == "old")
      version = oldVersion;
    else if (v == "new")
      version = newVersion;
    else if (v == "random")
      version = randomVersion;
    else if (v == "subset")
      version = subsetVersion;
    else if (v == "stirling")
      version = stirlingVersion;
    else {
      std::cerr << "Wrong delay version" << std::endl;
      return 1;
    }
  }

  // weight parameters for delay
  if (weightFlag){
    if (not delayFlag){
      std::cerr << "Wrong combination of arguments" << std::endl;
      return 1;
    }
    w = args::get(weightFlag);
    if (w < 0.0 or w > 1.0) {
      std::cerr << "Wrong weight parameter" << std::endl;
      return 1;
    }
  }

  // elevator rank
  if (elevatorFlag){
    elevatorRank = true;
  }

  // eta4
  if (eta4Flag){
    eta4 = true;
  }

  if (elevatorTestFlag){
    elevatorTest = true;
    if (statsFlag or delayFlag or weightFlag or elevatorFlag or eta4Flag){
      std::cerr << "Wrong combination of arguments" << std::endl;
      return 1;
    }
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
    BuchiAutomaton<StateSch, int> comp;
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
      outOrig.completeAPComplement();

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
        complementAutWrap(ren, &comp, &renCompl, &stats, delay, w, version, elevatorRank, eta4);
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
  map<StateSch, DelayLabel> delayMp;
  for(const auto& st : comp.getStates())
  {
    delayMp[st] = { .macrostateSize = (unsigned)st.S.size(), .maxRank = (unsigned)rankBound[st.S] };
  }

  //for(auto t : comp.getCycleClosingStates(slIgnore, delayMp))
  for (auto t : comp.getCycleClosingStates(slIgnore))
  {
    if((t.S.size() >= 9 && rankBound[t.S] >= 5) || (t.S.size() >= 8 && rankBound[t.S] >= 6))
    {
      return false;
    }
  }
  return true;
}
