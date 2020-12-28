
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


void complementAutWrap(BuchiAutomaton<int, int>& ren, BuchiAutomaton<int, int>* complRes, Stat* stats)
{
  BuchiAutomatonSpec sp(ren);
  ComplOptions opt = { .cutPoint = true, .succEmptyCheck = true, .ROMinState = 8, .ROMinRank = 6, .CacheMaxState = 6, .CacheMaxRank = 8 };
  sp.setComplOptions(opt);
  BuchiAutomaton<StateSch, int> comp;
  comp = sp.complementSchReduced();

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
  *complRes = renCompl;
}


void complementScheweAutWrap(BuchiAutomaton<int, int>& ren, BuchiAutomaton<int, int>* complRes, Stat* stats)
{
  BuchiAutomatonSpec sp(ren);
  ComplOptions opt = { .cutPoint = true, .CacheMaxState = 6, .CacheMaxRank = 8 };
  sp.setComplOptions(opt);
  BuchiAutomaton<StateSch, int> comp;
  comp = sp.complementSchOpt();

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
  *complRes = renCompl;
}


void printStat(Stat& st)
{
  cerr << "Generated states: " << st.generatedStates << "\nGenerated trans: " << st.generatedTrans << endl;
  cerr << "States: " << st.reachStates << "\nTransitions: " << st.reachTrans << endl;
  cerr << "Engine: " << st.engine << endl;
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
