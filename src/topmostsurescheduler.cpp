#include "topmostsurescheduler.h"

void TopMostSureScheduler::get_initial_schedule(const Intree& t, 
        vector<vector<task_id>>& target) const {
    vector<task_id> const leaves = t.get_leaves();
    vector<vector<task_id>> combis;
    all_possible_combinations(leaves, processorcount, 0, target);
}

void TopMostSureScheduler::get_next_tasks(const Intree& t, 
        const vector<task_id>& _marked,
        vector<pair<vector<task_id>,myfloat>>& target) const {
    vector<task_id> marked(_marked);
    marked.erase(remove_if(marked.begin(), marked.end(),
        [&t](const task_id a) -> bool {
            return !t.contains_task(a);
        }), 
        marked.end()
    );
#if 0
    auto tasks = t.get_tasks();
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
    auto tasks = t.get_leaves();
    for(auto it = tasks.begin(); it != tasks.end();){
        if(find(marked.begin(), marked.end(), *it)!=marked.end())
            it = tasks.erase(it);
        else
            ++it;
    }
#endif
#if 0
    vector<pair<vector<task_id>,myfloat>> tasks_probs;
    for(auto const it : tasks){
        tasks_probs.push_back(pair<vector<task_id>,myfloat>(
            vector<task_id>{it},
            ((myfloat)1)/(myfloat)tasks.size()
            )
        );
    }
    assert(tasks_probs.size()==tasks.size());
    for(unsigned int i=0; i<tasks_probs.size(); ++i){
        target.push_back(pair<vector<task_id>,myfloat>(tasks_probs[i]));
    }
#else
    for(auto const it : tasks){
        target.push_back(make_pair(
            vector<task_id>{it},
            ((myfloat)1)/(myfloat)tasks.size()
        ) );
    }
#endif
}
