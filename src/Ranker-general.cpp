
#include "Ranker-general.h"

InFormat parseRenamedAutomaton(istream& os)
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

BuchiAutomaton<int, APSymbol> parseRenameHOABA(BuchiAutomataParser& parser, ComplOptions opt, InFormat fmt)
{
  //BuchiAutomataParser parser(os);

  BuchiAutomaton<int, APSymbol> orig;

  if(fmt == HOA)
  {
    orig = parser.parseHoaBA();
  }
  else
  {
    BuchiAutomaton<string, string> ba = parser.parseBaFormat();
    BuchiAutomaton<int, int> renBA = ba.renameAut();
    map<int, string> symDict = Aux::reverseMap(ba.getRenameSymbolMap());
    orig = renBA.toAPBA(symDict);
  }

  if(opt.prered)
  {
    Simulations sim;
    auto dirsim = sim.directSimulation<int, APSymbol>(orig, -1);
    orig.setDirectSim(dirsim);
    orig = orig.reduce();
  }

  map<APSymbol, int> apint;
  map<int, APSymbol> intap;
  int i = 0;
  for(const APSymbol& s : orig.getAlphabet())
  {
    apint[s] = i;
    intap[i] = s;
    i++;
  }

  if(orig.isSemiDeterministic())
  {
    //opt.accPropagation = true;
    //assume states numbered from 0 (no gaps)
    //orig.complete(orig.getStates().size());
  }

  if(opt.semideterminize)
  {
    auto sd = orig.semideterminize();
    //cout << sd.toGraphwiz() << endl;
    auto rn = sd.renameStates();
    BuchiAutomaton<int, int> tmp = rn.renameAlphabet(apint);
    //tmp.removeUseless();
    orig = tmp.renameAlphabet(intap);
  }


  BuchiAutomaton<int, int> tmp = orig.renameAlphabet(apint);
  ElevatorAutomaton elev(tmp);

  if(opt.preprocess != NONE)
  {
    set<int> fins = tmp.getFinals();
    VecTrans<int, int> finsTrans = tmp.getFinTrans();

    auto isAcc = [&fins, &finsTrans] (SccClassif c) -> bool
    {
      if(std::any_of(c.states.begin(), c.states.end(), [&fins](int state){return fins.find(state) != fins.end();}))
        return true;
      for(auto& tr : finsTrans)
      {
        if(c.states.find(tr.from) != c.states.end() && c.states.find(tr.to) != c.states.end())
          return true;
      }
      return false;
    };

    auto prediwa = [&isAcc] (SccClassif c) -> bool
    {
      return c.inhWeak && isAcc(c);
    };
    auto preddet = [&isAcc] (SccClassif c) -> bool
    {
      return c.det && isAcc(c);
    };
    auto predall = [&isAcc] (SccClassif c) -> bool
    {
      return isAcc(c);
    };
    auto predtri = [&isAcc] (SccClassif c) -> bool
    {
      return c.states.size() == 1 && isAcc(c);
    };

    if(opt.preprocess == CPHEUR)
    {
      map<int,int> ranks = elev.elevatorRank(false);
      int m = Aux::maxValue(ranks);
      bool isElev = elev.isElevator() && !elev.isInherentlyWeakBA();

      auto predheur = [&isAcc, m, isElev] (SccClassif c) -> bool
      {
        if(m >= 5 && isElev)
          return isAcc(c);
        return false;
      };
      tmp = elev.copyPreprocessing(predheur);

      if(isElev)
      {
        ElevatorAutomaton elevPost(tmp);
        tmp = elevPost.nondetInitDeterminize();

        Simulations sim;
        auto ranksim = sim.directSimulation<int, int>(tmp, -1);

        //cout << tmp.toGraphwiz() << endl;
        // set<set<int>> eqcl = Aux::getEqClasses(ranksim, tmp.getStates());
        // for(const auto& cl : eqcl)
        // {
        //   cout << Aux::printIntSet(cl) << endl;
        // }

        tmp.setDirectSim(ranksim);
        tmp = tmp.reduce();
      }
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

    //cout << orig.toHOA() << endl;
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

  //std::cerr << orig.getFinTrans().size() << std::endl;

  if(opt.tba)
  {
    auto tba = orig.toTBA();
    ///
    if (orig.isTBA())
      tba = orig;
    ///
    auto renTba = tba.renameAut();
    ElevatorAutomaton el(renTba);

    if(tba.getStates().size() < orig.getStates().size() && (tba.isSemiDeterministic() || el.isInherentlyWeakBA()))
    {
      orig = tba;
    }
  }

  //std::cerr << orig.getFinTrans().size() << std::endl;
  return orig;
}

void complementAutWrap(BuchiAutomatonSpec& sp, BuchiAutomaton<int, int>* ren,
    BuchiAutomaton<StateSch, int>* complOrig, BuchiAutomaton<int, int>* complRes,
    Stat* stats, bool boundUpdate)
{
    map<int, int> id;
    for(auto al : ren->getAlphabet())
      id[al] = al;

    BuchiAutomaton<StateSch, int> comp;

    comp = sp.complementSchReduced(stats, boundUpdate);

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

void complementGcoBAWrap(GeneralizedCoBuchiAutomaton<int, int> *ren, BuchiAutomaton<StateGcoBA, int> *complOrig, BuchiAutomaton<int, int>* complRes, Stat* stats)
{
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

void complementCoBAWrap(CoBuchiAutomatonCompl *ren, BuchiAutomaton<StateGcoBA, int> *complOrig, BuchiAutomaton<int, int>* complRes, Stat* stats, ComplOptions opt)
{
  auto pure = ren->complementCoBA();
  auto complSim = ren->complementCoBASim(opt);

  map<int, int> id;
  for(auto al : pure.getAlphabet())
    id[al] = al;

  BuchiAutomaton<int, int> renSim = complSim.renameAutDict(id);
  renSim.removeUseless();
  renSim = renSim.renameAutDict(id);

  if(opt.iwOrigOnly)
  {
    BuchiAutomaton<int, int> renPure = pure.renameAutDict(id);
    renPure.removeUseless();
    renPure = renPure.renameAutDict(id);
    *complOrig = pure;
    *complRes = renPure;
  }
  else if(opt.iwPruneOnly)
  {
    *complOrig = complSim;
    *complRes = renSim;
  }
  else
  {
    if(!opt.light)
    {
      BuchiAutomaton<int, int> renPure = pure.renameAutDict(id);
      renPure.removeUseless();
      renPure = renPure.renameAutDict(id);

      if(renSim.getStates().size() > renPure.getStates().size())
      {
        *complOrig = pure;
        *complRes = renPure;
      }
      else
      {
        *complOrig = complSim;
        *complRes = renSim;
      }
    }
    else
    {
      *complOrig = complSim;
      *complRes = renSim;
    }
  }

  stats->generatedStates = complOrig->getStates().size();
  stats->generatedTrans = complOrig->getTransCount();

  stats->reachStates = complRes->getStates().size();
  stats->reachTrans = complRes->getTransCount();
  stats->engine = "Ranker";
  stats->transitionsToTight = -1;
  stats->originalStates = ren->getStates().size();
}

void complementSDWrap(SemiDeterministicCompl& sp, BuchiAutomaton<int, int>* ren, BuchiAutomaton<int, int>* complRes, Stat* stats, ComplOptions opt)
{
  BuchiAutomaton<StateSD, int> compOrig;
  BuchiAutomaton<StateSD, int> compLazy;

  ComplOptions optP = opt;
  optP.ncsbLazy = false;
  compOrig = sp.complementSD(optP);



  map<int, int> id;
  for(auto al : ren->getAlphabet())
    id[al] = al;

  //Simulations sim;
  BuchiAutomaton<int, int> renComplOrig = compOrig.renameAutDict(id);
  renComplOrig = renComplOrig.removeUselessRename();

  BuchiAutomaton<int, int> renComplLazy;
  if(opt.ncsbLazy || opt.sdLazyOnly)
  {
    opt.ncsbLazy = true;
    compLazy = sp.complementSD(opt);
    renComplLazy = compLazy.renameAutDict(id);
    renComplLazy = renComplLazy.removeUselessRename();
  }

  stats->generatedTransitionsToTight = 0;

  if(opt.sdLazyOnly)
  {
    *complRes = renComplLazy;
    stats->generatedStates = compLazy.getStates().size();
    stats->generatedTrans = compLazy.getTransCount();
  }
  else if(opt.sdMaxrankOnly)
  {
    *complRes = renComplOrig;
    stats->generatedStates = compOrig.getStates().size();
    stats->generatedTrans = compOrig.getTransCount();
  }
  else
  {
    if(opt.ncsbLazy)
    {
      if(renComplOrig.getStates().size() <= renComplLazy.getStates().size() + 10)
      {
        *complRes = renComplOrig;
        stats->generatedStates = compOrig.getStates().size();
        stats->generatedTrans = compOrig.getTransCount();
      }
      else
      {
        *complRes = renComplLazy;
        stats->generatedStates = compLazy.getStates().size();
        stats->generatedTrans = compLazy.getTransCount();
      }
    }
    else
    {
      *complRes = renComplOrig;
      stats->generatedStates = compOrig.getStates().size();
      stats->generatedTrans = compOrig.getTransCount();
    }
  }


  // BuchiAutomaton<int, int> renCompl = comp.renameAutDict(id);
  // //renCompl.removeUseless();
  // renCompl = renCompl.removeUselessRename();

  stats->reachStates = complRes->getStates().size();
  stats->reachTrans = complRes->getTransCount();
  stats->engine = "Ranker";
  stats->transitionsToTight = -1;
  stats->originalStates = sp.getStates().size();
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
  //cerr << "Elevator-automaton: " << (st.elevator ? "Yes" : "No") << endl;
  cerr << "States-before-complementation: " << st.originalStates << endl;
  //cerr << "Elevator-states: " << st.elevatorStates << endl;
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
