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
        // variables if all variables are exponentially distributed
        myfloat prod_of_lambdas = 1;
        myfloat denom = 0;
        myfloat numer = 1;
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
            throw "To be implemented";
        }
    }
    throw 1;
}
