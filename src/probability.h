#ifndef PROBABILITY_H
#define PROBABILITY_H

#include "config.h"
#include "snapshot.h"

class Probability_Computer{
    private:
        enum Distribution_Setting{
            Same_Distributions,
            Same_Distributions_Different_Parameters,
            Different_Distributions
        };
        Distribution_Setting distros_same(const Intree& intree, const vector<task_id>& marked) const;
        void simplify_isomorphisms_simple(const Intree& intree, const vector<task_id>& marked, vector<myfloat>& target) const;
    public:
        void compute_finish_probs(const Intree& intree, const vector<task_id>& marked, vector<myfloat>& target) const;
        void expected_runtime_of_min_task(const Intree& intree, const vector<task_id>& marked) const;
myfloat get_expected_remaining_time(const Intree& t, const task_id tid);
};

#endif
