#include "hlfnfcscheduler.h"

void HLFNFCscheduler::all_combinations(vector<task_id> nums, 
        unsigned int n, 
        unsigned int minindex,
        const Intree& t,
        const vector<int>& referencelevels,
        vector<task_id>& current,
        vector<vector<task_id>>& target) const {
    HLFscheduler::all_combinations(nums, n, minindex, t, referencelevels, current, target);
}

void HLFNFCscheduler::get_initial_schedule(const Intree& t,
        const unsigned int procs,
        vector<vector<task_id>>& target) const {
    HLFscheduler::get_initial_schedule(t, procs, target);
}

void HLFNFCscheduler::get_next_tasks(const Intree& t, 
        const vector<task_id>& marked,
        vector<pair<task_id,myfloat>>& target) const {
    HLFscheduler::get_next_tasks(t, marked, target);
    if(target.size() > 1){
        target.erase(
            remove_if(target.begin(), target.end(),
                [&](const pair<task_id, myfloat>& a) -> bool {
                    bool found = false;
                    for (auto it = marked.begin(); it!=marked.end(); ++it){
                        if (t.same_chain(a.first, *it)){
                            found = true;
                            break;
                        }
                    }
                    return found;
                }),
                target.end()
            );
    }
}
