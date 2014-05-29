#ifndef TOPMOSTSURESCHEDULER_H
#define TOPMOSTSURESCHEDULER_H

#include "leafscheduler.h"

#include <algorithm>

class TopMostSureScheduler : public Leafscheduler {
    protected:
    public:
        void get_initial_schedule(const Intree& t, 
                vector<vector<task_id>>&) const;
        virtual void get_next_tasks(const Intree& t, 
                const vector<task_id>& marked,
                vector<pair<vector<task_id>,myfloat>>& target) const;
};

#endif
