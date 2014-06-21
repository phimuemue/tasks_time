#include "leafscheduler.h"

void Leafscheduler::all_possible_combinations(
        const vector<task_id>& t,
        const unsigned int n,
        const unsigned int minindex,
        vector<vector<task_id>>& target
        ) const {
    // common special cases
    if (n==2) {
        if(t.size()==1){
            vector<task_id> tmp;
            tmp.push_back(t[0]);
            target.push_back(tmp);
            return;
        }
        for(unsigned int i1 = minindex; i1 < t.size(); ++i1){
            for(unsigned int i2 = i1+1; i2 < t.size(); ++i2){
                vector<task_id> tmp;
                tmp.push_back(t[i1]);
                tmp.push_back(t[i2]);
                target.push_back(tmp);
            }
        }
        return;
    }
    if (n==3) {
        if(t.size()==1){
            vector<task_id> tmp;
            tmp.push_back(t[0]);
            target.push_back(tmp);
            return;
        }
        if(t.size()==2){
            vector<task_id> tmp;
            tmp.push_back(t[0]);
            tmp.push_back(t[1]);
            target.push_back(tmp);
            return;
        }
        for(unsigned int i1 = minindex; i1 < t.size(); ++i1){
            for(unsigned int i2 = i1+1; i2 < t.size(); ++i2){
                for(unsigned int i3 = i2+1; i3 < t.size(); ++i3){
                    vector<task_id> tmp;
                    tmp.push_back(t[i1]);
                    tmp.push_back(t[i2]);
                    tmp.push_back(t[i3]);
                    target.push_back(tmp);
                }
            }
        }
        return;
    }
    if(n==0){
        target.push_back(vector<task_id>());
        return;
    }
    if(n>=t.size()){
        target.push_back(t);
        return;
    }
    if(n>t.size()-minindex){
        return;
    }
    for(unsigned int i=minindex; i<t.size(); ++i){
        vector<vector<task_id>> sub_combis_with;
        vector<vector<task_id>> sub_combis_without;
        all_possible_combinations(
            t, n-1, i+1, sub_combis_with
        );
        all_possible_combinations(
            t, n, i+1, sub_combis_without
        );
        for(auto const& it : sub_combis_with){
            vector<task_id> newcombo;
            newcombo.push_back(t[minindex]);
            newcombo.insert(newcombo.end(), it.begin(), it.end());
            assert(newcombo.size()==n);
            target.push_back(newcombo);
        }
        for(auto const& it : sub_combis_without){
            assert(it.size()==n);
            target.push_back(it);
        }
    }
}

void Leafscheduler::get_initial_schedule(const Intree& t, 
        vector<vector<task_id>>& target) const {
    vector<task_id> const leaves = t.get_leaves();
    vector<vector<task_id>> combis;
    all_possible_combinations(leaves, processorcount, 0, target);
}

Scheduler::schedulerchoice Leafscheduler::get_next_tasks(
    const Intree& t, 
    const vector<task_id>& marked
) const {
    schedulerchoice target;
    vector<task_id> const leaves = t.get_leaves();
    unsigned int count = 0;
    for(auto it : leaves) {
        if(find(marked.begin(), marked.end(), it)==marked.end()) {
            count++;
            target.push_back(
                make_pair(
                    vector<task_id>{it},
                    0
                )
            );
        }
    }
    for (auto& it : target) {
        it.second = myfloat(1)/myfloat(count);
    }
    return target;
}


