
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
 * Parse a Buchi automaton stored in HOA format
 */
BuchiAutomaton<int, APSymbol> BuchiAutomataParser::parseHoaBA()
{
  set<int> states;
  set<int> ini;
  set<int> finsBA;
  AutomatonStruct<int, APSymbol>::Transitions trans;
  set<APSymbol> syms;
  int statesnum = 0;
  map<string, int> aps;

  string line;
  string linecp;
  this->line = 0;
  while(getline (this->os, line))
  {
    this->line++;

    // states
    if(line.rfind("States:", 0) == 0)
    {
      linecp = string(line.begin() + 7, line.end());
      boost::algorithm::trim_left(linecp);
      statesnum = std::stoi(linecp);
      for(int i = 0; i < statesnum; i++)
        states.insert(i);
    }

    // start
    else if(line.rfind("Start:", 0) == 0)
    {
      linecp = string(line.begin() + 6, line.end());
      boost::algorithm::trim_left(linecp);
      ini.insert(std::stoi(linecp));
    }

    // AP
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

    // acceptance
    else if(line.rfind("Acceptance:", 0) == 0)
    {
      linecp = string(line.begin() + 11, line.end());
      linecp.erase(remove_if(linecp.begin(), linecp.end(), ::isspace), linecp.end());
      if(linecp.rfind("1Inf(0)", 0) != 0)
      {
        throw ParserException("Unsupported acceptance condition. Expected: \"1 Inf(0)\"", this->line);
      }
    }

    else if(line.rfind("--BODY--", 0) == 0)
    {
      trans = parseHoaBodyBA(aps.size(), this->os, finsBA);
    }
  }

  return BuchiAutomaton<int, APSymbol>(states, finsBA, ini, trans, syms, aps);
}

/*
 * Parse a generalized co-Buchi automaton stored in HOA format
 */
GeneralizedCoBuchiAutomaton<int, APSymbol> BuchiAutomataParser::parseHoaGCOBA()
{
  set<int> states;
  set<int> ini;
  set<int> finsBA;
  map<int, set<int>> generalizedFins;
  AutomatonStruct<int, APSymbol>::Transitions trans;
  set<APSymbol> syms;
  int statesnum = 0;
  map<string, int> aps;
  int accSetsCount = 0;

  string line;
  string linecp;
  this->line = 0;
  while(getline (this->os, line))
  {
    this->line++;

    // states
    if(line.rfind("States:", 0) == 0)
    {
      linecp = string(line.begin() + 7, line.end());
      boost::algorithm::trim_left(linecp);
      statesnum = std::stoi(linecp);
      for(int i = 0; i < statesnum; i++)
        states.insert(i);
    }

    // start
    else if(line.rfind("Start:", 0) == 0)
    {
      linecp = string(line.begin() + 6, line.end());
      boost::algorithm::trim_left(linecp);
      ini.insert(std::stoi(linecp));
    }

    // AP
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

    // acceptance
    else if(line.rfind("Acceptance:", 0) == 0)
    {
      linecp = string(line.begin() + 11, line.end());
      linecp.erase(remove_if(linecp.begin(), linecp.end(), ::isspace), linecp.end());
      std::regex gcobaAcceptance("([0-9]+)(\\s)*(\\()*Fin\\([0-9]+\\)((\\s)*\\|(\\s)*Fin\\([0-9]+\\))*(\\))*");
      if (!std::regex_match(linecp, gcobaAcceptance))
        throw ParserException("Unsupported acceptance condition.");
      else
      {
        std::smatch match;
        if (std::regex_search(linecp, match, gcobaAcceptance))
        {
          accSetsCount = std::stoi(match.str(1));
          for (int i=0; i<accSetsCount; i++)
          {
            std::set<int> tmpSet;
            generalizedFins.insert({i, tmpSet});
          }
        }
      }
    }

    else if(line.rfind("--BODY--", 0) == 0)
    {
      trans = parseHoaBodyGCOBA(aps.size(), this->os, generalizedFins);
    }
  }

  return GeneralizedCoBuchiAutomaton<int, APSymbol>(states, generalizedFins, ini, trans, syms, aps);
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
 * Parse body in HOA format with the expectation of Buchi automata
 * @param apNum Number of atomic propositions
 * @param os Reading stream
 * @param finsBA Accepting states of the BA
 * @return Transition function
 */
Delta<int, APSymbol> BuchiAutomataParser::parseHoaBodyBA(int apNum, ifstream & os, set<int>& finsBA)
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
      boost::regex baRegex("([0-9]+)\\s*(\\{\\s*0\\s*\\})?");
      boost::smatch what;

      // BA
      if(boost::regex_match(linecp, what, baRegex))
      {
        src = std::stoi(what[1]);
        if(what.size() == 3)
        {
          string v = what[2];
          if(v.size() > 0)
          {
            finsBA.insert(src);
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
 * Parse body in HOA format with the expectation of generalized co-Buchi automata
 * @param apNum Number of atomic propositions
 * @param os Reading stream
 * @param generalizedFins Accepting states of the Gco-BA
 * @return Transition function
 */
Delta<int, APSymbol> BuchiAutomataParser::parseHoaBodyGCOBA(int apNum, ifstream & os, map<int, set<int>>& generalizedFins)
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
      boost::regex gcobaRegex("([0-9]+)\\s*(\\{\\s*[0-9]+((\\s)+[0-9]+)*\\s*\\})?");
      boost::smatch what;


      // GcoBA
      if (boost::regex_match(linecp, what, gcobaRegex))
      {
        src = std::stoi(what[1]);
        if (what.size() >= 3 and what[2].length()>0){
          string v = what[2];
          v.erase(v.begin(), v.begin()+1);
          v.erase(v.end()-1, v.end());
          // get each acceptance condition
          std::string delimiter = " ";
          size_t pos = 0;
          std::string token = v;
          while ((pos = v.find(delimiter)) != std::string::npos){
            token = v.substr(0, pos);
            // insert state in finals
            auto it = generalizedFins.find(std::stoi(token));
            if (it == generalizedFins.end())
              throw ParserException("Wrong number of accepting sets.");
            else
              it->second.insert(src);
            v.erase(0, pos + delimiter.length());
          }
          // last element outside of the loop
          auto it = generalizedFins.find(std::stoi(v));
          if (it == generalizedFins.end())
            throw ParserException("Wrong number of accepting sets.");
          else
            it->second.insert(src);
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
 * Parse automaton type
 * @return Type of an omega-automaton
 */
AutomatonType BuchiAutomataParser::parseAutomatonType()
{
  string line;
  string linecp;
  while(getline (os, line))
  {
    if(line.rfind("Acceptance:", 0) == 0)
    {
      linecp = string(line.begin() + 11, line.end());
      linecp.erase(remove_if(linecp.begin(), linecp.end(), ::isspace), linecp.end());
      if(linecp.rfind("1Inf(0)", 0) == 0)
      {
        this->os.clear();
        this->os.seekg(0);
        return AUTBA;
      }
      else
      {
        std::regex gcobaAcceptance("([0-9]+)(\\s)*(\\()*Fin\\([0-9]+\\)((\\s)*\\|(\\s)*Fin\\([0-9]+\\))*(\\))*");
        if (std::regex_match(linecp, gcobaAcceptance))
        {
          this->os.clear();
          this->os.seekg(0);
          return AUTGCOBA;
        }
      }
    }

    // acc-name
    else if(line.rfind("acc-name:", 0) == 0)
    {
      linecp = string(line.begin() + 9, line.end());
      boost::algorithm::trim_left(linecp);

      std::regex baRegex("Buchi(\\s)*");
      std::regex gcobaRegex("generalized-co-Buchi(\\s)*[0-9]+(\\s)*");
      if (std::regex_match(linecp, baRegex))
      {
        this->os.clear();
        this->os.seekg(0);
        return AUTBA;
      }
      else if (std::regex_match(linecp, gcobaRegex))
      {
        this->os.clear();
        this->os.seekg(0);
        return AUTGCOBA;
      }
      else
      {
        this->os.clear();
        this->os.seekg(0);
        return AUTNONE;
      }
    }
  }
  this->os.clear();
  this->os.seekg(0);
  return AUTNONE;
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
