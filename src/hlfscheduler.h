#ifndef HLFSCHEDULER_H
#define HLFSCHEDULER_H

#include "leafscheduler.h"

class HLFscheduler : public Leafscheduler {
    public:
        void get_next_tasks(const Intree& t, 
                const vector<task_id>& marked,
                vector<pair<task_id,myfloat>>& target) const;
};

#endif
