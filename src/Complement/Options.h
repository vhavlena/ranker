
#ifndef _COMPL_OPT_H_
#define _COMPL_OPT_H_

struct ComplOptions
{
  bool cutPoint = false;
  bool succEmptyCheck = true;

  unsigned ROMinState = 9;
  int ROMinRank = 7;

  unsigned CacheMaxState = 6;
  int CacheMaxRank = 8;

  bool semidetOpt = false;
};

#endif
