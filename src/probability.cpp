#include "probability.h"

void Probability_Computer::compute_finish_probs(const Snapshot& s,
        vector<myfloat>& target) const {
    // TODO: Implement this for all kinds of random variables
    bool all_distros_same = true;
    Distribution first = s.intree.get_task_distribution(s.marked[0]);
    for(auto it = s.marked.begin(); it!=s.marked.end(); ++it){
        if(s.intree.get_task_distribution(*it) != first){
            all_distros_same = false;
        }
    }
    if (all_distros_same){
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
                //target.push_back(((myfloat)1)/(myfloat)s.marked.size());
            }
            return;
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
        }
    }
    throw 1;
}
