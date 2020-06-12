
CPPFLAGS=-std=c++17

test: test-parser test-kv-compl

test-parser: units/test-parser.cpp BuchiAutomataParser.o BuchiAutomaton.o BuchiAutomatonSpec.o
	g++ $(CPPFLAGS) -o units/$@ $?

test-kv-compl: units/test-kv-compl.cpp BuchiAutomataParser.o BuchiAutomaton.o BuchiAutomatonSpec.o
	g++ $(CPPFLAGS) -o units/$@ $?

BuchiAutomataParser.o: BuchiAutomataParser.cpp BuchiAutomataParser.h
	g++ $(CPPFLAGS) -c -o $@ $<

BuchiAutomaton.o: BuchiAutomaton.cpp BuchiAutomaton.h
	g++ $(CPPFLAGS) -c BuchiAutomaton.cpp  -o $@

BuchiAutomatonSpec.o: BuchiAutomaton.o BuchiAutomatonSpec.cpp BuchiAutomaton.h ComplementBA.h BuchiAutomatonSpec.h
	g++ $(CPPFLAGS) -c BuchiAutomatonSpec.cpp -o $@

clean:
	rm *.o units/test-parser units/test-kv-compl
