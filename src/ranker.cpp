
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
#include "Automata/GenCoBuchiAutomaton.h"
#include "Complement/GenCoBuchiAutomatonCompl.h"

using namespace std;

int main(int argc, char *argv[])
{
  Params params = { .output = "", .input = "", .stats = false, .checkWord = ""};
  ifstream os;
  //bool error = false;
  bool elevatorTest = false;
  //elevatorOptions elevator = {.elevatorRank = false, .detBeginning = false};

  args::ArgumentParser parser("Program complementing a (state-based acceptance condition) Buchi automaton.\n", "");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});

  args::Positional<std::string> inputFile(parser, "INPUT", "The name of a file in the HOA (Hanoi Omega Automata) format with the following restrictions:\n* only state-based acceptance is supported\n* transitions need to have the form of a single conjunction with exactly one positive atomic proposition\n* no aliases or any other fancy features of HOA are supported\n");
  args::Flag statsFlag(parser, "", "Print summary statistics", {"stats"});
  args::ValueFlag<std::string> delayFlag(parser, "version", "Use delay optimization, versions: old, new, random, subset, stirling", {"delay"});
  args::ValueFlag<std::string> checkFlag(parser, "word", "Product of the result with a given word", {"check"});
  args::ValueFlag<std::string> dataFlowFlag(parser, "dataflow", "Data flow analysis [light/inner]", {"flow"});
  args::ValueFlag<double> weightFlag(parser, "value", "Weight parameter for delay - value in <0,1>", {'w', "weight"});
  args::Flag elevatorFlag(parser, "no elevator rank", "Don't update rank upper bound of each macrostate based on elevator automaton structure", {"no-elevator-rank"});
  args::Flag elevatorDetBeg(parser, "elevator deterministic beginning", "Rank 0/1 to all states in the D/IW component in the beginning", {"det-beg"});
  args::Flag eta4Flag(parser, "eta4", "Max rank optimization - eta 4 only when going from some accepting state", {"eta4"});
  args::Flag elevatorTestFlag(parser, "elevator test", "Test if INPUT is an elevator automaton", {"elevator-test"});
  args::Flag debugFlag(parser, "debug", "Print debug statistics", {"debug"});
  args::Flag lightFlag(parser, "light", "Use lightweight optimizations", {"light"});
  args::ValueFlag<std::string> preprocessFlag(parser, "preprocess", "Preprocessing [copyiwa/copydet/copyall/copytrivial/copyheur]", {"preprocess"});
  args::Flag accPropagationFlag(parser, "acc-propagation", "Propagate accepting states in each SCC", {"acc-propagation"});

  ComplOptions opt = { .cutPoint = true, .succEmptyCheck = true, .ROMinState = 8,
      .ROMinRank = 6, .CacheMaxState = 6, .CacheMaxRank = 8, .semidetOpt = false,
      .dataFlow = INNER, .delay = false, .delayVersion = oldVersion, .delayW = 0.5,
      .debug = false, .elevator = { .elevatorRank = true, .detBeginning = false },
      .sim = true, .sl = true, .reach = true, .flowDirSim = false, .preprocess = NONE, .accPropagation = false };

  try
  {
      parser.ParseCLI(argc, argv);
  }
  catch (args::Help&)
  {
      std::cout << parser;
      return 0;
  }
  catch (args::ParseError& e)
  {
      std::cerr << e.what() << std::endl;
      std::cerr << parser;
      return 1;
  }
  catch (args::ValidationError& e)
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
  if (statsFlag)
  {
    params.stats = true;
  }

  if(checkFlag)
  {
    params.checkWord = args::get(checkFlag);
  }

  if(dataFlowFlag && args::get(dataFlowFlag) == "light")
  {
    opt.dataFlow = LIGHT;
  }

  if(preprocessFlag)
  {
    if(args::get(preprocessFlag) == "copyiwa")
      opt.preprocess = CPIWA;
    else if(args::get(preprocessFlag) == "copydet")
      opt.preprocess = CPDET;
    else if(args::get(preprocessFlag) == "copyall")
      opt.preprocess = CPALL;
    else if(args::get(preprocessFlag) == "copytrivial")
      opt.preprocess = CPTRIVIAL;
    else if(args::get(preprocessFlag) == "copyheur")
      opt.preprocess = CPHEUR;
    else {
      std::cerr << "Wrong copy attribute" << std::endl;
      return 1;
    }
  }

  // delay version
  if (delayFlag){
    opt.delay = true;
    std::string v = args::get(delayFlag);
    if (v == "old")
      opt.delayVersion = oldVersion;
    else if (v == "new")
      opt.delayVersion = newVersion;
    else if (v == "random")
      opt.delayVersion = randomVersion;
    else if (v == "subset")
      opt.delayVersion = subsetVersion;
    else if (v == "stirling")
      opt.delayVersion = stirlingVersion;
    else {
      std::cerr << "Wrong delay version" << std::endl;
      return 1;
    }
  }

  if(debugFlag)
  {
    opt.debug = true;
  }

  if (accPropagationFlag)
  {
    opt.accPropagation = true;
  }

  // weight parameters for delay
  if (weightFlag){
    if (not delayFlag){
      std::cerr << "Wrong combination of arguments" << std::endl;
      return 1;
    }
    float w = args::get(weightFlag);
    if (w < 0.0 or w > 1.0) {
      std::cerr << "Wrong weight parameter" << std::endl;
      return 1;
    }
    opt.delayW = w;
  }

  // elevator rank
  if (elevatorFlag){
    opt.elevator.elevatorRank = false;
    // if (elevatorDetBeg)
    //   elevator.detBeginning = true;
  }

  // eta4
  if (eta4Flag){
    opt.eta4 = true;
  }

  if (elevatorTestFlag){
    elevatorTest = true;
    if (statsFlag || delayFlag || weightFlag || elevatorFlag || eta4Flag){
      std::cerr << "Wrong combination of arguments" << std::endl;
      return 1;
    }
    opt.sim = false;
    opt.sl = false;
    opt.reach = false;
    opt.succEmptyCheck = false;
    opt.elevator.elevatorRank = true;
    opt.dataFlow = LIGHT;
  }

  if(lightFlag)
  {
    opt.sim = false;
    opt.sl = false;
    opt.reach = false;
    opt.succEmptyCheck = false;
    opt.elevator.elevatorRank = true;
    //opt.dataFlow = LIGHT;
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
      BuchiAutomaton<int, int> renBA = parseRenameBA(os, &ba);

      BuchiAutomaton<int, int> renCompl;
      BuchiAutomaton<StateSch, int> comp;

      // elevator test
      if (elevatorTest){
        ElevatorAutomaton sp(renBA);
        std::cout << "Elevator automaton: " << (sp.isElevator() ? "Yes" : "No") << std::endl;
        os.close();
        return 0;
      }

      try
      {
        complementAutWrap(&renBA, &comp, &renCompl, &stats, opt);
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
      BuchiAutomaton<int, int> renCompl;
      BuchiAutomaton<StateSch, int> compBA;
      BuchiAutomaton<StateGcoBA, int> compGcoBA;

      BuchiAutomaton<int, int> renBuchi;
      GeneralizedCoBuchiAutomaton<int, int> renGcoBA;

      BuchiAutomataParser parser(os);
      AutomatonType autType = parser.parseAutomatonType();

      map<int, APSymbol> symDict;

      try
      {
        if(autType == AUTBA)
        {
          BuchiAutomaton<int, APSymbol> orig = parseRenameHOABA(parser, opt);

          if(orig.isTBA())
          {
            opt.sim = false;
          }

          renBuchi = orig.renameAut();

          if (elevatorTest){
            ElevatorAutomaton sp(renBuchi);
            std::cout << "Elevator automaton: " << (sp.isElevator() ? "Yes" : "No") << std::endl;
            os.close();
            return 0;
          }

          complementAutWrap(&renBuchi, &compBA, &renCompl, &stats, opt);
          // cout << compBA.toGraphwiz() << endl;

          // for (auto t : compBA.getRenameStateMap())
          //   cout << t.first << " " << t.second << endl;
          symDict = Aux::reverseMap(orig.getRenameSymbolMap());
        }
        if(autType == AUTGCOBA)
        {
          GeneralizedCoBuchiAutomaton<int, APSymbol> orig = parseRenameHOAGCOBA(parser);
          renGcoBA = orig.renameAut();
          complementGcoBAWrap(&renGcoBA, &compGcoBA, &renCompl, &stats);
          symDict = Aux::reverseMap(orig.getRenameSymbolMap());
        }
      }
      catch(const ParserException& e)
      {
        os.close();
        cerr << "Parser error:" << endl;
        cerr << "line " << e.getLine() << ": " << e.what() << endl;
        return 2;
      }
      catch (const std::bad_alloc&)
      {
        os.close();
        cerr << "Memory error" << endl;
        return 2;
      }

      //Product with a word
      if(params.checkWord.size() > 0 && autType == AUTBA)
      {
        cout << "Product in Graphwiz:" << endl;
        auto appattern = compBA.getAPPattern();
        pair<APWord, APWord> inf = BuchiAutomataParser::parseHoaInfWord(params.checkWord, appattern);
        auto prefv = inf.first.getVector();
        auto loopv = inf.second.getVector();
        auto debugRename = compBA.renameAlphabet<APSymbol>(symDict);
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
      //cerr << endl << renCompl.toGraphwiz() << endl;
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
