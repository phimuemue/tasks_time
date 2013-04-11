#ifndef SCHEDULER_H
#define SCHEDULER_H

#include<vector>
#include<utility>

#include "snapshot.h"

using namespace std;

class Scheduler {
    // TODO: Better if the following was pure virtual
    // TODO: possibly better if only one Snapshot was the argument
    public:
        virtual void get_next_tasks(Intree t, 
                                    vector<task_id> marked,
                                    vector<pair<Intree,myfloat>> target);
};

#endif
