#include "SemiDeterministicCompl.h"

BuchiAutomaton<StateSD, int> SemiDeterministicCompl::complementSD(ComplOptions opt) {
    //std::cerr << this->getDet().size() << " " << this->getNonDet().size() << std::endl;

    std::set<StateSD> initials;
    std::set<StateSD> states;
    std::set<StateSD> finals;
    std::map<std::pair<StateSD, int>, std::set<StateSD>> transitions;
    std::set<int> alphabet = this->getAlphabet();

    // initial states
    std::set<int> N;
    if (this->getNonDet().size() > 0)
        std::set_intersection(this->getNonDet().begin(), this->getNonDet().end(), this->getInitials().begin(), this->getInitials().end(), std::inserter(N, N.begin()));
    std::vector<int> C_union_S;
    if (this->getDet().size() > 0)
        std::set_intersection(this->getDet().begin(), this->getDet().end(), this->getInitials().begin(), this->getInitials().end(), std::back_inserter(C_union_S));

    for (const auto& subset : getSubsets(C_union_S)) {
        std::vector<int> subsetComplement;
        if (subset.size() > 0)
            std::set_difference(C_union_S.begin(), C_union_S.end(), subset.begin(), subset.end(), std::back_inserter(subsetComplement));
        else {
            std::vector<int> tmp(C_union_S.begin(), C_union_S.end());
            subsetComplement = tmp;
        }

        std::set<int> S(subset.begin(), subset.end());
        std::set<int> F_in_S;
        std::set_intersection(S.begin(), S.end(), this->getFinals().begin(), this->getFinals().end(), std::inserter(F_in_S, F_in_S.begin()));
        if (F_in_S.size() > 0)
            continue;

        std::set<int> C(subsetComplement.begin(), subsetComplement.end());
        StateSD init = {N, C, S, C};
        initials.insert(init);
        states.insert(init);
    }


    std::stack<StateSD> stack;
    for (const auto& init : initials)
        stack.push(init);
    while (stack.size() > 0){
        StateSD state = stack.top();
        stack.pop();

        // final states
        if (isSDStateFinal(state))
            finals.insert(state);

        for (const auto& symbol : alphabet){
            // get successors
            std::vector<StateSD> successors;
            if (!opt.ncsbLazy)
                successors = getSuccessorsOriginal(state, symbol);
            else
                successors = getSuccessorsLazy(state, symbol);

            for (const auto& succ : successors){
                //std::cerr << succ.toString() << std::endl;
                if (states.find(succ) == states.end()){
                    states.insert(succ);
                    stack.push(succ);
                }
            }

            // transitions
            set<StateSD> successorsSet(successors.begin(), successors.end());
            transitions[{state, symbol}] = successorsSet;
        }
    }

    auto ret = BuchiAutomaton<StateSD, int>(states, finals, initials, transitions, alphabet, getAPPattern());
    //std::cerr << ret.toString() << std::endl;
    return ret;
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
    std::set_intersection(state.C.begin(), state.C.end(), this->getFinals().begin(), this->getFinals().end(), std::back_inserter(F_in_C));
    std::set<int> tmp(F_in_C.begin(), F_in_C.end());
    std::set<int> F_in_C_not_S_reach = this->succSet(tmp, symbol);

    std::vector<int> remaining;
    std::set_union(N_to_det.begin(), N_to_det.end(), F_in_C_not_S_reach.begin(), F_in_C_not_S_reach.end(), std::back_inserter(remaining));
    std::vector<int> rem;
    std::set_difference(remaining.begin(), remaining.end(), C_prime_base.begin(), C_prime_base.end(), std::back_inserter(rem));
    remaining = rem;
    std::vector<int> rem2;
    std::set_difference(remaining.begin(), remaining.end(), S_prime_base.begin(), S_prime_base.end(), std::back_inserter(rem2));
    remaining = rem2;

    auto subsets = getSubsets(remaining);

    for (const auto& subset : subsets){
        StateSD newState;
        newState.N = N_prime;

        std::vector<int> subsetComplement;
        std::set_difference(remaining.begin(), remaining.end(), subset.begin(), subset.end(), std::back_inserter(subsetComplement));

        std::set_union(C_prime_base.begin(), C_prime_base.end(), subset.begin(), subset.end(), std::inserter(newState.C, newState.C.begin()));
        std::set_union(S_prime_base.begin(), S_prime_base.end(), subsetComplement.begin(), subsetComplement.end(), std::inserter(newState.S, newState.S.begin()));

        std::set<int> F_in_S;
        std::set_intersection(newState.S.begin(), newState.S.end(), this->getFinals().begin(), this->getFinals().end(), std::inserter(F_in_S, F_in_S.begin()));
        if (F_in_S.size() > 0)
            continue;

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

std::vector<StateSD> SemiDeterministicCompl::getSuccessorsLazy(StateSD state, int symbol){
    std::vector<StateSD> successors;

    // N'
    std::set<int> N_prime;
    if (state.N.size() > 0){
        auto succSet = this->succSet(state.N, symbol);
        std::set_intersection(succSet.begin(), succSet.end(), this->getNonDet().begin(), this->getNonDet().end(), std::inserter(N_prime, N_prime.begin()));
    }

    // S'
    std::set<int> S_prime_base = this->succSet(state.S, symbol);

    if (state.B.size() == 0){
        // C'
        std::set<int> C_prime_base;

        std::vector<int> N_to_det;
        auto succSet = this->succSet(state.N, symbol);
        std::set_intersection(succSet.begin(), succSet.end(), this->getDet().begin(), this->getDet().end(), std::back_inserter(N_to_det));
        std::vector<int> F_in_C;
        std::set_intersection(state.C.begin(), state.C.end(), this->getFinals().begin(), this->getFinals().end(), std::back_inserter(F_in_C));

        std::set<int> tmp(F_in_C.begin(), F_in_C.end());
        std::set<int> F_in_C_not_S_reach = this->succSet(tmp, symbol);

        std::vector<int> remaining;
        std::set_union(N_to_det.begin(), N_to_det.end(), F_in_C_not_S_reach.begin(), F_in_C_not_S_reach.end(), std::back_inserter(remaining));
        std::vector<int> rem;
        std::set_difference(remaining.begin(), remaining.end(), C_prime_base.begin(), C_prime_base.end(), std::back_inserter(rem));
        remaining = rem;
        std::vector<int> rem2;
        std::set_difference(remaining.begin(), remaining.end(), S_prime_base.begin(), S_prime_base.end(), std::back_inserter(rem2));
        remaining = rem2;

        auto subsets = getSubsets(remaining);

        for (auto subset : subsets){
            StateSD newState;
            newState.N = N_prime;

            std::vector<int> subsetComplement;
            std::set_difference(remaining.begin(), remaining.end(), subset.begin(), subset.end(), std::back_inserter(subsetComplement));

            std::set_union(C_prime_base.begin(), C_prime_base.end(), subset.begin(), subset.end(), std::inserter(newState.C, newState.C.begin()));
            std::set_union(S_prime_base.begin(), S_prime_base.end(), subsetComplement.begin(), subsetComplement.end(), std::inserter(newState.S, newState.S.begin()));

            std::set<int> F_in_S;
            std::set_intersection(newState.S.begin(), newState.S.end(), this->getFinals().begin(), this->getFinals().end(), std::inserter(F_in_S, F_in_S.begin()));
            if (F_in_S.size() > 0)
                continue;

            newState.B = newState.C;

            successors.push_back(newState);
        }
    }

    else {
        std::set<int> B_not_acc;
        if (state.B.size() > 0)
            std::set_difference(state.B.begin(), state.B.end(), this->getFinals().begin(), this->getFinals().end(), std::inserter(B_not_acc, B_not_acc.begin()));
        std::set<int> B_prime_base = this->succSet(B_not_acc, symbol);

        std::set<int> B_union_S;
        std::set_union(state.B.begin(), state.B.end(), state.S.begin(), state.S.end(), std::inserter(B_union_S, B_union_S.begin()));
        std::set<int> remaining;
        std::set_difference(B_union_S.begin(), B_union_S.end(), B_not_acc.begin(), B_not_acc.end(), std::inserter(remaining, remaining.begin()));
        std::set<int> remTmp = this->succSet(remaining, symbol);
        std::vector<int> remReach(remTmp.begin(), remTmp.end());

        std::vector<int> rem;
        std::set_difference(remReach.begin(), remReach.end(), B_prime_base.begin(), B_prime_base.end(), std::back_inserter(rem));
        remReach = rem;
        std::vector<int> rem2;
        std::set_difference(remReach.begin(), remReach.end(), S_prime_base.begin(), S_prime_base.end(), std::back_inserter(rem2));
        remReach = rem2;

        auto subsets = getSubsets(remReach);

        for (auto subset : subsets){
            StateSD newState;
            newState.N = N_prime;

            std::vector<int> subsetComplement;
            std::set_difference(remReach.begin(), remReach.end(), subset.begin(), subset.end(), std::back_inserter(subsetComplement));

            std::set_union(B_prime_base.begin(), B_prime_base.end(), subset.begin(), subset.end(), std::inserter(newState.B, newState.B.begin()));
            std::set_union(S_prime_base.begin(), S_prime_base.end(), subsetComplement.begin(), subsetComplement.end(), std::inserter(newState.S, newState.S.begin()));

            std::set<int> F_in_S;
            std::set_intersection(newState.S.begin(), newState.S.end(), this->getFinals().begin(), this->getFinals().end(), std::inserter(F_in_S, F_in_S.begin()));
            if (F_in_S.size() > 0)
                continue;

            // C'
            std::set<int> C_reach = this->succSet(state.C, symbol);
            std::vector<int> N_to_det;
            auto succSet = this->succSet(state.N, symbol);
            std::set_intersection(succSet.begin(), succSet.end(), this->getDet().begin(), this->getDet().end(), std::back_inserter(N_to_det));
            std::set<int> reachUnion;
            std::set_union(C_reach.begin(), C_reach.end(), N_to_det.begin(), N_to_det.end(), std::inserter(reachUnion, reachUnion.begin()));
            std::set_difference(reachUnion.begin(), reachUnion.end(), newState.S.begin(), newState.S.end(), std::inserter(newState.C, newState.C.begin()));

            successors.push_back(newState);
        }
    }

    return successors;
}

std::vector<std::vector<int>> SemiDeterministicCompl::getSubsets(std::vector<int> states){
    std::vector<std::vector<int>> subset;
    vector<int> empty;
    subset.push_back( empty );

    for (unsigned i = 0; i < states.size(); i++)
    {
        std::vector<std::vector<int> > subsetTemp = subset;

        for (unsigned j = 0; j < subsetTemp.size(); j++)
            subsetTemp[j].push_back(states[i]);

        for (unsigned j = 0; j < subsetTemp.size(); j++)
            subset.push_back(subsetTemp[j]);
    }
    return subset;
}
