
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


void complementAutWrap(BuchiAutomaton<int, int>& ren, BuchiAutomaton<int, int>* complRes, Stat* stats, bool delay, double w, delayVersion version, bool elevatorRank)
{
  BuchiAutomatonSpec sp(ren);
  ComplOptions opt = { .cutPoint = true, .succEmptyCheck = true, .ROMinState = 8,
      .ROMinRank = 6, .CacheMaxState = 6, .CacheMaxRank = 8, .semidetOpt = false };
  sp.setComplOptions(opt);
  BuchiAutomaton<StateSch, int> comp;
  comp = sp.complementSchReduced(delay, ren.getFinals(), w, version, elevatorRank);

  stats->generatedStates = comp.getStates().size();
  stats->generatedTrans = comp.getTransCount();

  map<int, int> id;
  for(auto al : comp.getAlphabet())
    id[al] = al;
  BuchiAutomaton<int, int> renCompl = comp.renameAutDict(id);
  renCompl.removeUseless();
  renCompl = renCompl.renameAutDict(id);

  // cout << renCompl.getStates().size();
  // map<int, StateSch> symDict = Aux::reverseMap(comp.getRenameStateMap());
  // auto tmp = renCompl.getComplStructure(symDict);
  //cout << tmp.toGraphwiz() << endl;

  // auto prod = renCompl.productBA(ren);
  // BuchiAutomaton<int, int> renProd = prod.renameAutDict(id);
  // if(!renProd.isEmpty())
  //   cout << "FALSE" << endl;
  // else
  //   cout << "TRUE" << endl;

  // vector<int> word = {0,0,0,0,0,0,0,1,0,0,1};
  // auto wb = createBA(word);
  // cout << "HERE" << endl;
  // cout << wb.toGraphwiz() << endl << endl;
  // auto a = tmp.productBA(wb);
  // cout << a.toGraphwiz() << endl;
  // //set<int> fin = renCompl.getFinals();
  // for(auto t : tmp.getRunTree(word))
  // {
  //   for(auto m : t)
  //   {
  //     cout << m.toString() << " ";
  //   }
  //   cout << endl;
  // }

  stats->reachStates = renCompl.getStates().size();
  stats->reachTrans = renCompl.getTransCount();
  stats->engine = "Ranker";
  stats->transitionsToTight = comp.getTransitionsToTight();
  stats->elevator = ren.isElevator(); // original automaton before complementation
  *complRes = renCompl;
}


void complementScheweAutWrap(BuchiAutomaton<int, int>& ren, BuchiAutomaton<int, int>* complRes, Stat* stats, bool delay, double w, delayVersion version)
{
  BuchiAutomatonSpec sp(ren);
  ComplOptions opt = { .cutPoint = true, .CacheMaxState = 6, .CacheMaxRank = 8,
      .semidetOpt = false };
  sp.setComplOptions(opt);
  BuchiAutomaton<StateSch, int> comp;
  comp = sp.complementSchOpt(delay, ren.getFinals(), w, version);

  stats->generatedStates = comp.getStates().size();
  stats->generatedTrans = comp.getTransCount();

  map<int, int> id;
  for(auto al : comp.getAlphabet())
    id[al] = al;
  BuchiAutomaton<int, int> renCompl = comp.renameAutDict(id);
  renCompl.removeUseless();

  stats->reachStates = renCompl.getStates().size();
  stats->reachTrans = renCompl.getTransCount();
  stats->engine = "Ranker";
  stats->transitionsToTight = comp.getTransitionsToTight();
  stats->elevator = ren.isElevator(); // original automaton before complementation
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
  cerr << "Generated states: " << st.generatedStates << "\nGenerated trans: " << st.generatedTrans << endl;
  cerr << "States: " << st.reachStates << "\nTransitions: " << st.reachTrans << endl;
  cerr << "Generated transitions to tight: " << st.transitionsToTight << endl;
  cerr << "Elevator automaton: " << (st.elevator ? "Yes" : "No") << endl;
  cerr << "Engine: " << st.engine << endl;
  cerr << std::fixed;
  cerr << std::setprecision(2);
  cerr << "Time: " << (float)(st.duration/1000.0) << std::endl;
}

std::string getHelpMsg(const std::string& progName)
{
	std::string helpMsg;
	helpMsg += "Usage: \n";
  helpMsg += "1) Complementation:\n";
  helpMsg += "  " + progName + " [--stats] [--delay VERSION [-w WEIGHT]] [--elevator-rank] INPUT\n";
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
  helpMsg += "  VERSION             --old / --new / --random / --subset\n";
  helpMsg += "  WEIGHT              Weight parameter - in <0,1>\n";
  helpMsg += "  --elevator-rank     Update rank upper bound of each macrostate based on elevator automaton structure";
  helpMsg += "\n";
  helpMsg += "2) Tests if INPUT is an elevator automaton\n";
  helpMsg += "  " + progName + " --elevator-test INPUT\n";

	return helpMsg;
} // getHelpMsg
