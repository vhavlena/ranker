
CPPFLAGS=-std=c++17

test-parser: units/test-parser.cpp BuchiAutomataParser.o BuchiAutomaton.o
	g++ $(CPPFLAGS) -o units/$@ $?

BuchiAutomataParser.o: BuchiAutomataParser.cpp BuchiAutomataParser.h
	g++ $(CPPFLAGS) -c -o $@ $<

BuchiAutomaton.o: BuchiAutomaton.cpp BuchiAutomaton.h
	g++ $(CPPFLAGS) -c -o $@ $<


clean:
	rm *.o units/test-parser
