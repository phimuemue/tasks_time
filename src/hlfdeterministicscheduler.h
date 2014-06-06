#ifndef HLFDETERMINISTICSCHEDULER_H
#define HLFDETERMINISTICSCHEDULER_H

#include "hlfscheduler.h"

#include <algorithm>

class HLFDeterministicScheduler : public HLFscheduler {
public:
    schedulerchoice get_next_tasks(
        const Intree& t, 
        const vector<task_id>& marked
    ) const override;
};

#endif
