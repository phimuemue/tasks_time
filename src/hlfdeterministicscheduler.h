#ifndef HLFDETERMINISTICSCHEDULER_H
#define HLFDETERMINISTICSCHEDULER_H

#include <algorithm>

#include "leafscheduler.h"

class HLFDeterministicScheduler : public HLFscheduler {
    public:
        void get_next_tasks(const Intree& t, 
                const vector<task_id>& marked,
                vector<pair<task_id,myfloat>>& target) const;
};

#endif
