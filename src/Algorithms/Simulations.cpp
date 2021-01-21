
#include "Simulations.h"


/*
 * Split string according to whitespaces
 * @param line Input for splitting
 * @param split Vector of strings (out)
 */
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


/*
 * Parse relation line from Rabig
 * @param line Input for parsing
 * @return Pair of states with corresponding automata
 */
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


/*
 * Parse Rabit relation
 * @param os Input stream
 * @return Relation on states
 */
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


/*
 * Execute a command
 * @param cmd Command
 * @param timeout Timeout
 * @return Output
 */
string Simulations::execCmdTO(string& cmd, int timeout)
{
  array<char, 128> buffer;
  string result = "";
  string toutCmd = "gtimeout " + std::to_string(timeout) + "s " + cmd;
  //string toutCmd = cmd;
  auto pipe = popen(toutCmd.c_str(), "r");
  if (!pipe)
  {
    throw "Process failed";
  }
  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
  {
    result += buffer.data();
  }
  int r = pclose(pipe);
  if(r != 0) //Timeout expired
  {
    throw TimeoutException();
  }
  return result;
}


/*
 * Execute a command
 * @param cmd Command
 * @return Output
 */
string Simulations::execCmd(string& cmd)
{
  array<char, 128> buffer;
  string result = "";
  auto pipe = popen(cmd.c_str(), "r");
  if (!pipe)
  {
    throw "Process failed";
  }
  while (fgets(buffer.data(), buffer.size(), pipe) != nullptr)
  {
    result += buffer.data();
  }
  pclose(pipe);
  return result;
}
