#ifndef HLFRANDOMSCHEDULER_H
#define HLFRANDOMSCHEDULER_H

#include "hlfscheduler.h"

#include <algorithm>
#include <random>

class HLFRandomScheduler : public HLFscheduler {
    private:
        mutable mt19937 rng;
    public:
        HLFRandomScheduler();
        void get_next_tasks(const Intree& t, 
                const vector<task_id>& marked,
                vector<pair<task_id,myfloat>>& target) const;
};

#endif
