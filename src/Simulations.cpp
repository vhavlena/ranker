
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


string Simulations::execCmd(string& cmd, int timeout)
{
  array<char, 128> buffer;
  string result = "";
  string toutCmd = "gtimeout " + std::to_string(timeout) + "s " + cmd;
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


template<typename State, typename Symbol>
Relation<State> Simulations::computeDirectCompl(BuchiAutomaton<State, Symbol>& ba)
{
  map<tuple<Symbol, State, State>, int> counter;
  for(const Symbol& sym : ba.getAlphabet())
  {
    for(const State& s1 : ba.getStates())
    {
      for(const State& s2 : ba.getStates())
        counter[{sym, s1, s2}] = 0;
    }
  }

  queue<pair<State, State>> proc;
  Relation<State> ret;

  set<int> nofin;
  set<int> fin = ba.getFinals();
  auto trans = ba.getTransitions();

  std::set_difference(ba.getStates().begin(), ba.getStates().end(), fin.begin(),
    fin.end(), std::inserter(nofin, nofin.begin()));
  for(const State& f : fin)
  {
    for(const State& nf : nofin)
    {
      ret.insert({f, nf});
      proc.push({f, nf});
    }
  }

  while(proc.size() > 0)
  {
    auto item = proc.front();
    proc.pop();
    for(const Symbol& a : ba.getAlphabet())
    {
      for(const State& k : trans[item.second, a])
      {
        counter[{a, item.first, k}]++;
        if(counter[{a, item.first, k}] == trans[item.second, a].size())
        {
          for(const State& m : trans[item.first, a])
          {
            if(ret.find({m,k}) == ret.end())
            {
              ret.insert({m,k});
              proc.push({m,k});
            }
          }
        }
      }
    }
  }
  return ret;
}


template<typename State, typename Symbol>
Relation<State> directSimulation(BuchiAutomaton<State, Symbol>& ba)
{
  Relation<State> dir;
  Relation<State> comp = computeDirectCompl(ba);
  for(const State& s1 : ba.getStates())
  {
    for(const State& s2 : ba.getStates())
    {
      if(comp.find({s1, s2}) == comp.end())
        dir.insert({s1, s2});
    }
  }
  return dir;
}
