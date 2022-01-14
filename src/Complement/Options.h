
#ifndef _COMPL_OPT_H_
#define _COMPL_OPT_H_

#include "BuchiDelay.h"

enum DataFlowOptions
{
  LIGHT,
  INNER
};

enum PreprocessOptions
{
  NONE,
  CPIWA,
  CPDET,
  CPALL,
  CPTRIVIAL,
  CPHEUR
};

struct ComplOptions
{
  bool cutPoint = false;
  bool succEmptyCheck = false;

  unsigned ROMinState = 9;
  int ROMinRank = 7;

  unsigned CacheMaxState = 6;
  int CacheMaxRank = 8;

  bool semidetOpt = false;
  DataFlowOptions dataFlow = INNER;

  bool delay = false;
  DelayVersion delayVersion = oldVersion;
  float delayW = 0.5;

  bool eta4 = false;

  bool debug = false;

  ElevatorOptions elevator = { .elevatorRank = true, .detBeginning = false };

  bool dirsim = true;
  bool ranksim = true;
  bool sl = true;
  bool reach = true;

  bool flowDirSim = false;

  PreprocessOptions preprocess = NONE;
  bool accPropagation = false;

  bool semideterminize = false;

  bool backoff = false;

  vector<pair<unsigned, int>> BOBound = {{11,15}, {11,13}};

  bool semideterministic = false;
  bool complete = false;

  bool lowrankopt = false;

  bool iwSim = false;
  bool iwSat = false;

  bool ncsbLazy = false;

  bool tba = false;

  bool light = false;

  bool prered = true;
  bool postred = false;

  bool iwOrigOnly = false;
  bool iwPruneOnly = false;
  bool sdLazyOnly = false;
  bool sdMaxrankOnly = false;
};

#endif
