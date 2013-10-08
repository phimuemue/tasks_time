#include "specialcaseleafscheduler.h"

void SpecialCaseLeafscheduler::all_possible_combinations(
        const vector<task_id>& t,
        const unsigned int n,
        const unsigned int minindex,
        vector<vector<task_id>>& target
        ) const {
    Leafscheduler::all_possible_combinations(t, n, minindex, target);
}

void SpecialCaseLeafscheduler::get_initial_schedule(const Intree& t, 
        const unsigned int p, 
        vector<vector<task_id>>& target) const {
    if(t.is_degenerate_tree() || t.is_parallel_chain()){
        HLFDeterministicScheduler hlfds;
        hlfds.get_initial_schedule(t, p, target);
    }
    else {
        Leafscheduler::get_initial_schedule(t, p, target);
    }
#if 1 // use conjecture that as many toptask as possible shall be scheduled
    vector<task_id> leaves;
    t.get_leaves(leaves);
    sort(leaves.begin(), leaves.end(),
            [&](const task_id a, const task_id b) -> bool {
            return t.get_level(a) > t.get_level(b);
            }
        );
    unsigned int count = 0;
    int max_level = t.get_level(leaves[0]);
    for(auto it : leaves){
        if(t.get_level(it) == max_level){
            count++;
        }
        else {
            break;
        }
    }
    if(count > p){
        target.erase(remove_if(target.begin(), target.end(),
                    [&](const vector<task_id>& a) -> bool {
                        for(auto it : a){
                            if(t.get_level(it) < max_level){
                                return true;
                            }
                        }
                        return false;
                    }
                    ), target.end());
    }
#endif
}

void SpecialCaseLeafscheduler::get_next_tasks(const Intree& t, 
        const vector<task_id>& marked,
        vector<pair<task_id,myfloat>>& target) const {
    // cout << t << endl;
    // cout << t.is_degenerate_tree() << " " << t.is_parallel_chain() << endl;
    if(t.is_degenerate_tree() || t.is_parallel_chain()){
        HLFDeterministicScheduler hlfds;
        hlfds.get_next_tasks(t, marked, target);
    }
    else
    {
        Leafscheduler::get_next_tasks(t, marked, target);
#if 1 // use conjecture that as many toptask as possible shall be scheduled
        vector<task_id> leaves;
        t.get_leaves(leaves);
        sort(leaves.begin(), leaves.end(),
                [&](const task_id a, const task_id b) -> bool {
                    return t.get_level(a) > t.get_level(b);
                }
            );
        // cout << t << endl;
        // for(auto it : leaves){
        //     cout << it << " " << t.get_level(it) << endl;
        // }
        // count number of topmost tasks
        unsigned int count = 0;
        int max_level = t.get_level(leaves[0]);
        for(auto it : leaves){
            if(t.get_level(it) == max_level){
                count++;
            }
            else {
                break;
            }
        }
        // cout << count << " topmost tasks" << endl;
        // cout << marked.size() << endl;
        if(count > marked.size()){
            target.erase(remove_if(target.begin(), target.end(),
                [&](const pair<task_id, myfloat>& a) -> bool {
                    return t.get_level(a.first) < max_level;
                }
            ), target.end());
        }
        myfloat probsum = 0;
        for(const auto& it : target){
            probsum += it.second;
        }
        for(auto& it : target){
            it.second /= probsum;
        }
#endif
    }
    // if(t.is_degenerate_tree()){
    //     cout << "YES ";
    // }
    // else{
    //     cout << "NO! ";
    // }
    // cout << t << endl;
}
