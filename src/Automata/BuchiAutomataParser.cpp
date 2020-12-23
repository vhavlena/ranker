
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


BuchiAutomaton<string, string> BuchiAutomataParser::parseGffFormat(string& str)
{
  boost::algorithm::to_lower(str);
  istringstream iss (str);

  pt::ptree tree;
  pt::read_xml(iss, tree);
  return parseGffTree(tree);
}


BuchiAutomaton<string, string> BuchiAutomataParser::parseGffFormat(ifstream& is)
{
  std::string str((std::istreambuf_iterator<char>(is)),
                 std::istreambuf_iterator<char>());
  return parseGffFormat(str);
}


BuchiAutomaton<string, string> BuchiAutomataParser::parseGffTree(pt::ptree& tree)
{
  set<string> alph;
  for(pt::ptree::value_type &v : tree.get_child("structure.alphabet"))
  {
    if(v.first == "symbol")
      alph.insert(v.second.data());
  }

  set<string> states;
  for(pt::ptree::value_type &v : tree.get_child("structure.stateset"))
  {
    states.insert(v.second.get_child("<xmlattr>.sid").data());
  }

  set<string> fin;
  for(pt::ptree::value_type &v : tree.get_child("structure.acc"))
  {
    if(v.first == "stateid")
      fin.insert(v.second.data());
  }

  set<string> ini;
  for(pt::ptree::value_type &v : tree.get_child("structure.initialstateset"))
  {
    if(v.first == "stateid")
      ini.insert(v.second.data());
  }

  BuchiAutomaton<string, string>::Transitions trans;
  for(pt::ptree::value_type &v : tree.get_child("structure.transitionset"))
  {
    if(v.first != "transition")
      continue;
    Transition<string, string> tr = parseGffTransition(v.second);
    pair<string, string> pr = make_pair(tr.from, tr.symbol);
    if(trans.find(pr) == trans.end())
      trans.insert({pr, set<string>({tr.to})});
    else
      trans[pr].insert(tr.to);
  }

  return BuchiAutomaton<string, string>(states, fin, ini, trans, alph);
}


Transition<string, string> BuchiAutomataParser::parseGffTransition(pt::ptree& tr)
{
  Transition<string, string> ret;
  ret.from = tr.get_child("from").data();
  ret.to = tr.get_child("to").data();

  boost::optional< pt::ptree& > sym = tr.get_child_optional("label");
  if(!sym)
  {
    ret.symbol = tr.get_child("read").data();
  }
  else
  {
    ret.symbol = sym->data();
  }
  return ret;
}

BuchiAutomaton<int, APSymbol> BuchiAutomataParser::parseHoaFormat(ifstream & os)
{
  set<int> states;
  set<int> ini;
  set<int> fins;
  BuchiAutomaton<int, APSymbol>::Transitions trans;
  set<APSymbol> syms;

  string line;
  while(getline (os, line))
  {
    if(line.rfind("States:", 0) == 0)
    {
      //states
    }
    else if(line.rfind("Start:", 0) == 0)
    {
      //initial states
    }
    else if(line.rfind("AP:", 0) == 0)
    {
      //alphabet
    }
    else if(line.rfind("Acceptance:", 0) == 0)
    {
      //acceptance set
    }
    else if(line.rfind("acc-name:", 0) == 0)
    {
      //type of automaton
    }
  }

  return BuchiAutomaton<int, APSymbol>(states, fins, ini, trans, syms);
}
