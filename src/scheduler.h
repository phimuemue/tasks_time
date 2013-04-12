#ifndef SCHEDULER_H
#define SCHEDULER_H

#include<vector>
#include<utility>
#include<algorithm>

#include "config.h"
#include "intree.h"

using namespace std;

class Scheduler {
    // TODO: Better if the following was pure virtual
    // TODO: possibly better if only one Snapshot was the argument
    public:
        virtual void get_next_tasks(const Intree& t, 
                                    const vector<task_id>& marked,
                                    vector<pair<task_id,myfloat>>& target) const = 0;
};

#endif
