
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
  int statesnum = 0;
  vector<string> aps;

  string line;
  string linecp;
  this->line = 0;
  while(getline (os, line))
  {
    this->line++;
    if(line.rfind("States:", 0) == 0)
    {
      linecp = string(line.begin() + 7, line.end());
      boost::algorithm::trim_left(linecp);
      statesnum = std::stoi(linecp);
      for(int i = 0; i < statesnum; i++)
        states.insert(i);
    }
    else if(line.rfind("Start:", 0) == 0)
    {
      linecp = string(line.begin() + 6, line.end());
      boost::algorithm::trim_left(linecp);
      ini.insert(std::stoi(linecp));
    }
    else if(line.rfind("AP:", 0) == 0)
    {
      linecp = string(line.begin() + 3, line.end());
      boost::algorithm::trim_left(linecp);
      std::stringstream ss(linecp);
      int count;
      ss >> count;
      string singleap;
      for(int i = 0; i < count; i++)
      {
        ss >> std::quoted(singleap);
        aps.push_back(singleap);
      }
    }
    else if(line.rfind("Acceptance:", 0) == 0)
    {
      linecp = string(line.begin() + 11, line.end());
      linecp.erase(remove_if(linecp.begin(), linecp.end(), ::isspace), linecp.end());
      if(linecp.rfind("1Inf(0)", 0) != 0)
      {
        throw ParserException("Unsupported acceptance condition. Expected: \"1 Inf(0)\"", this->line);
      }
    }
    else if(line.rfind("acc-name:", 0) == 0)
    {
      linecp = string(line.begin() + 9, line.end());
      boost::algorithm::trim_left(linecp);
      if(linecp.rfind("Buchi", 0) != 0)
      {
        throw ParserException("Unsupported automaton type. Expected: \"Buchi\"", this->line);
      }
    }
    else if(line.rfind("--BODY--", 0) == 0)
    {
      trans = parseHoaBody(aps.size(), os, fins);
    }
  }

  return BuchiAutomaton<int, APSymbol>(states, fins, ini, trans, syms, aps);
}


Transition<int, APSymbol> BuchiAutomataParser::parseHoaTransition(int srcstate, int apNum, string& line)
{
  boost::regex e("\\s*\\[([!&\\|\\s0-9]+)\\]\\s*([0-9]+)");
  boost::smatch what;
  if(boost::regex_match(line, what, e))
  {
    string tr = what[1];
    int dest = std::stoi(what[2]);
    return {srcstate, dest, parseHoaExpression(tr, apNum)};
  }
  else
  {
    throw ParserException("Unsupported format of transitions.", this->line);
  }
}


Delta<int, APSymbol> BuchiAutomataParser::parseHoaBody(int apNum, ifstream & os, set<int>& fin)
{
  Delta<int, APSymbol> trans;
  int src;
  string line;
  while(getline (os, line))
  {
    this->line++;
    if(line.rfind("--END--", 0) == 0)
    {
      return trans;
    }
    else if(line.rfind("State:", 0) == 0)
    {
      string linecp(line.begin()+7, line.end());
      boost::regex e("([0-9]+)\\s*(\\{\\s*0\\s*\\})?");
      boost::smatch what;
      if(boost::regex_match(linecp, what, e))
      {
        src = std::stoi(what[1]);
        if(what.size() == 3)
        {
          string v = what[2];
          if(v.size() > 0)
          {
            fin.insert(src);
          }
        }
      }
      else
      {
        throw ParserException("Unsupported state format. Expected \"State: INT {INT}?\"", this->line);
      }
    }
    else
    {
      Transition<int, APSymbol> tr = parseHoaTransition(src, apNum, line);
      auto pr = std::make_pair(src, tr.symbol);
      if(trans.find(pr) == trans.end())
      {
        trans[pr] = {tr.to};
      }
      else
      {
        trans[pr].insert(tr.to);
      }
    }
  }
  return trans;
}


APSymbol BuchiAutomataParser::parseHoaExpression(string& line, int apNum)
{
  //remove whitespaces
  line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
  if(line.find("|") != string::npos)
  {
    throw ParserException("Only transitions containing & are allowed", this->line);
  }

  APSymbol symbol(apNum);
  vector<char> symvar(apNum, 0);
  int stateid;
  vector<string> tokens;
  boost::split(tokens, line, boost::is_any_of("&"));
  for(const string& t : tokens)
  {
    if(t.size() == 0)
      continue;
    if(t[0] == '!')
    {
      string statestr(t.begin()+1, t.end());
      stateid = stoi(statestr);
      symvar[stateid] = 2;
    }
    else
    {
      stateid = stoi(t);
      symvar[stateid] = 1;
    }
  }

  for(unsigned int i = 0; i < symvar.size(); i++)
  {
    if(symvar[i] == 0)
      throw ParserException("Only simple transitions are allowed", this->line);
    if(symvar[i] == 1)
      symbol.ap.set(i);
  }
  return symbol;
}
