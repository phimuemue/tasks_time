#ifndef PROBABILITY_H
#define PROBABILITY_H

#include "config.h"
#include "snapshot.h"

class Snapshot;

class Probability_Computer{
    private:
        enum Distribution_Setting{
            Same_Distributions,
            Same_Distributions_Different_Parameters,
            Different_Distributions
        };
        Distribution_Setting distros_same(const Snapshot& s) const;
    public:
        void compute_finish_probs(const Snapshot& s,
                                  vector<myfloat>& target) const;
        void expected_runtime_of_min_task(const Snapshot& s) const;
};

#endif
