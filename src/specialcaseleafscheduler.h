#ifndef SPECIALCASELEAFSCHEDULER_H
#define SPECIALCASELEAFSCHEDULER_H

#include "scheduler.h"
#include "leafscheduler.h"
#include "hlfdeterministicscheduler.h"

class SpecialCaseLeafscheduler : public Leafscheduler {
    protected:
        void all_possible_combinations(
            const vector<task_id>& t,
            const unsigned int n,
            const unsigned int minindex,
            vector<vector<task_id>>& target
        ) const;
    public:
        void get_initial_schedule(const Intree& t, 
                vector<vector<task_id>>&) const;
        void get_next_tasks(const Intree& t, 
                const vector<task_id>& marked,
                vector<pair<vector<task_id>,myfloat>>& target) const;
};

#endif
