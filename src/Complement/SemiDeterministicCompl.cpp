#include "SemiDeterministicCompl.h"

BuchiAutomaton<StateSD, int> SemiDeterministicCompl::complementSD(bool original) {
    std::cerr << "SD complement" << std::endl;
    std::cerr << this->getDet().size() << " " << this->getNonDet().size() << std::endl;

    std::set<StateSD> initials;
    std::set<StateSD> states;
    std::set<StateSD> finals;
    std::map<std::pair<StateSD, int>, std::set<StateSD>> transitions;
    std::set<int> alphabet = this->getAlphabet();
    
    // initial state
    std::set<int> N;
    if (this->getNonDet().size() > 0)
        std::set_intersection(this->getNonDet().begin(), this->getNonDet().end(), this->getInitials().begin(), this->getInitials().end(), std::inserter(N, N.begin()));
    std::set<int> C;
    if (this->getDet().size() > 0)
        std::set_intersection(this->getDet().begin(), this->getDet().end(), this->getInitials().begin(), this->getInitials().end(), std::inserter(C, C.begin()));
    std::set<int> S;
    StateSD init = {N, C, S, C};
    initials.insert(init);


    std::stack<StateSD> stack;
    stack.push(init);
    while (stack.size() > 0){
        // TODO pop
        StateSD state = stack.top();
        stack.pop();   

        // TODO final states
        if (isSDStateFinal(state))
            finals.insert(state); 

        // TODO get successors


        // TODO push successors to stack if not in states

        // TODO insert successors into states if not there already

        // TODO transitions
    }

    return BuchiAutomaton<StateSD, int>(states, finals, initials, transitions, alphabet, getAPPattern());
}

std::vector<StateSD> SemiDeterministicCompl::getSuccessorsOriginal(StateSD state, int symbol){
    std::vector<StateSD> successors;
    
    std::set<int> N_prime;
    if (state.N.size() > 0){
        auto succSet = this->succSet(state.N, symbol);
        std::set_intersection(succSet.begin(), succSet.end(), this->getNonDet().begin(), this->getNonDet().end(), std::inserter(N_prime, N_prime.begin()));
    }

    std::set<int> S_prime_base = this->succSet(state.S, symbol);
    std::set<int> C_not_acc;
    if (state.C.size() > 0)
        std::set_difference(state.C.begin(), state.C.end(), this->getFinals().begin(), this->getFinals().end(), std::inserter(C_not_acc, C_not_acc.begin()));
    std::set<int> C_prime_base = this->succSet(C_not_acc, symbol);

    std::vector<int> N_to_det;
    auto succSet = this->succSet(state.N, symbol);
    std::set_intersection(succSet.begin(), succSet.end(), this->getDet().begin(), this->getDet().end(), std::back_inserter(N_to_det));
    std::vector<int> F_in_C;
    std::set_intersection(state.C.begin(), state.C.end(), C_not_acc.begin(), C_not_acc.end(), std::back_inserter(F_in_C));
    std::vector<int> F_in_C_not_S;
    std::set_difference(F_in_C.begin(), F_in_C.end(), state.S.begin(), state.S.end(), std::back_inserter(F_in_C_not_S));
    std::vector<int> remaining;
    std::set_union(N_to_det.begin(), N_to_det.end(), F_in_C_not_S.begin(), F_in_C_not_S.end(), std::back_inserter(remaining));

    auto subsets = getSubsets(remaining);

    for (auto subset : subsets){
        StateSD newState;
        newState.N = N_prime;

        std::vector<int> subsetComplement;
        std::set_difference(remaining.begin(), remaining.end(), subset.begin(), subset.end(), std::back_inserter(subsetComplement));

        std::set_union(C_prime_base.begin(), C_prime_base.end(), subset.begin(), subset.end(), std::inserter(newState.C, newState.C.begin()));
        std::set_union(S_prime_base.begin(), S_prime_base.end(), subsetComplement.begin(), subsetComplement.end(), std::inserter(newState.S, newState.S.begin()));

        if (state.B.size() == 0)
            newState.B = newState.C;
        else {
            auto succSet = this->succSet(state.B, symbol);
            std::set_intersection(succSet.begin(), succSet.end(), newState.C.begin(), newState.C.end(), std::inserter(newState.B, newState.B.begin()));
        }

        successors.push_back(newState);
    }

    return successors;
}

std::vector<std::vector<int>> SemiDeterministicCompl::getSubsets(std::vector<int> states){
    std::vector<std::vector<int>> subset;
    vector<int> empty;
    subset.push_back( empty );

    for (unsigned i = 0; i < states.size(); i++)
    {
        std::vector<std::vector<int> > subsetTemp = subset;  //making a copy of given 2-d vector.

        for (unsigned j = 0; j < subsetTemp.size(); j++)
            subsetTemp[j].push_back(states[i]);   // adding set[i] element to each subset of subsetTemp. like adding {2}(in 2nd iteration  to {{},{1}} which gives {{2},{1,2}}.

        for (unsigned j = 0; j < subsetTemp.size(); j++)
            subset.push_back(subsetTemp[j]);  //now adding modified subsetTemp to original subset (before{{},{1}} , after{{},{1},{2},{1,2}}) 
    }
    return subset;
}