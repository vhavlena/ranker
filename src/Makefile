
CPPFLAGS=-std=c++17 -O2

test: test-parser test-kv-compl test-sch-compl test-process

test-parser: units/test-parser.cpp BuchiAutomataParser.o BuchiAutomaton.o BuchiAutomatonSpec.o RankFunc.o AutGraph.o
	g++ $(CPPFLAGS) -o units/$@ $?

test-kv-compl: units/test-kv-compl.cpp BuchiAutomataParser.o BuchiAutomaton.o BuchiAutomatonSpec.o RankFunc.o AutGraph.o
	g++ $(CPPFLAGS) -o units/$@ $?

test-sch-compl: units/test-sch-compl.cpp BuchiAutomataParser.o BuchiAutomaton.o BuchiAutomatonSpec.o RankFunc.o AutGraph.o
	g++ $(CPPFLAGS) -o units/$@ $?

test-process: units/test-process.cpp BuchiAutomataParser.o BuchiAutomaton.o BuchiAutomatonSpec.o RankFunc.o AutGraph.o
	g++ $(CPPFLAGS) -o units/$@ $?

BuchiAutomataParser.o: BuchiAutomataParser.cpp BuchiAutomataParser.h AutGraph.o
	g++ $(CPPFLAGS) -c -o $@ $<

BuchiAutomaton.o: BuchiAutomaton.cpp BuchiAutomaton.h RankFunc.h AutGraph.h AutGraph.o
	g++ $(CPPFLAGS) -c BuchiAutomaton.cpp -o $@

RankFunc.o: RankFunc.cpp RankFunc.h
	g++ $(CPPFLAGS) -c -o $@ $<

AutGraph.o: AutGraph.cpp AutGraph.h
	g++ $(CPPFLAGS) -c -o $@ $<

BuchiAutomatonSpec.o: BuchiAutomaton.o RankFunc.o BuchiAutomatonSpec.cpp BuchiAutomaton.h StateKV.h StateSch.h BuchiAutomatonSpec.h
	g++ $(CPPFLAGS) -c BuchiAutomatonSpec.cpp -o $@

clean:
	rm *.o units/test-parser units/test-kv-compl
