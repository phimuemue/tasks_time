#include "hlfdeterministicscheduler.h"

void HLFDeterministicScheduler::get_next_tasks(const Intree& t, 
        const vector<task_id>& marked,
        vector<pair<task_id,myfloat>>& target) const {
    HLFscheduler::get_next_tasks(t, marked, target);
    if(target.size()==0){
        return;
    }
    auto tmp = target[0];
    tmp.second = (myfloat)1;
    target.clear();
    target.push_back(tmp);
}
