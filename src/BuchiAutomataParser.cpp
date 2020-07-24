
#include "BuchiAutomataParser.h"

BuchiAutomaton<string, string> BuchiAutomataParser::parseBaFormat(ifstream & os)
{
  set<string> states;
  set<string> ini;
  set<string> fins;
  BuchiAutomaton<string, string>::Transitions trans;
  set<string> syms;

  string line;
  int state = 0;
  while(getline (os, line))
  {
    boost::trim_right(line);
    if (line.length() == 0)
      continue;
    if(line.find(',') != string::npos)
    {
      Transition<string, string> tr = parseBATransition(line);
      state = 1;
      states.insert(tr.from);
      states.insert(tr.to);
      pair<string, string> pr = make_pair(tr.from, tr.symbol);
      syms.insert(tr.symbol);
      if(trans.find(pr) == trans.end())
        trans.insert({pr, set<string>({tr.to})});
      else
        trans[pr].insert(tr.to);
    }
    else
    {
      if(state == 0)
        ini.insert(line);
      else
        fins.insert(line);
      states.insert(line);
    }
  }
  if(syms.size() == 0)
  {
    syms.insert({"a0", "a1"});
  }
  return BuchiAutomaton<string, string>(states, fins, ini, trans, syms);
}

Transition<string, string> BuchiAutomataParser::parseBATransition(string line)
{
  Transition<string, string> tr;
  int state = 0;
  for (char ch : line)
  {
    if(state == 0 && ch == ',')
    {
      state = 1;
      continue;
    }
    if(state == 1 && ch == '>')
    {
      state = 2;
      continue;
    }
    if(ch == '-')
      continue;

    switch(state)
    {
      case 0:
        tr.symbol.push_back(ch); break;
      case 1:
        tr.from.push_back(ch); break;
      case 2:
        tr.to.push_back(ch); break;
    }

  }
  return tr;
}
