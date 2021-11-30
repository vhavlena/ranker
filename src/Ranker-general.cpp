
#include "Ranker-general.h"

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


GeneralizedCoBuchiAutomaton<int, APSymbol> parseRenameHOAGCOBA(BuchiAutomataParser& parser)
{
  GeneralizedCoBuchiAutomaton<int, APSymbol> hoa = parser.parseHoaGCOBA();
  return hoa;
}

BuchiAutomaton<int, APSymbol> parseRenameHOABA(BuchiAutomataParser& parser, ComplOptions opt)
{
  //BuchiAutomataParser parser(os);
  BuchiAutomaton<int, APSymbol> orig = parser.parseHoaBA();

  if(opt.preprocess != NONE)
  {
    map<APSymbol, int> apint;
    map<int, APSymbol> intap;
    int i = 0;
    for(const APSymbol& s : orig.getAlphabet())
    {
      apint[s] = i;
      intap[i] = s;
      i++;
    }
    BuchiAutomaton<int, int> tmp = orig.renameAlphabet(apint);
    ElevatorAutomaton elev(tmp);

    set<int> fins = tmp.getFinals();
    auto prediwa = [&fins] (SccClassif c) -> bool
    {
      return c.inhWeak && (std::any_of(c.states.begin(), c.states.end(), [&fins](int state){return fins.find(state) != fins.end();}));
    };
    auto preddet = [&fins] (SccClassif c) -> bool
    {
      return c.det && (std::any_of(c.states.begin(), c.states.end(), [&fins](int state){return fins.find(state) != fins.end();}));
    };
    auto predall = [&fins] (SccClassif c) -> bool
    {
      return (std::any_of(c.states.begin(), c.states.end(), [&fins](int state){return fins.find(state) != fins.end();}));
    };
    auto predtri = [&fins] (SccClassif c) -> bool
    {
      return c.states.size() == 1 && (std::any_of(c.states.begin(), c.states.end(), [&fins](int state){return fins.find(state) != fins.end();}));
    };

    if(opt.preprocess == CPHEUR)
    {
      map<int,int> ranks = elev.elevatorRank(false);
      int m = Aux::maxValue(ranks);
      bool isElev = elev.isElevator();
      auto predheur = [&fins, m, isElev] (SccClassif c) -> bool
      {
        if(m >= 7 && isElev)
          return (std::any_of(c.states.begin(), c.states.end(), [&fins](int state){return fins.find(state) != fins.end();}));
        return false;
      };
      tmp = elev.copyPreprocessing(predheur);
    }
    else if(opt.preprocess == CPIWA)
    {
      tmp = elev.copyPreprocessing(prediwa);
    }
    else if(opt.preprocess == CPDET)
    {
      tmp = elev.copyPreprocessing(preddet);
    }
    else if(opt.preprocess == CPALL)
    {
      tmp = elev.copyPreprocessing(predall);
    }
    else if(opt.preprocess == CPTRIVIAL)
    {
      tmp = elev.copyPreprocessing(predtri);
    }

    orig = tmp.renameAlphabet(intap);
  }

  if (opt.accPropagation)
  { 
    map<APSymbol, int> apint;
    map<int, APSymbol> intap;
    int i = 0;
    for(const APSymbol& s : orig.getAlphabet())
    {
      apint[s] = i;
      intap[i] = s;
      i++;
    }
    BuchiAutomaton<int, int> tmp = orig.renameAlphabet(apint);
    ElevatorAutomaton elev(tmp);
    tmp = elev.propagateAccStates();
    orig = tmp.renameAlphabet(intap);
    //std::cerr << tmp.toGraphwiz() << std::endl;
  }

  Simulations sim;
  if(!opt.sim || orig.isTBA())
  {
    auto ranksim = sim.identity(orig);
    orig.setDirectSim(ranksim);
    orig.setOddRankSim(ranksim);
  }
  else
  {
    auto ranksim = sim.directSimulation<int, APSymbol>(orig, -1);
    orig.setDirectSim(ranksim);

    auto cl = set<int>();
    orig.computeRankSim(cl);
  }

  return orig;
}


BuchiAutomaton<int, int> parseRenameBA(ifstream& os, BuchiAutomaton<string, string>* orig)
{
  BuchiAutomataParser parser(os);
  *orig = BuchiAutomaton<string, string>(parser.parseBaFormat(os));
  Simulations sim;

  auto ranksim = sim.directSimulation<string, string>(*orig, "-1");
  orig->setDirectSim(ranksim);
  auto cl = set<std::string>();

  orig->computeRankSim(cl);
  return orig->renameAut();
}

void complementAutWrap(BuchiAutomaton<int, int>* ren, BuchiAutomaton<StateSch, int>* complOrig, BuchiAutomaton<int, int>* complRes, Stat* stats, ComplOptions &opt)
{
    //TODO
    // rename automaton
    map<int, int> id;
    for(auto al : ren->getAlphabet())
      id[al] = al;
    BuchiAutomaton<int, int> renptr = ren->renameAutDict(id);
    renptr.removeUseless();
    renptr = renptr.renameAutDict(id);

    // ElevatorAutomaton elev(renptr);
    // cout << renptr.toGraphwiz() << endl;
    // renptr = elev.copyPreprocessing();
    // cout << renptr.toGraphwiz() << endl;

    BuchiAutomatonSpec sp(&renptr); //TODO


    // ComplOptions opt = { .cutPoint = true, .succEmptyCheck = true, .ROMinState = 8,
    //     .ROMinRank = 6, .CacheMaxState = 6, .CacheMaxRank = 8, .semidetOpt = false };
    sp.setComplOptions(opt);
    BuchiAutomaton<StateSch, int> comp;

    comp = sp.complementSchReduced(ren->getFinals(), stats);

    BuchiAutomatonDelay<int> compDelay(comp);
    *complOrig = comp;

    stats->generatedStates = comp.getStates().size();
    stats->generatedTrans = comp.getTransCount();
    stats->generatedTransitionsToTight = compDelay.getTransitionsToTight();

    // rename automaton
    //map<int, int> id;
    //for(auto al : comp.getAlphabet())
    //  id[al] = al;
    BuchiAutomaton<int, int> renCompl = comp.renameAutDict(id);
    renCompl.removeUseless();
    renCompl = renCompl.renameAutDict(id);

    stats->reachStates = renCompl.getStates().size();
    stats->reachTrans = renCompl.getTransCount();
    stats->engine = "Ranker";
    stats->transitionsToTight = -1;
    // stats->elevator = elev.isElevator(); // original automaton before complementation
    // stats->elevatorStates = elev.elevatorStates();
    stats->originalStates = sp.getStates().size();
    *complRes = renCompl;
}

void complementGcoBAWrap(GeneralizedCoBuchiAutomaton<int, int> *ren, BuchiAutomaton<StateGcoBA, int> *complOrig, BuchiAutomaton<int, int>* complRes, Stat* stats){
  //ren->removeUseless();
  //std::cerr << ren->toGraphwiz() << std::endl;
  GeneralizedCoBuchiAutomatonCompl sp(ren);

  *complOrig = sp.complementGcoBA();

  stats->generatedStates = complOrig->getStates().size();
  stats->generatedTrans = complOrig->getTransCount();

  map<int, int> id;
  for(auto al : complOrig->getAlphabet())
    id[al] = al;
  //std::cerr << complOrig->toString() << std::endl;
  BuchiAutomaton<int, int> renCompl = complOrig->renameAutDict(id);
  renCompl.removeUseless();
  renCompl = renCompl.renameAutDict(id);

  stats->reachStates = renCompl.getStates().size();
  stats->reachTrans = renCompl.getTransCount();
  stats->engine = "Ranker";
  stats->transitionsToTight = -1;
  stats->originalStates = sp.getStates().size();
  *complRes = renCompl;
}

void complementScheweAutWrap(BuchiAutomaton<int, int>* ren, BuchiAutomaton<int, int>* complRes, Stat* stats, bool delay, double w)
{
    BuchiAutomatonSpec sp(ren);
    ElevatorAutomaton elev(*ren);

    ComplOptions opt = { .cutPoint = true, .CacheMaxState = 6, .CacheMaxRank = 8,
        .semidetOpt = false };
    sp.setComplOptions(opt);
    BuchiAutomaton<StateSch, int> comp;
    comp = sp.complementSchOpt(delay, ren->getFinals(), w, stats);
    BuchiAutomatonDelay<int> compDelay(comp);

    stats->generatedStates = comp.getStates().size();
    stats->generatedTrans = comp.getTransCount();
    stats->generatedTransitionsToTight = compDelay.getTransitionsToTight();

    map<int, int> id;
    for(auto al : comp.getAlphabet())
      id[al] = al;
    BuchiAutomaton<int, int> renCompl = comp.renameAutDict(id);
    renCompl.removeUseless();

    stats->reachStates = renCompl.getStates().size();
    stats->reachTrans = renCompl.getTransCount();
    stats->engine = "Ranker";
    stats->elevator = elev.isElevator(); // original automaton before complementation
    stats->elevatorStates = elev.elevatorStates();
    stats->originalStates = sp.getStates().size();
    *complRes = renCompl;
}

BuchiAutomaton<int, int> createBA(vector<int>& loop)
{
  set<int> states = {0};
  set<int> ini = {0};
  set<int> fins = {0};
  BuchiAutomaton<int, int>::Transitions trans;
  for(int i = 0; i < (int)loop.size(); i++)
  {
    if(i == (int)loop.size() - 1)
    {
      trans[{i, loop[i]}] = set<int>({0});
    }
    else
    {
      trans[{i, loop[i]}] = set<int>({i+1});
      states.insert(i+1);
    }
  }
  return BuchiAutomaton<int, int>(states, fins, ini, trans);
}


void printStat(Stat& st)
{
  cerr << "Generated-states: " << st.generatedStates << "\nGenerated-trans: " << st.generatedTrans << endl;
  cerr << "States: " << st.reachStates << "\nTransitions: " << st.reachTrans << endl;
  cerr << "Generated-transitions-to-tight: " << st.generatedTransitionsToTight << endl;
  //cerr << "Transitions to tight: " << st.transitionsToTight << endl;
  cerr << "Elevator-automaton: " << (st.elevator ? "Yes" : "No") << endl;
  cerr << "States-before-complementation: " << st.originalStates << endl;
  cerr << "Elevator-states: " << st.elevatorStates << endl;
  cerr << "Engine: " << st.engine << endl;
  cerr << std::fixed;
  cerr << std::setprecision(2);

  float duration = (float)(st.duration/1000.0);
  float rest = duration;
  cerr << "Time: " << (float)(st.duration/1000.0) << endl;
  if (st.duration/1000.0 != 0.0){
    cerr << "Waiting-part: " << (float)(st.waitingPart/1000.0) << " " << ((float)(st.waitingPart/1000.0)*100)/duration << "%" << endl;
    rest -= (float)(st.waitingPart/1000.0);
    cerr << "Rank-bound: " << (float)(st.rankBound/1000.0) << " " << ((float)(st.rankBound/1000.0)*100)/duration << "%" << endl;
    rest -= (float)(st.rankBound/1000.0);
    if (st.elevatorRank != -1){
      cerr << "Elevator-rank: " << (float)(st.elevatorRank/1000.0) << " " << ((float)(st.elevatorRank/1000.0)*100)/duration << "%" << endl;
      rest -= (float)(st.elevatorRank/1000.0);
    }
    cerr << "Start-of-tight-part: " << (float)(st.cycleClosingStates/1000.0) << " " << ((float)(st.cycleClosingStates/1000.0)*100)/duration << "%" << endl;
    rest -= (float)(st.cycleClosingStates/1000.0);
    if (st.getAllCycles != -1){
      // delay
      cerr << "\tGet-all-cycles: " << (float)(st.getAllCycles/1000.0) << " " << ((float)(st.getAllCycles/1000.0)*100)/duration << "%" << endl;
      cerr << "\tStates-to-generate: " << (float)(st.statesToGenerate/1000.0) << " " << ((float)(st.statesToGenerate/1000.0)*100)/duration << "%" << endl;
    }
    cerr << "Simulations: " << (float)(st.simulations/1000.0) << " " << ((float)(st.simulations/1000.0)*100)/duration << "%" << endl;
    rest -= (float)(st.simulations/1000.0);
    cerr << "Tight-part-construction: " << (float)(st.tightPart/1000.0) << " " << ((float)(st.tightPart/1000.0)*100)/duration << "%" << endl;
    rest -= (float)(st.tightPart/1000.0);
    cerr << "Rest: " << rest << " " << (rest*100.0)/duration << "%" << endl;
  }
}

std::string getHelpMsg(const std::string& progName)
{
	std::string helpMsg;
	helpMsg += "Usage: \n";
  helpMsg += "1) Complementation:\n";
  helpMsg += "  " + progName + " [--stats] [--delay VERSION [-w WEIGHT]] [--elevator-rank] [--eta4] INPUT\n";
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
	helpMsg += "  --stats             Print summary statistics\n";
  helpMsg += "  --delay             Use delay optimization\n";
  helpMsg += "  VERSION             --old / --new / --random / --subset / --stirling\n";
  helpMsg += "  WEIGHT              Weight parameter - in <0,1>\n";
  helpMsg += "  --elevator-rank     Update rank upper bound of each macrostate based on elevator automaton structure";
  helpMsg += "  --eta4              Max rank optimization - eta 4 only when going from some accepting state";
  helpMsg += "  --check=<word>      Product of the complementary automaton with the word\n";
  helpMsg += "\n\n";
  helpMsg += "2) Tests if INPUT is an elevator automaton\n";
  helpMsg += "  " + progName + " --elevator-test INPUT\n";

	return helpMsg;
} // getHelpMsg
