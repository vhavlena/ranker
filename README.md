# Ranker: A Tool for Buchi Automata Complementation

The repository contains _Ranker_, a tool tool for complementation of Buchi
automata. _Ranker_ implements several optimized constructions tailored for
certain automata types (e.g., inherently weak, semi-deterministic, or general
nondeterministic). Currently supported types of omega-automata:
- Buchi automata with mixed transition-based/state-based acceptance condition
- Generalized co-Buchi automata

### Installation

Prerequisites:
```
- gcc >= 8.3.0
- boost library
```

Install using `make ranker` in `src` folder. Another option is to use `cmake`
in `src` folder as follows:
```sh
mkdir build && cd build
cmake ..
make
```
You also need to specify a correct
path to `autfilt` (not necessary if the `--no-backoff` option is set) via
```sh
export SPOTEXE=<path to autfilt>
```

### Input Automata Format

The tool accepts automata in the *HOA* format (Hanoi Omega Automata) as well as automata in the *BA* format.
Moreover, the tool does not support aliases or any other fancy features of HOA.

### Running

To run _Ranker_ use the following command:
```sh
./ranker [INPUT] {OPTIONS}
```
where `INPUT` is an input automaton (if the
file is not provided, the automaton is taken from the standard input) and `OPTIONS` are from the following:
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
  --preprocess=[value...]           Preprocessing
                                    [copyiwa/copydet/copyall/copytrivial/copyheur/accsat/no-red]
  --sd                              Use semideterminization
  --iw-sim                          Use direct simulation
  --iw-sat                          Macrostates saturation
  --no-backoff                      Do NOT use backoff
  --version                         Git commit version
  --ncsb-lazy                       Use NCSB-lazy for SD complementation
  --no-tba                          Do NOT use TBA preprocessing
  --best                            Use the settings leading to smallest
                                    possible automata
```

### Evaluation and Benchmarks

Benchmarks and automated evaluation scripts can be found in the [Benchmark evaluation environment](https://github.com/VeriFIT/ba-compl-eval). Follow the README file to install other tools and run the evaluation scripts.

### Publications

_Ranker_ is based on the following papers:

- **Sky Is Not the Limit: Tighter Rank Bounds for Elevator Automata in Büchi Automata Complementation**. Vojtěch Havlena, Ondřej Lengál, and Barbora Šmahlíková. In *Proceedings of TACAS'22*. 2022. ([url](https://link.springer.com/chapter/10.1007/978-3-030-99527-0_7))
- **Reducing (to) the Ranks: Efficient Rank-based Büchi Automata Complementation**. Vojtěch Havlena and Ondřej Lengál. In
  *Proceedings of CONCUR'21*. ISSN 1868-8969. 2021.
  ([url](https://drops.dagstuhl.de/opus/volltexte/2021/14379/))
- **Simulations in rank-based Büchi automata complementation**. Yu-Fang Chen, Vojtěch Havlena, and Ondřej Lengál. In *Proceedings of APLAS'19*. 2019. ([url](https://link.springer.com/chapter/10.1007/978-3-030-34175-6_23))

### Authors

- Vojtěch Havlena `<ihavlena at fit.vutbr.cz>`
- Ondřej Lengál `<lengal at fit.vutbr.cz>`
- Barbora Šmahlíková `<xsmahl00 at fit.vutbr.cz>`
