#ifndef _BUCHI_PARSER_H_
#define _BUCHI_PARSER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <streambuf>
#include <exception>
#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/optional/optional.hpp>

#include "BuchiAutomaton.h"
#include "APSymbol.h"

using namespace std;
namespace pt = boost::property_tree;

class ParserException : public exception
{
private:
  string msg;

public:
  ParserException(string info = "Parser exception") : msg(info) {}

  virtual const char* what() const throw () { return msg.c_str(); }
};

class BuchiAutomataParser {

public:
  BuchiAutomaton<string, string> parseBaFormat(ifstream & os);
  BuchiAutomaton<string, string> parseGffFormat(string& str);
  BuchiAutomaton<string, string> parseGffFormat(ifstream& is);
  BuchiAutomaton<int, APSymbol> parseHoaFormat(ifstream & os);

private:
  Transition<string, string> parseBATransition(string line);
  Transition<string, string> parseGffTransition(pt::ptree& tr);
  BuchiAutomaton<string, string> parseGffTree(pt::ptree& tr);

};

#endif
