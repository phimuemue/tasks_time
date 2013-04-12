#include "hlfscheduler.h"

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
    for(auto it=tmp.begin(); it!=tmp.end(); ++it){
        target.push_back(*it);
    }
}
