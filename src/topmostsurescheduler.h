#ifndef TOPMOSTSURESCHEDULER_H
#define TOPMOSTSURESCHEDULER_H

#include <algorithm>

#include "leafscheduler.h"

class TopMostSureScheduler : public Leafscheduler {
    protected:
    public:
        void get_initial_schedule(const Intree& t, 
                const unsigned int, 
                vector<vector<task_id>>&) const;
        virtual void get_next_tasks(const Intree& t, 
                const vector<task_id>& marked,
                vector<pair<task_id,myfloat>>& target) const;
};

#endif
