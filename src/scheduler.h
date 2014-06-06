#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "intree.h"
#include "config.h"

#include<vector>
#include<utility>
#include<algorithm>

using namespace std;

class Scheduler {
    protected:
        using schedulerchoice = vector<pair<vector<task_id>, myfloat>>;
    public:
        virtual ~Scheduler();
        virtual void get_initial_schedule(
            const Intree& t, 
            vector<vector<task_id>>&
        ) const = 0;
        virtual schedulerchoice get_next_tasks(
            const Intree& t, 
            const vector<task_id>& marked
        ) const = 0;
        void set_processorcount(unsigned int n);
    protected:
        unsigned int processorcount = 0;
};

#endif
