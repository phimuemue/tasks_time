#include "probability.h"

myfloat Probability_Computer::get_expected_remaining_time(
        const Intree& t, 
        const task_id tid
        )
{
#if USE_TASKMAP
    return t.get_task_by_id(tid).get_expected_remaining_time();
#else
    return 1;
#endif
}

Probability_Computer::Distribution_Setting Probability_Computer::distros_same(const Intree& intree, const vector<task_id>& marked) const {
#if !USE_TASKMAP
    return Same_Distributions;
#else
    bool all_distros_same = true;
    Distribution first = intree.get_task_distribution(marked[0]);
    for(auto it = marked.begin(); it!=marked.end(); ++it){
        if(intree.get_task_distribution(*it) != first){
            all_distros_same = false;
        }
    }
    if(all_distros_same == false)
        return Different_Distributions;
    auto u_a = intree.get_task_by_id(0).uniform_a;
    auto u_b = intree.get_task_by_id(0).uniform_b;
    switch(first){
        case(Exponential):
            for(auto it = marked.begin(); it!=marked.end(); ++it){
                if(intree.get_task_by_id(*it).exponential_lambda !=
                        intree.get_task_by_id(marked[0]).exponential_lambda){
                    return Same_Distributions_Different_Parameters;
                }
            }
            break;
        case(Uniform):
            for(auto it = marked.begin(); it!=marked.end(); ++it){
                if(intree.get_task_by_id(*it).uniform_a != u_a ||
                        intree.get_task_by_id(*it).uniform_b != u_b){
                    return Same_Distributions_Different_Parameters;
                }
            }
            break;
    }
    return Same_Distributions;
#endif
}

void Probability_Computer::simplify_isomorphisms_simple(const Intree& intree, const vector<task_id>& marked, vector<myfloat>& target) const{
    // if we are here, we can simplify!
    for (unsigned i1 = 0; i1 < marked.size(); ++i1){
        for (unsigned i2 = i1 + 1; i2 < marked.size(); ++i2){
            if(intree.edges.find(marked[i1])->second == 
                    intree.edges.find(marked[i2])->second){
#pragma omp critical
                {
                    target[i1] += target[i2];
                    target[i2] = 0;
                }
            }
        }
    }
}

void Probability_Computer::compute_finish_probs(const Intree& intree, const vector<task_id>& marked, vector<myfloat>& target) const {
#if !USE_TASKMAP
    // this is the easy case
    myfloat prob = ((myfloat)1)/((myfloat)marked.size());
    for(unsigned int i=0; i<marked.size(); ++i){
        target.push_back(prob);
    }
#else
    // TODO: Implement this for all kinds of random variables
    Distribution first = intree.get_task_distribution(marked[0]);
    if (distros_same(intree, marked) == Same_Distributions || 
            distros_same(intree, marked) == Same_Distributions_Different_Parameters){
        // random variables are exponentially distributed
        myfloat prod_of_lambdas = 1;
        myfloat denom = 0;
        myfloat numer = 1;
        // all variables are uniformly distributed
        myfloat u_a = intree.get_task_by_id(marked[0]).uniform_a;
        myfloat u_b = intree.get_task_by_id(marked[0]).uniform_b;
        switch(first){
        case(Exponential):
            for(auto it = marked.begin(); it!=marked.end(); ++it){
                Task tmp = intree.get_task_by_id(*it);
                prod_of_lambdas *= tmp.exponential_lambda;
            }
            for(auto it = marked.begin(); it!=marked.end(); ++it){
                denom += prod_of_lambdas / intree.get_task_by_id(*it).exponential_lambda;
            }
            for(auto it = marked.begin(); it!=marked.end(); ++it){
                numer = prod_of_lambdas / intree.get_task_by_id(*it).exponential_lambda;
                target.push_back(
                    numer / denom
                );
            }
            break;
        case(Uniform):
            for(auto it = marked.begin(); it!=marked.end(); ++it){
                if(intree.get_task_by_id(*it).uniform_a != u_a ||
                        intree.get_task_by_id(*it).uniform_b != u_b){
                    throw "Cannot compute finish probabilities if not all "
                          "rv's are uniformly distributed with same parameters";
                }
                for(auto it = marked.begin(); it!=marked.end(); ++it){
                    target.push_back(((myfloat)1)/(myfloat)marked.size());
                }
            }
            break;
        }
        // if all variables are equally distributed, 
        // we can do some simplification
#if SIMPLE_ISOMORPHISM_CHECK==1
        // for safety, we check whether we can even simplify
        if(distros_same(intree, marked) != Same_Distributions){
            throw "No simplify if rv's are not iid with same parameters";
        }
        simplify_isomorphisms_simple(intree, marked, target);
#endif
    }
    else {
        cout << "Not possible to compute finish_probs." << endl;
        throw 1;
    }
#endif
}
