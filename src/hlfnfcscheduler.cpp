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
    vector<task_id> newmarked(marked);
    newmarked.erase(
        remove_if(newmarked.begin(), newmarked.end(),
            [&](const task_id& task) -> bool {
                return !t.contains_task(task);
            }
        )
        , newmarked.end()
    );
    HLFscheduler::get_next_tasks(t, newmarked, target);
    // we grab all chains ...
    vector<vector<task_id>> allchains;
    t.get_chains(allchains);
    vector<vector<task_id>> marked_chains(newmarked.size());
    for(unsigned int i = 0; i<newmarked.size(); ++i){
        t.get_chain(newmarked[i], marked_chains[i]);
    }
    allchains.erase(
        remove_if(
            allchains.begin(), allchains.end(),
            [&](const vector<task_id>& c) -> bool {
                for(auto task = c.begin(); task!=c.end(); ++task){
                    for(auto mc = marked_chains.begin(); mc!=marked_chains.end(); ++mc){
                        if(find(mc->begin(), mc->end(), *task) != mc->end()){
                            return true;
                        }
                    }
                }
                return false;
            }
        ), 
        allchains.end()
    );
    // ... and remove the ones that are "occupied" by marked
    return;

}
