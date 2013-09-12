#include "topmostsurescheduler.h"

void TopMostSureScheduler::get_initial_schedule(const Intree& t, 
        const unsigned int procs, 
        vector<vector<task_id>>& target) const {
    vector<task_id> leaves;
    t.get_leaves(leaves);
    vector<vector<task_id>> combis;
    all_possible_combinations(leaves, procs, 0, target);
}

void TopMostSureScheduler::get_next_tasks(const Intree& t, 
        const vector<task_id>& _marked,
        vector<pair<task_id,myfloat>>& target) const {
    vector<task_id> marked(_marked);
    marked.erase(remove_if(marked.begin(), marked.end(),
        [&t](const task_id a) -> bool {
            return !t.contains_task(a);
        }), 
        marked.end()
    );
    set<task_id> tasks;
#if 0
    t.get_tasks(tasks);
    for(auto it = tasks.begin(); it != tasks.end();){
        if(find(marked.begin(), marked.end(), *it)!=marked.end() ||
           t.get_in_degree(*it) > 0
          )    
            tasks.erase(it++);
        else
            ++it;
    }
#else
    // just to see if another method would behave the same
    t.get_leaves(tasks);
    for(auto it = tasks.begin(); it != tasks.end();){
        if(find(marked.begin(), marked.end(), *it)!=marked.end())
            tasks.erase(it++);
        else
            ++it;
    }
#endif
#if 0
    vector<pair<task_id,myfloat>> tasks_probs;
    for(auto it = tasks.begin(); it != tasks.end(); ++it){
        tasks_probs.push_back(pair<task_id,myfloat>(
            *it,
            ((myfloat)1)/(myfloat)tasks.size()
            )
        );
    }
    // TODO: more elegant, possibly directly
    assert(tasks_probs.size()==tasks.size());
    for(unsigned int i=0; i<tasks_probs.size(); ++i){
        target.push_back(pair<task_id,myfloat>(tasks_probs[i]));
    }
#else
    vector<pair<task_id,myfloat>> tasks_probs;
    for(auto it = tasks.begin(); it != tasks.end(); ++it){
        target.push_back(pair<task_id,myfloat>(
            *it,
            ((myfloat)1)/(myfloat)tasks.size()
            )
        );
    }
#endif
}
