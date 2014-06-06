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
                vector<vector<task_id>>&) const;
        vector<pair<vector<task_id>, myfloat>> get_next_tasks(
            const Intree& t, 
            const vector<task_id>& marked
        ) const override;
};

#endif
