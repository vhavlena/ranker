
CPPFLAGS=-std=c++17 -O2 -Wall -Wextra #-Wl,-no_pie
OBJ=obj
GCC=g++

complement: rnk-red-complement

test: test-parser test-kv-compl test-sch-compl test-process test-nfa-prop \
	test-sch-red-compl test-sch-hard

test-parser: units/test-parser.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $?

test-kv-compl: units/test-kv-compl.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $?

test-sch-compl: units/test-sch-compl.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $?

compl-check: units/compl-check.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $?

test-sch-hard: units/test-sch-hard.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $?

test-sch-red-compl: units/test-sch-red-compl.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $?

test-nfa-prop: units/test-nfa-prop.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $?

test-process: units/test-process.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $?

rnk-red-complement: rnk-red-complement.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o $@ $?

$(OBJ)/BuchiAutomataParser.o: BuchiAutomataParser.cpp BuchiAutomataParser.h \
	$(OBJ)/AutGraph.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/BuchiAutomaton.o: BuchiAutomaton.cpp BuchiAutomaton.h RankFunc.h \
	AuxFunctions.h AutGraph.h $(OBJ)/AutGraph.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -c BuchiAutomaton.cpp -o $@

$(OBJ)/RankFunc.o: RankFunc.cpp RankFunc.h
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/Simulations.o: Simulations.cpp Simulations.h
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/AutGraph.o: AutGraph.cpp AutGraph.h
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/AuxFunctions.o: AuxFunctions.cpp AuxFunctions.h
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/BuchiAutomatonSpec.o: $(OBJ)/BuchiAutomaton.o $(OBJ)/RankFunc.o \
	$(OBJ)/AuxFunctions.o BuchiAutomatonSpec.cpp BuchiAutomaton.h StateKV.h \
	StateSch.h BuchiAutomatonSpec.h Options.h
	$(GCC) $(CPPFLAGS) -c BuchiAutomatonSpec.cpp -o $@

clean:
	rm -f obj/*.o units/test-parser units/test-kv-compl units/test-sch-red-compl \
	units/test-sch-compl units/test-nfa-prop units/test-process
