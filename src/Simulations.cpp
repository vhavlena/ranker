
#include "Simulations.h"


void Simulations::splitString(string& line, vector<string>& split)
{
  istringstream stream(line);
  string item;
  while(!stream.eof())
  {
    stream >> item;
    split.push_back(item);
  }
}


RabitSimLine Simulations::parseRabitRelLine(string& line)
{
  vector<string> sp;
  Simulations::splitString(line, sp);
  if(sp.size() != 4)
  {
    throw string("Parser: uncompatible relation format: ") + line;
  }

  return {{sp[0], sp[1]}, {sp[2], sp[3]}};
}


BuchiAutomaton<std::string, std::string>::StateRelation Simulations::parseRabitRelation(istringstream & os)
{
  string line;
  BuchiAutomaton<std::string, std::string>::StateRelation rel;
  RabitSimLine rabLine;
  while(getline (os, line))
  {
    boost::trim_right(line);
    if (line.length() == 0)
      continue;

    rabLine = Simulations::parseRabitRelLine(line);
    rel.insert(rabLine.rel);
  }
  return rel;
}
