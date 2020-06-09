#ifndef _BUCHI_PARSER_H_
#define _BUCHI_PARSER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <boost/algorithm/string.hpp>
#include "BuchiAutomaton.h"

using namespace std;

class BuchiAutomataParser {

public:
  BuchiAutomaton<std::string, std::string> parseBaFormat(ifstream & os);

private:
  Transition<string, string> parseBATransition(string line);

};

#endif
