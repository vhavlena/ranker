
#include <cstdlib>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <chrono>
#include <iomanip>
#include "External/args.hxx" // argument parsing

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
  Params params = { .output = "", .input = "", .stats = false, .checkWord = ""};
  ifstream os;
  bool delay = false;
  double w = 0.5;
  delayVersion version = oldVersion;
  //bool error = false;
  bool elevatorTest = false;
  bool elevatorRank = false;
  bool eta4 = false;

  args::ArgumentParser parser("Program complementing a (state-based acceptance condition) Buchi automaton.\n", "");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});

  args::Positional<std::string> inputFile(parser, "INPUT", "The name of a file in the HOA (Hanoi Omega Automata) format with the following restrictions:\n* only state-based acceptance is supported\n* transitions need to have the form of a single conjunction with exactly one positive atomic proposition\n* no aliases or any other fancy features of HOA are supported\n");
  args::Flag statsFlag(parser, "", "Print summary statistics", {"stats"});
  args::ValueFlag<std::string> delayFlag(parser, "version", "Use delay optimization, versions: old, new, random, subset, stirling", {"delay"});
  args::ValueFlag<std::string> checkFlag(parser, "word", "Product of the result with a given word", {"check"});
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

  if(checkFlag)
  {
    params.checkWord = args::get(checkFlag);
  }

  // delay version
  if (delayFlag){
    delay = true;
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


  string filename(params.input);
  os.open(filename);
  if(os)
  { // file opened correctly
    InFormat fmt = parseRenamedAutomaton(os);

    Stat stats;
    stats.beginning = std::chrono::high_resolution_clock::now();

    if(fmt == BA)
    {
      BuchiAutomaton<string, string> ba;
      AutomatonStruct<int, int> *ren = parseRenameBA(os, &ba);

      BuchiAutomaton<int, int> renCompl;
      BuchiAutomaton<StateSch, int> comp;

      // elevator test
      if (elevatorTest){
        std::cout << "Elevator automaton: " << (ren->isElevator() ? "Yes" : "No") << std::endl;
        os.close();
        return 0;
      }

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
      map<int, string> symDict = Aux::reverseMap(ba.getRenameSymbolMap());
      BuchiAutomaton<int, string> outOrig = renCompl.renameAlphabet<string>(symDict);

      stats.end = std::chrono::high_resolution_clock::now();
      stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(stats.end - stats.beginning).count();

      if(params.stats)
        printStat(stats);
      cout << outOrig.toHOA() << endl;
    }

    else if(fmt == HOA)
    {
      AutomatonStruct<int, APSymbol> *ba;
      AutomatonStruct<int, int> *ren;

      BuchiAutomaton<int, int> renCompl;
      BuchiAutomaton<StateSch, int> comp;

      BuchiAutomaton<int, int> *renBuchi = nullptr;

      try
      {
        ba = parseRenameHOA(os);
        ren = ba->renameAut();
        if (dynamic_cast<BuchiAutomaton<int, int>*>(ren))
          renBuchi = (BuchiAutomaton<int, int>*)ren;

        // elevator test
        if (elevatorTest){
          std::cout << "Elevator automaton: " << (ren->isElevator() ? "Yes" : "No") << std::endl;
          os.close();
          return 0;
        }
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
        complementAutWrap(renBuchi, &comp, &renCompl, &stats, delay, w, version, elevatorRank, eta4);
      }
      catch (const std::bad_alloc&)
      {
        os.close();
        cerr << "Memory error" << endl;
        return 2;
      }

      map<int, APSymbol> symDict = Aux::reverseMap(ba->getRenameSymbolMap());

      //Product with a word
      if(params.checkWord.size() > 0)
      {
        cout << "Product in Graphwiz:" << endl;
        auto appattern = comp.getAPPattern();
        pair<APWord, APWord> inf = BuchiAutomataParser::parseHoaInfWord(params.checkWord, appattern);
        auto prefv = inf.first.getVector();
        auto loopv = inf.second.getVector();
        auto debugRename = comp.renameAlphabet<APSymbol>(symDict);
        BuchiAutomatonDebug<StateSch, APSymbol> compDebug(debugRename);
        auto ret = compDebug.getSubAutomatonWord(prefv, loopv);
        cout << ret.toGraphwiz() << endl;

        os.close();
        return 0;
      }


      BuchiAutomaton<int, APSymbol> outOrig = renCompl.renameAlphabet<APSymbol>(symDict);
      outOrig.completeAPComplement();
      stats.reachStates = outOrig.getStates().size();
      stats.reachTrans = outOrig.getTransCount();

      stats.end = std::chrono::high_resolution_clock::now();
      stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(stats.end - stats.beginning).count();

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
