# Buchi Automata Complementation and Inclusion Checking

The repository contains the tool _Ranker_, rank-based-inspired complementation
of Buchi automata. Prerequisites:
```
- gcc >= 8.3.0
- boost library
```

Install using `make complement` in `src` folder.

### Input Format

The tool accepts automata in *HOA* format (Hanoi Omega Automata) as well as automata in *BA* format.
Moreover the tool assumes HOA with the following restrictions
- only state-based acceptance is supported
- transitions need to have the form of a single conjunction with exactly one positive atomic proposition
- no aliases or any other fancy features of HOA are supported
