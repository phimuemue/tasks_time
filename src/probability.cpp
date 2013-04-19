#include "probability.h"

Probability_Computer::Distribution_Setting Probability_Computer::distros_same(const Snapshot& s) const {
    bool all_distros_same = true;
    Distribution first = s.intree.get_task_distribution(s.marked[0]);
    for(auto it = s.marked.begin(); it!=s.marked.end(); ++it){
        if(s.intree.get_task_distribution(*it) != first){
            all_distros_same = false;
        }
    }
    if(all_distros_same == false)
        return Different_Distributions;
    auto u_a = s.intree.get_task_by_id(0).uniform_a;
    auto u_b = s.intree.get_task_by_id(0).uniform_b;
    switch(first){
        case(Exponential):
            for(auto it = s.marked.begin(); it!=s.marked.end(); ++it){
                if(s.intree.get_task_by_id(*it).exponential_lambda !=
                        s.intree.get_task_by_id(s.marked[0]).exponential_lambda){
                    return Same_Distributions_Different_Parameters;
                }
            }
            break;
        case(Uniform):
            for(auto it = s.marked.begin(); it!=s.marked.end(); ++it){
                if(s.intree.get_task_by_id(*it).uniform_a != u_a ||
                        s.intree.get_task_by_id(*it).uniform_b != u_b){
                    return Same_Distributions_Different_Parameters;
                }
            }
            break;
    }
    return Same_Distributions;
}

void Probability_Computer::simplify_isomorphisms_simple(const Snapshot& s, 
        vector<myfloat>& target) const{
    // if we are here, we can simplify!
    for (unsigned i1 = 0; i1 < s.marked.size(); ++i1){
        for (unsigned i2 = i1 + 1; i2 < s.marked.size(); ++i2){
            if(s.intree.edges.find(s.marked[i1])->second == 
                    s.intree.edges.find(s.marked[i2])->second){
#pragma omp critical
                {
                    target[i1] += target[i2];
                    target[i2] = 0;
                }
            }
        }
    }
}

void Probability_Computer::compute_finish_probs(const Snapshot& s,
        vector<myfloat>& target) const {
    // TODO: Implement this for all kinds of random variables
    Distribution first = s.intree.get_task_distribution(s.marked[0]);
    if (distros_same(s) == Same_Distributions || 
            distros_same(s) == Same_Distributions_Different_Parameters){
        // random variables are exponentially distributed
        myfloat prod_of_lambdas = 1;
        myfloat denom = 0;
        myfloat numer = 1;
        // all variables are uniformly distributed
        myfloat u_a = s.intree.get_task_by_id(s.marked[0]).uniform_a;
        myfloat u_b = s.intree.get_task_by_id(s.marked[0]).uniform_b;
        switch(first){
        case(Exponential):
            for(auto it = s.marked.begin(); it!=s.marked.end(); ++it){
                Task tmp = s.intree.get_task_by_id(*it);
                prod_of_lambdas *= tmp.exponential_lambda;
            }
            for(auto it = s.marked.begin(); it!=s.marked.end(); ++it){
                denom += prod_of_lambdas / s.intree.get_task_by_id(*it).exponential_lambda;
            }
            for(auto it = s.marked.begin(); it!=s.marked.end(); ++it){
                numer = prod_of_lambdas / s.intree.get_task_by_id(*it).exponential_lambda;
                target.push_back(
                    numer / denom
                );
            }
            break;
        case(Uniform):
            for(auto it = s.marked.begin(); it!=s.marked.end(); ++it){
                if(s.intree.get_task_by_id(*it).uniform_a != u_a ||
                        s.intree.get_task_by_id(*it).uniform_b != u_b){
                    throw "Cannot compute finish probabilities if not all "
                          "rv's are uniformly distributed with same parameters";
                }
                for(auto it = s.marked.begin(); it!=s.marked.end(); ++it){
                    target.push_back(((myfloat)1)/(myfloat)s.marked.size());
                }
            }
            break;
        }
        // if all variables are equally distributed, 
        // we can do some simplification
#if SIMPLE_ISOMORPHISM_CHECK==1
        // for safety, we check whether we can even simplify
        if(distros_same(s) != Same_Distributions){
            throw "No simplify if rv's are not iid with same parameters";
        }
        simplify_isomorphisms_simple(s, target);
#endif
    }
    else {
        throw 1;
    }
}
