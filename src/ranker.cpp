
#include <cstdlib>
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <chrono>
#include <iomanip>
#include <stdio.h>
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
#include "Complement/CoBuchiCompl.h"
#include "Complement/SemiDeterministicCompl.h"

extern const char *gitversion;
extern const char *gitdescribe;
const string tmpFile = "__tmp__ranker_639652108";

using namespace std;

int main(int argc, char *argv[])
{
  Params params = { .output = "", .input = "", .tmpFile = false, .stats = false, .checkWord = ""};
  ifstream os;
  //bool error = false;
  bool elevatorTest = false;
  //elevatorOptions elevator = {.elevatorRank = false, .detBeginning = false};

  args::ArgumentParser parser("Program complementing a (state-based acceptance condition) Buchi automaton.\n", "");
  args::HelpFlag help(parser, "help", "Display this help menu", {'h', "help"});

  args::Positional<std::string> inputFile(parser, "INPUT", "The name of a file in the HOA (Hanoi Omega Automata) or the BA format\n");
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
  args::ValueFlagList<std::string> preprocessFlag(parser, "value", "Preprocessing [copyiwa/copydet/copyall/copytrivial/copyheur/accsat/no-red]", {"preprocess"});
  args::Flag sdFlag(parser, "sd", "Use semideterminization", {"sd"});
  args::Flag iwSimFlag(parser, "iw-sim", "Use direct simulation", {"iw-sim"});
  args::Flag iwSatFlag(parser, "iw-sat", "Macrostates saturation", {"iw-sat"});
  args::Flag nobackoffFlag(parser, "no-backoff", "Do NOT use backoff", {"no-backoff"});
  args::Flag versionFlag(parser, "version", "Git commit version", {"version"});
  args::Flag sdVersionFlag(parser, "ncsb-lazy", "Use NCSB-lazy for SD complementation", {"ncsb-lazy"});
  args::Flag tbaFlag(parser, "no-tba", "Do NOT use TBA preprocessing", {"no-tba"});
  args::Flag bestFlag(parser, "best", "Use the settings leading to smallest possible automata", {"best"});
  args::Flag slFlag(parser, "sl", "Use self-loop waiting optimization", {"sl"});

  args::Flag iwOrigOnlyFlag(parser, "iw-orig-only", "Use original IW procedure only", {"iw-orig-only"});
  args::Flag iwPruneOnlyFlag(parser, "iw-prune-only", "Use pruning optimization in IW complementation", {"iw-prune-only"});

  args::Flag sdLazyOnlyFlag(parser, "sd-ncsb-lazy-only", "Use NCSB-Lazy procedure only", {"sd-ncsb-lazy-only"});
  args::Flag sdMaxrankOnlyFlag(parser, "sd-ncsb-maxrank-only", "Use NCSB-MaxRank procedure only", {"sd-ncsb-maxrank-only"});

  ComplOptions opt = { .cutPoint = true, .succEmptyCheck = false, .ROMinState = 8,
      .ROMinRank = 6, .CacheMaxState = 6, .CacheMaxRank = 8, .semidetOpt = false,
      .dataFlow = INNER, .delay = false, .delayVersion = oldVersion, .delayW = 0.5,
      .debug = false, .elevator = { .elevatorRank = true, .detBeginning = false },
      .dirsim = true, .ranksim = true, .sl = true, .reach = true, .flowDirSim = false, .preprocess = CPHEUR, .accPropagation = false,
      .semideterminize = false, .backoff = true, .BOBound = { {11,15}, {11,13} },
      .semideterministic = false, .complete = false, .lowrankopt = false,
      .iwSim = true, .iwSat = false, .ncsbLazy = false, .tba = true, .light = false,
      .prered = true, .postred = false, .iwOrigOnly = false, .iwPruneOnly = false,
      .sdLazyOnly = false, .sdMaxrankOnly = false};

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
  if (inputFile && args::get(inputFile) != ""){
    params.input = args::get(inputFile);
  }
  else
  {
    ofstream tmpStream;
    tmpStream.open(tmpFile, ofstream::out);

    if(!tmpStream)
    {
      std::cerr << "Error during creating a temporal file" << endl;
      return 1;
    }

    string line;
    while (getline(cin, line))
    {
       tmpStream << line << endl;
    }
    tmpStream.close();
    params.input = tmpFile;
    params.tmpFile = true;
  }

  if(versionFlag)
  {
    cout << gitversion << endl;
    return 0;
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

  if(nobackoffFlag)
  {
    opt.backoff = false;
  }

  if(preprocessFlag)
  {
    for(const auto& fl : args::get(preprocessFlag))
    {
      if(fl == "copyiwa")
        opt.preprocess = CPIWA;
      else if(fl == "copydet")
        opt.preprocess = CPDET;
      else if(fl == "copyall")
        opt.preprocess = CPALL;
      else if(fl == "copytrivial")
        opt.preprocess = CPTRIVIAL;
      else if(fl == "copyheur")
        opt.preprocess = CPHEUR;
      else if(fl == "accsat")
        opt.accPropagation = true;
      else if(fl == "no-red")
        opt.prered = false;
      else {
        std::cerr << "Wrong copy attribute" << std::endl;
        return 1;
      }
    }
  }

  if(sdFlag)
  {
    opt.semideterminize = true;
  }

  if (iwSimFlag)
  {
    opt.iwSim = true;
  }
  else if (iwSatFlag)
  {
    opt.iwSat = true;
  }

  if (sdVersionFlag){
    opt.ncsbLazy = true;
  }

  if(tbaFlag)
  {
    opt.tba = false;
  }

  if(iwOrigOnlyFlag)
  {
    opt.iwOrigOnly = true;
  }
  if(iwPruneOnlyFlag)
  {
    opt.iwPruneOnly = true;
  }
  if(sdMaxrankOnlyFlag)
  {
    opt.sdMaxrankOnly = true;
  }
  if(sdLazyOnlyFlag)
  {
    opt.sdLazyOnly = true;
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
    opt.dirsim = false;
    opt.ranksim = false;
    opt.sl = false;
    opt.reach = false;
    opt.elevator.elevatorRank = true;
    opt.dataFlow = LIGHT;
  }

  if(lightFlag)
  {
    opt.light = true;
  }


  string filename(params.input);
  os.open(filename);

  if(os)
  { // file opened correctly
    InFormat fmt = parseRenamedAutomaton(os);

    Stat stats;
    stats.beginning = std::chrono::high_resolution_clock::now();

    BuchiAutomaton<int, int> renCompl;
    BuchiAutomaton<StateSch, int> compBA;
    BuchiAutomaton<StateGcoBA, int> compGcoBA;

    BuchiAutomaton<int, int> renBuchi;
    GeneralizedCoBuchiAutomaton<int, int> renGcoBA;

    BuchiAutomataParser parser(os);

    AutomatonType autType = AUTBA;
    if(fmt == HOA)
      autType = parser.parseAutomatonType();

    map<int, APSymbol> symDict;

    try
    {
      if(autType == AUTBA)
      {
        BuchiAutomaton<int, APSymbol> orig = parseRenameHOABA(parser, opt, fmt);

        if(orig.isTBA())
        {
          opt.ranksim = false;
        }

        if(orig.getStates().size() >= 20)
        {
          opt.reach = false;
          opt.dirsim = false;
          opt.ranksim = false;
          opt.sl = false;
        }

        if(slFlag || fmt == BA)
        {
          opt.sl = true;
        }

        Simulations sim;
        if(!opt.dirsim)
        {
          auto ranksim = sim.identity(orig);
          orig.setDirectSim(ranksim);
          orig.setOddRankSim(ranksim);
        }
        else
        {
          auto ranksim = sim.directSimulation<int, APSymbol>(orig, -1);
          orig.setDirectSim(ranksim);

          if(!opt.ranksim)
          {
            auto ranksim = sim.identity(orig);
            orig.setOddRankSim(ranksim);
          }
          else
          {
            auto cl = set<int>();
            orig.computeRankSim(cl);
          }
        }

        renBuchi = orig.renameAut();

        ElevatorAutomaton el(renBuchi);
        if (elevatorTest){
          cout << "Elevator automaton: " << (el.isElevator() ? "Yes" : "No") << endl;
          cout << "Elevator states: " << el.elevatorStates() << endl;
          os.close();
          if (params.tmpFile) std::remove(params.input.c_str());
          return 0;
        }

        map<int,int> ranks = el.elevatorRank(false);
        int m = Aux::maxValue(ranks);

        if(renBuchi.isDeterministic())
        {
          opt.reach = false;
          opt.sl = false;
        }
        else if(m <= 2)
        {
          opt.sl = false;
        }
        else if(m <= 4)
        {
          opt.lowrankopt = true;
          opt.cutPoint = false;
          opt.sl = false;
        }

        if(renBuchi.isComplete())
        {
          opt.complete = true;
        }

        BuchiAutomatonSpec sp(&renBuchi);
        sp.setComplOptions(opt);

        if(opt.backoff && !el.isInherentlyWeakBA())
        {
          BuchiAutomaton<StateSch, int> comp = sp.complementSchNFA(sp.getInitials());
          sp.computeRankBound(comp, &stats);

          if(sp.meetsBackOff() && fmt == HOA)
          {
            if (params.input != "") os.close();
            const char* env = std::getenv("SPOTEXE");
            if(env == NULL)
            {
              cerr << "Error: $SPOTEXE not found" << endl;
              return 1;
            }
            const string spotpath_cstr = string(env);
            string cmd = spotpath_cstr + " --complement --ba " + filename;
            string ret = "";

            try
            {
              ret = Simulations::execCmd(cmd);
            }
            catch(const char* c)
            {
              cerr << "Error occurred: " << c << endl;
              if(ret.size() > 0)
                cerr << ret << endl;
              return 1;
            }

            stringstream strm(ret);
            BuchiAutomataParser spotpar(strm);
            BuchiAutomaton<int, APSymbol> spotaut = spotpar.parseHoaBA();

            stats.engine = "SPOT";
            stats.duration = 0;
            stats.generatedStates = 0;
            stats.generatedTrans = 0;
            stats.reachStates = spotaut.getStates().size();
            stats.reachTrans = spotaut.getTransCount();
            if(params.stats)
              printStat(stats);

            cout << spotaut.toHOA() << endl;
            return 0;
          }
        }

        sp.setComplOptions(opt);

        if(el.isInherentlyWeakBA())
        {
          CoBuchiAutomatonCompl iw(el);
          complementCoBAWrap(&iw, &compGcoBA, &renCompl, &stats, opt);
        }
        else if (el.isSemiDeterministic())
        {
          SemiDeterministicCompl sd(&renBuchi);
          BuchiAutomaton<int, int> renComplSD;
          complementSDWrap(sd, &renBuchi, &renComplSD, &stats, opt);

          if(!opt.light && !opt.sdLazyOnly && !opt.sdMaxrankOnly)
          {
            Stat s1 = stats;

            complementAutWrap(sp, &renBuchi, &compBA, &renCompl, &stats, !opt.backoff);

            if(renComplSD.getStates().size() <= renCompl.getStates().size())
            {
              stats = s1;
              renCompl = renComplSD;
            }
          }
          else
          {
            renCompl = renComplSD;
          }
        }
        else
        {
          complementAutWrap(sp, &renBuchi, &compBA, &renCompl, &stats, !opt.backoff);
          // cout << compBA.toGraphwiz() << endl;
        }

        symDict = Aux::reverseMap(orig.getRenameSymbolMap());
      }
      if(autType == AUTGCOBA)
      {
        GeneralizedCoBuchiAutomaton<int, APSymbol> orig = parseRenameHOAGCOBA(parser);
        renGcoBA = orig.renameAut();
        complementGcoBAWrap(&renGcoBA, &compGcoBA, &renCompl, &stats);
        symDict = Aux::reverseMap(orig.getRenameSymbolMap());
      }
      if (autType != AUTBA and autType != AUTGCOBA){
        std::cerr << "Error: Unsupported automaton type" << std::endl;
        return 2;
      }
    }
    catch(const ParserException& e)
    {
      os.close();
      if (params.tmpFile) std::remove(params.input.c_str());
      cerr << "Parser error:" << endl;
      cerr << "line " << e.getLine() << ": " << e.what() << endl;
      return 2;
    }
    catch (const std::bad_alloc&)
    {
      os.close();
      if (params.tmpFile) std::remove(params.input.c_str());
      cerr << "Memory error" << endl;
      return 2;
    }

    //Product with a word
    if(params.checkWord.size() > 0)
    {
      cout << "Product in Graphwiz:" << endl;
      auto appattern = renBuchi.getAPPattern();
      pair<APWord, APWord> inf = BuchiAutomataParser::parseHoaInfWord(params.checkWord, appattern);
      auto prefv = inf.first.getVector();
      auto loopv = inf.second.getVector();

      if(compBA.getStates().size() > 0)
      {
        auto debugRename = compBA.renameAlphabet<APSymbol>(symDict);
        BuchiAutomatonDebug<StateSch, APSymbol> compDebug(debugRename);
        auto ret = compDebug.getSubAutomatonWord(prefv, loopv);
        cout << ret.toGraphwiz() << endl;
      }
      if(compGcoBA.getStates().size() > 0)
      {
        auto debugRename = compGcoBA.renameAlphabet<APSymbol>(symDict);
        BuchiAutomatonDebug<StateGcoBA, APSymbol> compDebug(debugRename);
        auto ret = compDebug.getSubAutomatonWord(prefv, loopv);
        cout << ret.toGraphwiz() << endl;
      }

      os.close();
      if (params.tmpFile) std::remove(params.input.c_str());
      return 0;
    }

    if(opt.postred)
    {
      Simulations sim;
      auto dirsim = sim.directSimulation<int, int>(renCompl, -1);
      renCompl.setDirectSim(dirsim);
      renCompl = renCompl.reduce();
    }

    BuchiAutomaton<int, APSymbol> outOrig = renCompl.renameAlphabet<APSymbol>(symDict);

    if(fmt == HOA)
    {
      outOrig.completeAPComplement();
    }
    stats.reachStates = outOrig.getStates().size();
    stats.reachTrans = outOrig.getTransCount();

    stats.end = std::chrono::high_resolution_clock::now();
    stats.duration = std::chrono::duration_cast<std::chrono::milliseconds>(stats.end - stats.beginning).count();

    if(params.stats)
      printStat(stats);
    cout << outOrig.toHOA() << endl;
    //cerr << endl << renCompl.toGraphwiz() << endl;
  }
  else
  { // file cannot be opened
		std::cerr << "Cannot open file \"" + filename + "\"\n";
		return 1;
	}

  os.close();
  if (params.tmpFile) std::remove(params.input.c_str());
  return 0;
}
