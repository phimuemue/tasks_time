#include "hlfscheduler.h"

void HLFscheduler::all_combinations(vector<task_id> nums, 
        unsigned int n, 
        unsigned int minindex,
        const Intree& t,
        const vector<int>& referencelevels,
        vector<task_id>& current,
        vector<vector<task_id>>& target) const {
    if(n==0){
        for(unsigned int i=0; i<current.size(); ++i){
            if(t.get_level(current[i]) != referencelevels[i])
                return;
            if(t.get_in_degree(current[i]) > 0)
                return;
        }
        target.push_back(current);
        return;
    }
    if(nums.size() - minindex <= n)
        return;
    for(unsigned int i=minindex+1; i<nums.size(); ++i){
        vector<task_id> newcombo(current);
        newcombo.push_back(nums[i]);
        all_combinations(nums,
            n-1,
            i,
            t,
            referencelevels,
            newcombo,
            target);
    }
}

void HLFscheduler::get_initial_schedule(const Intree& t,
        const unsigned int procs,
        vector<vector<task_id>>& target) const {
    // fetch all tasks, and sort them according to their level
    set<task_id> tasks;
    t.get_tasks(tasks);
    vector<task_id> tmp;
    for(auto it=tasks.begin(); it!=tasks.end(); ++it){
        if(t.get_in_degree(*it) == 0)
            tmp.push_back(*it);
    }
    sort(tmp.begin(), tmp.end(),
        [&t](const task_id& a, const task_id& b) -> bool {
            return t.get_level(a) > t.get_level(b);
        }
        ); 
    // compute possible combinations
    vector<vector<task_id>> combos;
    vector<task_id> dummy;
    vector<int> referencelevels;
    unsigned int numtasks = tmp.size();
    unsigned int actualprocs = min(procs, numtasks);
    for(unsigned int i=0; i<actualprocs; ++i){
        referencelevels.push_back(t.get_level(tmp[i]));
    }
    all_combinations(tmp, actualprocs, -1, t, referencelevels, dummy, combos);
    for(auto it = combos.begin(); it!=combos.end(); ++it){
        target.push_back(*it);
    }
}

void HLFscheduler::get_next_tasks(const Intree& t, 
        const vector<task_id>& _marked,
        vector<pair<task_id,myfloat>>& target) const {
    vector<task_id> marked(_marked);
    marked.erase(remove_if(marked.begin(), marked.end(),
        [&t](const task_id a) -> bool {
            return !t.contains_task(a);
        }), 
        marked.end()
    );
    vector<pair<task_id, myfloat>> tmp;
    Leafscheduler::get_next_tasks(t, marked, tmp);
    int maxlevel = 0; 
    for(auto it=tmp.begin(); it!=tmp.end(); ++it){
        maxlevel = t.get_level(it->first) > maxlevel ? t.get_level(it->first) : maxlevel;
    }
    tmp.erase(remove_if(tmp.begin(), tmp.end(),
                [maxlevel, t](const pair<task_id, myfloat>& a){
                    if(a.first==NOTASK)
                        return false;
                    return t.get_level(a.first)!=maxlevel;
                }), tmp.end());
    // normalize probability values
    myfloat sum = 0;
    for(auto it=tmp.begin(); it!=tmp.end(); ++it){
        sum += it->second;
    }
    for(unsigned int i=0; i<tmp.size(); ++i){
        tmp[i].second = tmp[i].second / sum;
    }
    for(auto it=tmp.begin(); it!=tmp.end(); ++it){
        target.push_back(*it);
    }
}
