#include "hlfrandomscheduler.h"

HLFRandomScheduler::HLFRandomScheduler(){
    rng.seed(time(NULL));
}

Scheduler::schedulerchoice HLFRandomScheduler::get_next_tasks(
    const Intree& t, 
    const vector<task_id>& marked
) const {
    auto target = HLFscheduler::get_next_tasks(t, marked);
    if(target.size()==0){
        return target;
    }
    std::uniform_int_distribution<unsigned int> uni_dist(0, target.size()-1);
    auto a = uni_dist(rng);
    auto tmp = target[a];
    tmp.second=(myfloat)1;
    target.clear();
    target.push_back(tmp);
    return target;
}
