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
        vector<vector<task_id>>& target) const {
    if(t.is_degenerate_tree() || t.is_parallel_chain()){
        HLFDeterministicScheduler hlfds;
        hlfds.get_initial_schedule(t, target);
    }
    else {
        Leafscheduler::get_initial_schedule(t, target);
    }
#if 1 // use conjecture that as many toptask as possible shall be scheduled
    vector<task_id> leaves = t.get_leaves();
    // TODO: Are leaves possibly already sorted?
    sort(leaves.begin(), leaves.end(),
            [&](const task_id a, const task_id b) -> bool {
            return t.get_level(a) > t.get_level(b);
            }
        );
    unsigned int count = 0;
    unsigned int max_level = t.get_level(leaves[0]);
    for(auto it : leaves){
        if(t.get_level(it) == max_level){
            count++;
        }
        else {
            break;
        }
    }
    // more topmost tasks than processors -> only consider topmost-combinations
    if(count >= processorcount){
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
    // less topmost tasks than processors -> only consider those schedules with all topmost tasks
    else {
        target.erase(remove_if(target.begin(), target.end(),
                    [&](const vector<task_id>& a) -> bool {
                        unsigned int schedtopcount = 0;
                        for(auto it : a){
                            if(t.get_level(it) == max_level){
                                schedtopcount++;
                            }
                        }
                        return schedtopcount != count;
                    }
                    ), target.end());
    }
#endif
}

void SpecialCaseLeafscheduler::get_next_tasks(const Intree& t, 
//#define CHECK_TARGET_VALIDITY
        const vector<task_id>& marked,
        vector<pair<vector<task_id>,myfloat>>& target) const {
    // cout << t << endl;
    // cout << t.is_degenerate_tree() << " " << t.is_parallel_chain() << endl;
    if(t.is_degenerate_tree() || t.is_parallel_chain()){
        HLFDeterministicScheduler hlfds;
        hlfds.get_next_tasks(t, marked, target);
    }
    else
    {
        Leafscheduler::get_next_tasks(t, marked, target);
#ifdef CHECK_TARGET_VALIDITY
        bool more_than_zero_in_target = target.size() > 0;
#endif
#if 1 // use conjecture that as many toptask as possible shall be scheduled
        vector<task_id> leaves = t.get_leaves();
        // TODO: are leaves already sorted?
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
        unsigned int max_level = t.get_level(leaves[0]);
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
        // more topmost than marked -> only topmost-combinations
        if(count >= marked.size()){
            target.erase(remove_if(target.begin(), target.end(),
                [&](const pair<vector<task_id>, myfloat>& a) -> bool {
                    return t.get_level(a.first[0]) < max_level;
                }
            ), target.end());
        }
        // less topmost than marked -> all topmost must be scheduled
        else{
            target.erase(remove_if(target.begin(), target.end(),
                [&](const pair<vector<task_id>, myfloat>& a) -> bool {
                    unsigned int newcount = 0;
                    for (auto tsk : marked){
                        if(t.get_level(tsk) == max_level){
                            newcount++;
                        }
                    }
                    if(t.get_level(a.first[0]) == max_level){
                        newcount++;
                    }
                    return newcount < count;
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
#ifdef CHECK_TARGET_VALIDITY
        if(more_than_zero_in_target){
            if(target.size() == 0){
                cout << t << endl;
            }
            assert(target.size() > 0);
        }
#endif
#endif
    }
    // if(t.is_degenerate_tree()){
    //     cout << "YES ";
    // }
    // else{
    //     cout << "NO! ";
    // }
    // cout << t << endl;
#undef CHECK_TARGET_VALIDITY
}
