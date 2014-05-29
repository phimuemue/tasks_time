#ifndef HLFSCHEDULER_H
#define HLFSCHEDULER_H

#include "leafscheduler.h"

#include <algorithm>

class HLFscheduler : public Leafscheduler {
    protected:
        void all_combinations(vector<task_id> nums,
                unsigned int n,
                unsigned int minindex,
                const Intree& t,
                const vector<unsigned int>& referencelevels,
                vector<task_id>& current,
                vector<vector<task_id>>& target) const;
    public:
        void get_initial_schedule(const Intree& t, 
                const unsigned int, 
                vector<vector<task_id>>&) const;
        virtual void get_next_tasks(const Intree& t, 
                const vector<task_id>& marked,
                vector<pair<vector<task_id>,myfloat>>& target) const;
};

#endif
