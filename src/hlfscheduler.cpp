#include "hlfscheduler.h"

void HLFscheduler::all_combinations(vector<task_id> nums, 
        unsigned int size,
        unsigned int n, 
        unsigned int minindex,
        vector<task_id>& current,
        vector<vector<task_id>>& target) const {
    if(n==0){
        target.push_back(current);
        return;
    }
    if(size - minindex <= n)
        return;
    for(unsigned int i=minindex+1; i<size; ++i){
        vector<task_id> newcombo(current);
        newcombo.push_back(nums[i]);
        all_combinations(nums,
            size, 
            n-1,
            i,
            newcombo,
            target);
    }
}

void HLFscheduler::get_initial_schedule(const Intree& t,
        const unsigned int procs,
        vector<vector<task_id>>& target) const {
    set<task_id> tasks;
    t.get_tasks(tasks);
    vector<task_id> tmp;
    for(auto it=tasks.begin(); it!=tasks.end(); ++it){
        tmp.push_back(*it);
    }
    sort(tmp.begin(), tmp.end(),
        [&t](const task_id& a, const task_id& b) -> bool {
            return t.get_level(a) > t.get_level(b);
        }
        ); 
    int maxlevel = t.get_level(tmp[0]);
    int minindex = procs-1;
    while(t.get_level(tmp[minindex])==t.get_level(tmp[minindex+1]))
        minindex++;
    int minlevel = t.get_level(tmp[minindex]);
    int maxindex=minindex;

    for(auto it=tmp.begin(); it!=tmp.end(); ++it){
        cout << "task no. " << *it << "(" << t.get_level(*it) << ")" << endl;
    }
    cout << "maxlevel " << maxlevel << endl;
    cout << "minlevel " << minlevel << endl;
    cout << "maxindex " << maxindex << endl;
    cout << "minindex " << minindex << endl;
    vector<vector<task_id>> combos;
    vector<task_id> dummy;

    all_combinations(tmp, 5, procs, 1, dummy, combos);
    for(auto cit=combos.begin(); cit!=combos.end(); cit++){
        cout << "New task combo" << endl;
        for(auto it=cit->begin(); it!=cit->end(); ++it){
            cout << "task no. " << *it << "(" << t.get_level(*it) << ")" << endl;
        }
    }

    if(minlevel == maxlevel){
        all_combinations(tmp, minindex, procs, 0, dummy, combos);
        for(auto it = combos.begin(); it!=combos.end(); ++it){
            target.push_back(*it);
        }
    }
    else{
        maxindex = minindex-1;
        while(t.get_level(tmp[maxindex]) == t.get_level(tmp[maxindex-1]))
            maxindex--;
        all_combinations(tmp, minindex, procs - maxindex - 1, maxindex, dummy, combos);
    }
    cout << "Combinations done." << endl;
    for(auto cit=combos.begin(); cit!=combos.end(); cit++){
        cout << "New task combo" << endl;
        for(auto it=cit->begin(); it!=cit->end(); ++it){
            cout << "task no. " << *it << "(" << t.get_level(*it) << ")" << endl;
        }
    }
}

void HLFscheduler::get_next_tasks(const Intree& t, 
        const vector<task_id>& marked,
        vector<pair<task_id,myfloat>>& target) const {
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
