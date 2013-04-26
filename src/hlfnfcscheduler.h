#ifndef HLFNFCSCHEDULER_H
#define HLFNFCSCHEDULER_H

#include <algorithm>

#include "leafscheduler.h"
#include "hlfscheduler.h"

class HLFNFCscheduler : public HLFscheduler {
    private:
        void all_combinations(vector<task_id> nums,
                unsigned int n,
                unsigned int minindex,
                const Intree& t,
                const vector<int>& referencelevels,
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
