# Ranker: A Tool for Buchi Automata Complementation

The repository contains the _Ranker_ tool, rank-based-inspired complementation
of Buchi automata. Currently supported types of omega-automata:
- Buchi automata with mixed transition-based/state-based acceptance condition
- Generalized co-Buchi automata

### Installation

Prerequisites:
```
- gcc >= 8.3.0
- boost library
```

Install using `make ranker` in `src` folder.

### Input Automata Format

The tool accepts automata in *HOA* format (Hanoi Omega Automata) as well as automata in *BA* format.
Moreover the tool assumes HOA with the following restrictions
- no aliases or any other fancy features of HOA are supported

### Running

To run _Ranker_ use the following command:
```
./ranker [INPUT] {OPTIONS}
```
where `OPTIONS` are from the following:
```
    -h, --help                        Display this help menu
    --stats                           Print summary statistics
    --delay=[version]                 Use delay optimization, versions: old,
                                      new, random, subset, stirling
    --check=[word]                    Product of the result with a given word
    --flow=[dataflow]                 Data flow analysis [light/inner]
    -w[value], --weight=[value]       Weight parameter for delay - value in
                                      <0,1>
    --no-elevator-rank                Don't update rank upper bound of each
                                      macrostate based on elevator automaton
                                      structure
    --det-beg                         Rank 0/1 to all states in the D/IW
                                      component in the beginning
    --eta4                            Max rank optimization - eta 4 only when
                                      going from some accepting state
    --elevator-test                   Test if INPUT is an elevator automaton
    --debug                           Print debug statistics
    --light                           Use lightweight optimizations
    --preprocess=[preprocess]         Preprocessing
                                      [copyiwa/copydet/copyall/copytrivial/copyheur]
    --acc-propagation                 Propagate accepting states in each SCC
```

### Citing

If you use _Ranker_ for academic purposes, please use the following citation

**Reducing (to) the Ranks: Efficient Rank-based Büchi Automata Complementation**. Vojtěch Havlena and Ondřej Lengál. In
*Proceedings of CONCUR'21*. ISSN 1868-8969. 2021
([url](https://drops.dagstuhl.de/opus/volltexte/2021/14379/))

### Authors

- Vojtěch Havlena `<ihavlena at fit.vutbr.cz>`
- Ondřej Lengál `<lengal at fit.vutbr.cz>`
- Barbora Šmahlíková `<xsmahl00 at fit.vutbr.cz>`
