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
    Leafscheduler::get_initial_schedule(t, p, target);
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
    }
    // if(t.is_degenerate_tree()){
    //     cout << "YES ";
    // }
    // else{
    //     cout << "NO! ";
    // }
    // cout << t << endl;
}
