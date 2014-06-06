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
        schedulerchoice get_next_tasks (
            const Intree& t, 
            const vector<task_id>& marked
        ) const override;
};

#endif
