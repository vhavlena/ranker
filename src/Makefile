CPPFLAGS=-std=c++17 -O2 -Wall -Wextra #-Wl,-no_pie
OBJ=obj
GCC=g++
SUFF=-lboost_regex

complement: ranker ranker-composition

test: test-parser test-kv-compl test-sch-compl test-process test-nfa-prop \
	test-sch-red-compl test-sch-hard test-simulation

test-parser: units/test-parser.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

test-hoa-word: units/test-hoa-word.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

test-kv-compl: units/test-kv-compl.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

test-hoa-parser: units/test-hoa-parser.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

test-sch-compl: units/test-sch-compl.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

compl-check: units/compl-check.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

test-sch-hard: units/test-sch-hard.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

test-sch-red-compl: units/test-sch-red-compl.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

test-sch-opt-compl: units/test-sch-opt-compl.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

test-nfa-prop: units/test-nfa-prop.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

test-process: units/test-process.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o \
	$(OBJ)/BuchiAutomatonDebug.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

test-simulation: units/test-simulation.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

test-classify: units/test-classify.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomatonSpec.o $(OBJ)/RankFunc.o \
	$(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -o units/$@ $^ $(SUFF)

ranker: ranker.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/AutomatonStruct.o $(OBJ)/BuchiAutomatonSpec.o \
	$(OBJ)/RankFunc.o $(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o \
	$(OBJ)/ranker-general.o $(OBJ)/BuchiAutomatonDebug.o $(OBJ)/BuchiDelay.o \
	$(OBJ)/GenCoBuchiAutomaton.o $(OBJ)/GenCoBuchiAutomatonCompl.o \
	$(OBJ)/ElevatorAutomaton.o $(OBJ)/CoBuchiCompl.o gitversion.cpp
	$(GCC) $(CPPFLAGS) -o $@ $^ $(SUFF)

ranker-sim: ranker-sim.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o  $(OBJ)/Simulations.o $(OBJ)/BuchiAutomatonSpec.o \
	$(OBJ)/RankFunc.o $(OBJ)/AutGraph.o $(OBJ)/AuxFunctions.o \
	$(OBJ)/BuchiAutomatonDebug.o \
	$(OBJ)/GenCoBuchiAutomaton.o $(OBJ)/GenCoBuchiAutomatonCompl.o
	$(GCC) $(CPPFLAGS) -o $@ $^ $(SUFF)

ranker-composition: ranker-composition.cpp $(OBJ)/ranker-general.o $(OBJ)/AuxFunctions.o \
	$(OBJ)/RankFunc.o $(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/Simulations.o   $(OBJ)/AutGraph.o  $(OBJ)/BuchiAutomatonSpec.o \
	$(OBJ)/BuchiAutomatonDebug.o $(OBJ)/BuchiDelay.o $(OBJ)/AutomatonStruct.o \
	$(OBJ)/GenCoBuchiAutomaton.o $(OBJ)/GenCoBuchiAutomatonCompl.o \
	$(OBJ)/ElevatorAutomaton.o
	$(GCC) $(CPPFLAGS) -o ranker-composition $^ $(SUFF)

ranker-tight: ranker-tight.cpp $(OBJ)/AuxFunctions.o $(OBJ)/ranker-general.o \
	$(OBJ)/RankFunc.o $(OBJ)/BuchiAutomaton.o $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/Simulations.o   $(OBJ)/AutGraph.o  $(OBJ)/BuchiAutomatonSpec.o \
	$(OBJ)/BuchiAutomatonDebug.o $(OBJ)/BuchiDelay.o $(OBJ)/AutomatonStruct.o \
	$(OBJ)/GenCoBuchiAutomaton.o $(OBJ)/GenCoBuchiAutomatonCompl.o
	$(GCC) $(CPPFLAGS) -o $@ $^ $(SUFF)

$(OBJ)/ranker-general.o: Ranker-general.cpp $(OBJ)/BuchiAutomataParser.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/AutomatonStruct.o $(OBJ)/BuchiAutomatonSpec.o \
	$(OBJ)/RankFunc.o $(OBJ)/AutGraph.o $(OBJ)/Simulations.o $(OBJ)/AuxFunctions.o \
	$(OBJ)/BuchiAutomatonDebug.o $(OBJ)/GenCoBuchiAutomaton.o $(OBJ)/GenCoBuchiAutomatonCompl.o $(OBJ)/CoBuchiCompl.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/BuchiAutomatonDebug.o: Debug/BuchiAutomatonDebug.cpp \
	Automata/BuchiAutomaton.h Complement/StateSch.h Debug/BuchiAutomatonDebug.h \
	Automata/BuchiAutomatonException.h $(OBJ)/BuchiAutomaton.o $(OBJ)/RankFunc.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/BuchiAutomataParser.o: Automata/BuchiAutomataParser.cpp \
	Automata/BuchiAutomataParser.h $(OBJ)/BuchiAutomaton.o $(OBJ)/AutomatonStruct.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/BuchiAutomaton.o: Automata/BuchiAutomaton.cpp Automata/BuchiAutomaton.h \
	Automata/AutomatonStruct.h Complement/StateSch.h Complement/StateKV.h \
	Complement/RankFunc.h Automata/APSymbol.h Automata/StateSemiDet.h $(OBJ)/AutGraph.o \
	$(OBJ)/AuxFunctions.o $(OBJ)/AutomatonStruct.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/GenCoBuchiAutomaton.o: Automata/GenCoBuchiAutomaton.cpp Automata/GenCoBuchiAutomaton.h \
	Automata/AutomatonStruct.h Complement/StateGcoBA.h \
	Automata/APSymbol.h $(OBJ)/AutGraph.o \
	$(OBJ)/AuxFunctions.o $(OBJ)/AutomatonStruct.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/AutomatonStruct.o: Automata/AutomatonStruct.cpp Automata/AutomatonStruct.h \
	Complement/StateSch.h Complement/StateKV.h Complement/RankFunc.h \
	Automata/APSymbol.h $(OBJ)/AutGraph.o $(OBJ)/AuxFunctions.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/RankFunc.o: Complement/RankFunc.cpp Complement/RankFunc.h
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/Simulations.o: Algorithms/Simulations.cpp Algorithms/Simulations.h \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/AutomatonStruct.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/AutGraph.o: Automata/AutGraph.cpp Automata/AutGraph.h
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/ElevatorAutomaton.o: Complement/ElevatorAutomaton.cpp Automata/AutomatonStruct.h Automata/BuchiAutomaton.h \
	Complement/ElevatorAutomaton.h $(OBJ)/BuchiAutomaton.o $(OBJ)/AutomatonStruct.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/AuxFunctions.o: Algorithms/AuxFunctions.cpp Algorithms/AuxFunctions.h
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/BuchiAutomatonSpec.o: Complement/BuchiAutomatonSpec.cpp \
	Automata/BuchiAutomaton.h Automata/AutomatonStruct.h Complement/StateKV.h \
	Complement/StateSch.h Complement/BuchiAutomatonSpec.h Complement/Options.h \
	Automata/StateSemiDet.h $(OBJ)/RankFunc.o $(OBJ)/AuxFunctions.o $(OBJ)/BuchiDelay.o \
	$(OBJ)/BuchiAutomaton.o $(OBJ)/AutomatonStruct.o $(OBJ)/ElevatorAutomaton.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/GenCoBuchiAutomatonCompl.o: Complement/GenCoBuchiAutomatonCompl.cpp Complement/GenCoBuchiAutomatonCompl.h \
	Automata/BuchiAutomaton.h Automata/AutomatonStruct.h Complement/StateGcoBA.h \
	$(OBJ)/AuxFunctions.o $(OBJ)/BuchiAutomaton.o $(OBJ)/AutomatonStruct.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

gitversion.cpp: ../.git/HEAD ../.git/index
	echo "const char *gitversion = \"$(shell git rev-parse HEAD)\";" > $@

$(OBJ)/CoBuchiCompl.o: Complement/CoBuchiCompl.cpp Complement/CoBuchiCompl.h \
	Automata/BuchiAutomaton.h Automata/AutomatonStruct.h Complement/StateGcoBA.h \
	Automata/GenCoBuchiAutomaton.h $(OBJ)/AuxFunctions.o $(OBJ)/BuchiAutomaton.o \
	$(OBJ)/AutomatonStruct.o $(OBJ)/GenCoBuchiAutomaton.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

$(OBJ)/BuchiDelay.o: Complement/BuchiDelay.cpp \
	Automata/BuchiAutomaton.h Automata/AutomatonStruct.h Complement/StateSch.h \
	Complement/BuchiDelay.h $(OBJ)/RankFunc.o \
	$(OBJ)/AuxFunctions.o $(OBJ)/BuchiAutomaton.o $(OBJ)/AutomatonStruct.o
	$(GCC) $(CPPFLAGS) -c -o $@ $<

clean:
	rm -f obj/*.o units/test-parser units/test-kv-compl units/test-sch-red-compl \
	units/test-sch-compl units/test-nfa-prop units/test-sch-hard \
	units/test-simulation units/test-process units/test-simulation ranker \
	units/test-hoa-parser units/test-classify ranker-composition ranker-sim \
	units/test-hoa-word ranker-tight gitversion.cpp
