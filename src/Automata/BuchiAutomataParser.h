#ifndef _BUCHI_PARSER_H_
#define _BUCHI_PARSER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <streambuf>
#include <exception>
#include <algorithm>

#include <boost/algorithm/string.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>
#include <boost/optional/optional.hpp>
#include <boost/regex.hpp>

#include "BuchiAutomaton.h"
#include "APSymbol.h"

using namespace std;
namespace pt = boost::property_tree;

/*
 * Parser exception
 */
class ParserException : public exception
{
private:
  string msg;
  int line;

public:
  ParserException(string info = "Parser exception", int ln = -1) : msg(info), line(ln) {}

  virtual const char* what() const throw () { return msg.c_str(); }
  int getLine() const { return line; }
};

class BuchiAutomataParser {

private:
  // Current line in a file
  int line;

public:
  BuchiAutomaton<string, string> parseBaFormat(ifstream & os);
  BuchiAutomaton<string, string> parseGffFormat(string& str);
  BuchiAutomaton<string, string> parseGffFormat(ifstream& is);
  BuchiAutomaton<int, APSymbol> parseHoaFormat(ifstream & os);

private:
  Transition<string, string> parseBATransition(string line);
  Transition<string, string> parseGffTransition(pt::ptree& tr);
  BuchiAutomaton<string, string> parseGffTree(pt::ptree& tr);

  Transition<int, APSymbol> parseHoaTransition(int srcstate, int apNum, string& line);
  Delta<int, APSymbol> parseHoaBody(int apNum, ifstream & os, set<int>& fin);
  APSymbol parseHoaExpression(string & line, int apNum);

};

#endif
