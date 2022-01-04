#include "SemiDeterministicCompl.h"

BuchiAutomaton<StateSD, int> SemiDeterministicCompl::complementSD(ComplOptions opt) {
    if (opt.ncsbLazy)
        this->ncsbTransform();
    //std::cerr << this->toString() << std::endl;

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

    // one initial state
    std::set<int> C_union_S_set(C_union_S.begin(), C_union_S.end());
    StateSD init = {N, C_union_S_set, std::set<int>(), C_union_S_set};
    initials.insert(init);
    states.insert(init);

    std::stack<StateSD> stack;
    for (const auto& init : initials)
        stack.push(init);
    while (stack.size() > 0){
        StateSD state = stack.top();
        stack.pop();

        // final states
        if (isSDStateFinal(state))
            finals.insert(state);

        for (const auto& symbol : alphabet)
        {
            // get successors
            std::vector<StateSD> successors;
            if (!opt.ncsbLazy)
            {
                //successors = getSuccessorsOriginal(state, symbol);
                successors = getSuccessorsMaxRank(state, symbol);
            }
            else
            {
                successors = getSuccessorsLazy(state, symbol);
            }

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


std::vector<StateSD> SemiDeterministicCompl::getSuccessorsMaxRank(StateSD& state, int symbol)
{
    std::vector<StateSD> successors;
    auto fin = this->getFinals();

    StateSD succ1;

    auto NsuccSet = this->succSet(state.N, symbol);
    std::set_intersection(NsuccSet.begin(), NsuccSet.end(), this->getNonDet().begin(), this->getNonDet().end(), std::inserter(succ1.N, succ1.N.begin()));

    std::vector<int> NtoDet;
    std::set_intersection(NsuccSet.begin(), NsuccSet.end(), this->getDet().begin(), this->getDet().end(), std::back_inserter(NtoDet));

    succ1.C = this->succSet(state.C, symbol);
    succ1.C.insert(NtoDet.begin(), NtoDet.end());

    auto SsuccSet = this->succSet(state.S, symbol);

    set<int> finReachS;
    for (auto tr : this->getFinTrans())
    {
        if (tr.symbol == symbol and state.S.find(tr.from) != state.S.end() and SsuccSet.find(tr.to) != SsuccSet.end())
        {
            finReachS.insert(tr.to);
        }
    }
    set<int> allFins;
    std::set_union(fin.begin(), fin.end(), finReachS.begin(), finReachS.end(), std::inserter(allFins, allFins.begin()));

    set<int> FinS;
    std::set_intersection(SsuccSet.begin(), SsuccSet.end(), allFins.begin(), allFins.end(), std::inserter(FinS, FinS.begin()));
    if(FinS.size() > 0)
    {
      return successors;
    }

    set<int> tmp;
    std::set_difference(succ1.C.begin(), succ1.C.end(), SsuccSet.begin(), SsuccSet.end(), std::inserter(tmp, tmp.begin()));
    succ1.C = tmp;
    succ1.S = SsuccSet;

    if(state.B.size() == 0)
    {
      succ1.B = succ1.C;
    }
    else
    {
      auto succB = this->succSet(state.B, symbol);
      std::set_intersection(succB.begin(), succB.end(), succ1.C.begin(), succ1.C.end(), std::inserter(succ1.B, succ1.B.begin()));
    }

    StateSD succ2 = succ1;
    set<int> rem;
    succ2.B = set<int>();
    std::set_difference(succ1.B.begin(), succ1.B.end(), allFins.begin(), allFins.end(), std::inserter(rem, rem.begin()));
    std::set_intersection(succ1.B.begin(), succ1.B.end(), allFins.begin(), allFins.end(), std::inserter(succ2.B, succ2.B.begin()));
    succ2.S.insert(rem.begin(), rem.end());
    set<int> aux;
    std::set_difference(succ2.C.begin(), succ2.C.end(), rem.begin(), rem.end(), std::inserter(aux, aux.begin()));
    succ2.C = aux;

    aux = set<int>();
    std::set_intersection(succ1.B.begin(), succ1.B.end(), fin.begin(), fin.end(), std::inserter(aux, aux.begin()));
    if(aux.size() == 0)
    {
      successors.push_back(succ2);
    }

    successors.push_back(succ1);

    return successors;
}


std::vector<StateSD> SemiDeterministicCompl::getSuccessorsOriginal(StateSD& state, int symbol){
    std::vector<StateSD> successors;

    std::set<int> N_prime;
    if (state.N.size() > 0){
        auto succSet = this->succSet(state.N, symbol);
        std::set_intersection(succSet.begin(), succSet.end(), this->getNonDet().begin(), this->getNonDet().end(), std::inserter(N_prime, N_prime.begin()));
    }

    std::set<int> S_prime_base = this->succSet(state.S, symbol);

    std::set<int> C_not_acc;
    set<int> CFinTrans;
    for (auto tr : this->getFinTrans()){
        if (tr.symbol == symbol and state.C.find(tr.from) != state.C.end()){
            CFinTrans.insert(tr.from);
        }
    }
    set<int> allFins;
    std::set_union(this->getFinals().begin(), this->getFinals().end(), CFinTrans.begin(), CFinTrans.end(), std::inserter(allFins, allFins.begin()));
    std::set_difference(state.C.begin(), state.C.end(), allFins.begin(), allFins.end(), std::inserter(C_not_acc, C_not_acc.begin()));
    std::set<int> C_prime_base = this->succSet(C_not_acc, symbol);

    std::vector<int> N_to_det;
    auto succSet = this->succSet(state.N, symbol);
    std::set_intersection(succSet.begin(), succSet.end(), this->getDet().begin(), this->getDet().end(), std::back_inserter(N_to_det));
    std::vector<int> F_in_C;
    std::set_intersection(state.C.begin(), state.C.end(), allFins.begin(), allFins.end(), std::back_inserter(F_in_C));
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

    auto subsets = Aux::getAllSubsets(remaining);

    for (const auto& subset : subsets){
        StateSD newState;
        newState.N = N_prime;

        std::vector<int> subsetComplement;
        std::set_difference(remaining.begin(), remaining.end(), subset.begin(), subset.end(), std::back_inserter(subsetComplement));

        std::set_union(C_prime_base.begin(), C_prime_base.end(), subset.begin(), subset.end(), std::inserter(newState.C, newState.C.begin()));

        std::set_union(S_prime_base.begin(), S_prime_base.end(), subsetComplement.begin(), subsetComplement.end(), std::inserter(newState.S, newState.S.begin()));

        set<int> SFinTrans;
        for (auto tr : this->getFinTrans()){
            if (tr.symbol == symbol and state.S.find(tr.from) != state.S.end()){
                SFinTrans.insert(tr.to);
            }
        }

        allFins.clear();
        std::set_union(this->getFinals().begin(), this->getFinals().end(), SFinTrans.begin(), SFinTrans.end(), std::inserter(allFins, allFins.begin()));

        std::set<int> F_in_S;
        std::set_intersection(newState.S.begin(), newState.S.end(), allFins.begin(), allFins.end(), std::inserter(F_in_S, F_in_S.begin()));
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

std::vector<StateSD> SemiDeterministicCompl::getSuccessorsLazy(StateSD& state, int symbol){
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

        std::set<int> C_reach = this->succSet(state.C, symbol);
        std::set<int> union_reach;
        std::set_union(N_to_det.begin(), N_to_det.end(), C_reach.begin(), C_reach.end(), std::inserter(union_reach, union_reach.begin()));
        std::vector<int> remaining;
        std::set_difference(union_reach.begin(), union_reach.end(), S_prime_base.begin(), S_prime_base.end(), std::back_inserter(remaining));

        auto subsets = Aux::getAllSubsets(remaining);

        for (const auto& subset : subsets){
            StateSD newState;
            newState.N = N_prime;

            std::vector<int> subsetComplement;
            std::set_difference(remaining.begin(), remaining.end(), subset.begin(), subset.end(), std::back_inserter(subsetComplement));

            std::set_union(C_prime_base.begin(), C_prime_base.end(), subset.begin(), subset.end(), std::inserter(newState.C, newState.C.begin()));

            std::set_union(S_prime_base.begin(), S_prime_base.end(), subsetComplement.begin(), subsetComplement.end(), std::inserter(newState.S, newState.S.begin()));

            set<int> SFinTrans;
            for (auto tr : this->getFinTrans()){
                if (tr.symbol == symbol and state.S.find(tr.from) != state.S.end()){
                    SFinTrans.insert(tr.to);
                }
            }

            std::set<int> allFins;
            std::set_union(this->getFinals().begin(), this->getFinals().end(), SFinTrans.begin(), SFinTrans.end(), std::inserter(allFins, allFins.begin()));

            std::set<int> F_in_S;
            std::set_intersection(newState.S.begin(), newState.S.end(), allFins.begin(), allFins.end(), std::inserter(F_in_S, F_in_S.begin()));
            if (F_in_S.size() > 0)
                continue;

            newState.B = newState.C;

            successors.push_back(newState);
        }
    }

    else {
        set<int> BFinTrans;
        for (auto tr : this->getFinTrans()){
            if (tr.symbol == symbol and state.B.find(tr.from) != state.B.end()){
                BFinTrans.insert(tr.from);
            }
        }

        set<int> allFins;
        std::set_union(this->getFinals().begin(), this->getFinals().end(), BFinTrans.begin(), BFinTrans.end(), std::inserter(allFins, allFins.begin()));

        std::set<int> B_not_acc;
        if (state.B.size() > 0){
            std::set_difference(state.B.begin(), state.B.end(), allFins.begin(), allFins.end(), std::inserter(B_not_acc, B_not_acc.begin()));
        }
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

        auto subsets = Aux::getAllSubsets(remReach);

        for (const auto& subset : subsets){
            StateSD newState;
            newState.N = N_prime;

            std::vector<int> subsetComplement;
            std::set_difference(remReach.begin(), remReach.end(), subset.begin(), subset.end(), std::back_inserter(subsetComplement));

            std::set_union(B_prime_base.begin(), B_prime_base.end(), subset.begin(), subset.end(), std::inserter(newState.B, newState.B.begin()));

            std::set_union(S_prime_base.begin(), S_prime_base.end(), subsetComplement.begin(), subsetComplement.end(), std::inserter(newState.S, newState.S.begin()));

            set<int> SFinTrans;
            for (auto tr : this->getFinTrans()){
                if (tr.symbol == symbol and state.S.find(tr.from) != state.S.end()){
                    SFinTrans.insert(tr.to);
                }
            }

            allFins.clear();
            std::set_union(this->getFinals().begin(), this->getFinals().end(), SFinTrans.begin(), SFinTrans.end(), std::inserter(allFins, allFins.begin()));

            std::set<int> F_in_S;
            std::set_intersection(newState.S.begin(), newState.S.end(), allFins.begin(), allFins.end(), std::inserter(F_in_S, F_in_S.begin()));
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

void SemiDeterministicCompl::ncsbTransform() {
    // get D start states
    std::set<int> detStart;
    auto nonDet = this->getNonDet();
    auto det = this->getDet();
    for (auto tr : this->getTransitions()){
        if (nonDet.find(tr.first.first) != nonDet.end()){
            for (auto second : tr.second){
                if (det.find(second) != det.end() and this->getFinals().find(second) == this->getFinals().end())
                    detStart.insert(second);
            }
        }
    }

    for (auto state : detStart){
        // add new accepting state
        int newState = this->getStates().size();
        this->addStates(newState);
        this->addFinals(newState);
        this->addDetState(newState);
        auto trans = this->getTransitions();
        for (auto& tr : trans){
            // redirect transitions to 'state' to the new state
            auto it = tr.second.find(state);
            if (it != tr.second.end()){
                tr.second.erase(state);
                tr.second.insert(newState);
            }

            // duplicate all outgoing transitions of 'state' to the new state
            if (tr.first.first == state){
                if (this->getTransitions().count({newState, tr.first.second}) == 0){
                    // transition does not exist
                    this->addNewTransition({newState, tr.first.second}, tr.second);
                } else {
                    // transition already exists
                    this->addNewStatesToTransition({newState, tr.first.second}, tr.second);
                }
            }
        }
    }
}
