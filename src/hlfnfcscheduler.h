#ifndef HLFNFCSCHEDULER_H
#define HLFNFCSCHEDULER_H

#include "leafscheduler.h"
#include "hlfscheduler.h"

#include <algorithm>

class HLFNFCscheduler : public HLFscheduler {
    protected:
        void all_combinations(vector<task_id> nums,
                unsigned int n,
                unsigned int minindex,
                const Intree& t,
                const vector<unsigned int>& referencelevels,
                vector<task_id>& current,
                vector<vector<task_id>>& target) const;
        unsigned int count_free_chains(const Intree& t,
                const vector<task_id>& newmarked,
                const vector<task_id>& target_tasks) const;
    public:
        void get_initial_schedule(const Intree& t, 
                vector<vector<task_id>>&) const;
        schedulerchoice get_next_tasks (
            const Intree& t, 
            const vector<task_id>& marked
        ) const override;
};

#endif
