#include "leafscheduler.h"

void Leafscheduler::get_next_tasks(const Intree& t, 
        const vector<task_id>& marked,
        vector<pair<task_id,myfloat>>& target) const {
    cout << "Leafscheduler at work..." << endl;
    cout << t << endl;
    set<task_id> tasks;
    t.get_tasks(tasks);
    for(auto it = tasks.begin(); it != tasks.end();){
        if(find(marked.begin(), marked.end(), *it)!=marked.end() ||
           t.get_in_degree(*it) > 0
          )    
            tasks.erase(it++);
        else
            ++it;
    }
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
}
