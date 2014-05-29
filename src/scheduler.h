#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "intree.h"
#include "config.h"

#include<vector>
#include<utility>
#include<algorithm>

using namespace std;

class Scheduler {
    public:
        virtual ~Scheduler();
        virtual void get_initial_schedule(
            const Intree& t, 
            vector<vector<task_id>>&
        ) const = 0;
        virtual void get_next_tasks(
            const Intree& t, 
            const vector<task_id>& marked,
            vector<pair<vector<task_id>,myfloat>>& target
        ) const = 0;
        void set_processorcount(unsigned int n);
    protected:
        unsigned int processorcount = 0;
};

#endif
