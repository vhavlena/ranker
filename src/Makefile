
CPPFLAGS=-std=c++17 -O2
OBJ=obj

test: test-parser test-kv-compl test-sch-compl test-process test-nfa-prop

test-parser: units/test-parser.cpp $(OBJ)/BuchiAutomataParser.o $(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o $(OBJ)/AutGraph.o
	g++ $(CPPFLAGS) -o units/$@ $?

test-kv-compl: units/test-kv-compl.cpp $(OBJ)/BuchiAutomataParser.o $(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o $(OBJ)/AutGraph.o
	g++ $(CPPFLAGS) -o units/$@ $?

test-sch-compl: units/test-sch-compl.cpp $(OBJ)/BuchiAutomataParser.o $(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o $(OBJ)/AutGraph.o
	g++ $(CPPFLAGS) -o units/$@ $?

test-nfa-prop: units/test-nfa-prop.cpp $(OBJ)/BuchiAutomataParser.o $(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o $(OBJ)/AutGraph.o
	g++ $(CPPFLAGS) -o units/$@ $?

test-process: units/test-process.cpp $(OBJ)/BuchiAutomataParser.o $(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o $(OBJ)/AutGraph.o
	g++ $(CPPFLAGS) -o units/$@ $?

BuchiAutomataParser.o: BuchiAutomataParser.cpp BuchiAutomataParser.h AutGraph.o
	g++ $(CPPFLAGS) -c -o $(OBJ)/$@ $<

BuchiAutomaton.o: BuchiAutomaton.cpp BuchiAutomaton.h RankFunc.h AutGraph.h AutGraph.o
	g++ $(CPPFLAGS) -c BuchiAutomaton.cpp -o $(OBJ)/$@

RankFunc.o: RankFunc.cpp RankFunc.h
	g++ $(CPPFLAGS) -c -o $(OBJ)/$@ $<

AutGraph.o: AutGraph.cpp AutGraph.h
	g++ $(CPPFLAGS) -c -o $(OBJ)/$@ $<

BuchiAutomatonSpec.o: BuchiAutomaton.o RankFunc.o BuchiAutomatonSpec.cpp BuchiAutomaton.h StateKV.h StateSch.h BuchiAutomatonSpec.h
	g++ $(CPPFLAGS) -c BuchiAutomatonSpec.cpp -o $(OBJ)/$@

clean:
	rm *.o units/test-parser units/test-kv-compl
