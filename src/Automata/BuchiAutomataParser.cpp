
#include "BuchiAutomataParser.h"

/*
 * Parse automaton in BA format
 * @param os Input stream
 * @return BA <string, string>
 */
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


/*
 * Parse transition in BA format
 * @param line File line
 * @return Parsed transition
 */
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


/*
 * Parse automaton in GFF format
 * @param str String with the automaton in GFF format
 * @return Parsed BA <string, string>
 */
BuchiAutomaton<string, string> BuchiAutomataParser::parseGffFormat(string& str)
{
  boost::algorithm::to_lower(str);
  istringstream iss (str);

  pt::ptree tree;
  pt::read_xml(iss, tree);
  return parseGffTree(tree);
}


/*
 * Parse automaton in GFF format
 * @param os Input stream
 * @return BA <string, string>
 */
BuchiAutomaton<string, string> BuchiAutomataParser::parseGffFormat(ifstream& is)
{
  std::string str((std::istreambuf_iterator<char>(is)),
                 std::istreambuf_iterator<char>());
  return parseGffFormat(str);
}


/*
 * Parse automaton from GFF XML tree
 * @param tree Boost XML tree
 * @return BA <string, string>
 */
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


/*
 * Parse GFF transition
 * @param tree Boost XML tree corresponding to a single transition
 * @return Transition
 */
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


/*
 * Parse automaton in HOA format
 * @param os Input stream
 * @return BA <int, APSymbol>
 */
BuchiAutomaton<int, APSymbol> BuchiAutomataParser::parseHoaFormat(ifstream & os)
{
  set<int> states;
  set<int> ini;
  set<int> fins;
  BuchiAutomaton<int, APSymbol>::Transitions trans;
  set<APSymbol> syms;
  int statesnum = 0;
  map<string, int> aps;

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
        aps.insert({singleap, i});
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

/*
 * Parse single transition in HOA format
 * @param srcstate Source state
 * @param apNum Number of APs
 * @param line String with the transition
 * @return Transition
 */
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


/*
 * Parse HOA body (transition function with accepting states)
 * @param apNum Number of APs
 * @param os Input stream
 * @param fin Final states (out parameter)
 * @return Transition function
 */
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


/*
 * Parse HOA expression
 * @param line String with an expression
 * @param apNum Number of APs
 * @return APSymbol corresponding to the model of given expression
 */
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


/*
 * Parse a HOA symbol
 * @param line String with a symbol
 * @param apInd Mapping AP to indices
 * @return APSymbol corresponding to the symbol
 */
APSymbol BuchiAutomataParser::parseHoaSymbol(string& line, map<string, int>& apInd)
{
  int apNum = 0;
  vector<char> symvar(apInd.size(), 0);
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
      stateid = apInd[statestr];
      symvar[stateid] = 2;
    }
    else
    {
      stateid = apInd[t];
      symvar[stateid] = 1;
    }
    apNum++;
  }

  APSymbol symbol(apNum);
  for(unsigned int i = 0; i < symvar.size(); i++)
  {
    if(symvar[i] == 0)
      throw ParserException("Only simple symbols are allowed");
    if(symvar[i] == 1)
      symbol.ap.set(i);
  }
  return symbol;
}


/*
 * Parse a finite HOA word
 * @param line String with a word
 * @param apInd Mapping AP to indices
 * @return AP finite word
 */
APWord BuchiAutomataParser::parseHoaFinWord(string& line, map<string, int>& apInd)
{
  vector<string> symbols;
  APWord word;
  boost::split(symbols, line, boost::is_any_of(";"));
  for(string& t : symbols)
  {
    if(t.size() == 0)
      continue;

    word.push_back(BuchiAutomataParser::parseHoaSymbol(t, apInd));
  }
  return word;
}


/*
 * Parse an infinite HOA word
 * @param line String with a word
 * @param apInd Mapping AP to indices
 * @return AP finite word
 */
pair<APWord, APWord> BuchiAutomataParser::parseHoaInfWord(string& line, map<string, int>& apInd)
{
  //remove whitespaces
  line.erase(remove_if(line.begin(), line.end(), ::isspace), line.end());
  boost::regex e("([a-zA-Z0-9!;&]*)cycle\\{([a-zA-Z0-9!;&]*)\\}$");
  boost::smatch what;
  if(boost::regex_match(line, what, e))
  {
    string prefix = what[1];
    string loop = what[2];
    return { BuchiAutomataParser::parseHoaFinWord(prefix, apInd), BuchiAutomataParser::parseHoaFinWord(loop, apInd) };
  }
  else
  {
    cout << line << endl;
    throw ParserException("Unsupported format of infinite words");
  }
}
