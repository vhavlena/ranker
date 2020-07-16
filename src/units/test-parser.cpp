
#include <iostream>
#include <set>
#include <map>
#include <fstream>
#include <cstdio>
#include <memory>
#include <stdexcept>
#include <string>
#include <array>
#include "../BuchiAutomaton.h"
#include "../BuchiAutomataParser.h"
#include "../Simulations.h"

using namespace std;

string RABITEXE = "";

std::string exec(string& cmd) {
    std::array<char, 128> buffer;
    std::string result;
    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd.c_str(), "r"), pclose);
    if (!pipe)
    {
        throw "Process failed";
    }
    while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
    {
        result += buffer.data();
    }
    return result;
}

int main()
{
  BuchiAutomataParser parser;
  ifstream os;
  os.open("../../examples/A3.ba");
  cout << "../../examples/A3.ba" << endl;

  if(os)
  {
    auto ba = parser.parseBaFormat(os);
    cout << ba.toString() << endl;
  }
  os.close();

  os.open("../../examples/A4.ba");
  cout << endl << "../../examples/A4.ba" << endl;
  if(os)
  {
    auto ba = parser.parseBaFormat(os);
    cout << ba.toString() << endl;
  }
  os.close();

  os.open("../../examples/A6.ba");
  cout << endl << "../../examples/A6.ba" << endl;
  if(os)
  {
    auto ba = parser.parseBaFormat(os);
    cout << ba.toString() << endl;
  }
  os.close();

  cout << endl << "Simulation parsing: " << endl;
  string cmd = "java -jar " + RABITEXE + " ../../examples/A3.ba ../../examples/A3.ba -dirsim";
  Simulations sim;
  istringstream strr(exec(cmd));
  try
  {
    for(const auto& item : sim.parseRabitRelation(strr))
    {
      cout << item.first << " " << item.second << endl;
    }
  }
  catch(string& msg)
  {
    cout << msg << endl;
  }

  return 0;
}
