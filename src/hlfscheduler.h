#ifndef HLFSCHEDULER_H
#define HLFSCHEDULER_H

#include "leafscheduler.h"

class HLFscheduler : public Leafscheduler {
    private:
        void all_combinations(vector<task_id> nums,
                unsigned int size,
                unsigned int n,
                unsigned int minindex,
                vector<task_id>& current,
                vector<vector<task_id>>& target) const;
    public:
        void get_initial_schedule(const Intree& t, 
                const unsigned int, 
                vector<vector<task_id>>&) const;
        void get_next_tasks(const Intree& t, 
                const vector<task_id>& marked,
                vector<pair<task_id,myfloat>>& target) const;
};

#endif
