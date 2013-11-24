#include "hlfnfcscheduler.h"

void HLFNFCscheduler::all_combinations(vector<task_id> nums, 
        unsigned int n, 
        unsigned int minindex,
        const Intree& t,
        const vector<unsigned int>& referencelevels,
        vector<task_id>& current,
        vector<vector<task_id>>& target) const {
    HLFscheduler::all_combinations(nums, n, minindex, t, referencelevels, current, target);
}

unsigned int HLFNFCscheduler::count_free_chains(const Intree& t, const vector<task_id>& newmarked, const task_id& target_task) const {
        vector<vector<task_id>> allchains;
        t.get_chains(allchains);
        vector<vector<task_id>> marked_chains(newmarked.size());
        for(unsigned int i = 0; i<newmarked.size(); ++i){
            t.get_chain(newmarked[i], marked_chains[i]);
        }
        marked_chains.push_back(vector<task_id>());
        t.get_chain(target_task, marked_chains.back());
        allchains.erase(
            remove_if(
                allchains.begin(), allchains.end(),
                [&](const vector<task_id>& c) -> bool {
                    for(auto task = c.begin(); task!=c.end(); ++task){
                        for(auto mc = marked_chains.begin(); mc!=marked_chains.end(); ++mc){
                            if(find(mc->begin(), mc->end(), *task) != mc->end() && *task!=0){
                                return true;
                            }
                        }
                    }
                    return false;
                }
            ), 
            allchains.end()
        );
        return allchains.size();
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
    vector<unsigned int> num_free_chains;
    for(auto target_task=target.begin(); target_task!=target.end(); ++target_task){
        num_free_chains.push_back(count_free_chains(t, newmarked, target_task->first));
    }
    unsigned int max_free_chains = num_free_chains.size() == 0 ? 0 :
        *max_element(num_free_chains.begin(), num_free_chains.end());
    bool shall_i_remove = false;
    for(unsigned int i = 0; i<num_free_chains.size(); ++i){
        if(num_free_chains[i]!=max_free_chains){
            shall_i_remove = true;
        }
    }
    if(shall_i_remove){
        for(int i = num_free_chains.size() - 1; i>=0; --i){
            if(num_free_chains[i] == max_free_chains){
                target.erase(target.begin() + i);
            }
        }
    }
    // normalize probabilities
    myfloat sum = 0;
    for(auto it=target.begin(); it!=target.end(); ++it){
        sum += it->second;
    }
    for(unsigned int i=0; i<target.size(); ++i){
        target[i].second = target[i].second / sum;
    }
}
