
#ifndef _COMPL_OPT_H_
#define _COMPL_OPT_H_

#include "BuchiDelay.h"

enum DataFlowOptions
{
  LIGHT,
  INNER
};

struct ComplOptions
{
  bool cutPoint = false;
  bool succEmptyCheck = true;

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
};

#endif
