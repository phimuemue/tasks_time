#include "hlfdeterministicscheduler.h"

Scheduler::schedulerchoice HLFDeterministicScheduler::get_next_tasks(
    const Intree& t, 
    const vector<task_id>& marked
) const {
    auto target = HLFscheduler::get_next_tasks(t, marked);
    if(target.size()==0){
        return target;
    }
    auto tmp = target[0];
    tmp.second = (myfloat)1;
    target.clear();
    target.push_back(tmp);
    return target;
}
