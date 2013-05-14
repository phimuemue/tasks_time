#ifndef LEAFSCHEDULER_H
#define LEAFSCHEDULER_H

#include "scheduler.h"

class Leafscheduler : public Scheduler {
    protected:
        void all_possible_combinations(
            const vector<task_id>& t,
            const unsigned int n,
            const unsigned int minindex,
            vector<vector<task_id>>& target
        ) const;
    public:
        void get_initial_schedule(const Intree& t, 
                const unsigned int, 
                vector<vector<task_id>>&) const;
        void get_next_tasks(const Intree& t, 
                const vector<task_id>& marked,
                vector<pair<task_id,myfloat>>& target) const;
};

#endif
