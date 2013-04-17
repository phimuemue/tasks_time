#ifndef PROBABILITY_H
#define PROBABILITY_H

#include "config.h"
#include "snapshot.h"

class Snapshot;

class Probability_Computer{
    public:
        void compute_finish_probs(const Snapshot& s,
                                  vector<myfloat>& target) const;
        void expected_runtime_of_min_task(const Snapshot& s) const;
};

#endif
