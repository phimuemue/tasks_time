#ifndef LEAFSCHEDULER_H
#define LEAFSCHEDULER_H

#include "scheduler.h"

class Leafscheduler : public Scheduler {
    public:
        void get_next_tasks(const Intree& t, 
                const vector<task_id>& marked,
                vector<pair<task_id,myfloat>>& target) const;
};

#endif
