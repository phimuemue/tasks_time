#include "snapshot.h"

Scheduler Snapshot::scheduler;

Snapshot::Snapshot(Intree& t) :
    intree(t)
{

}

Snapshot::Snapshot(Intree& t, vector<task_id> m) :
    marked(m),
    intree(t)
{

}

void Snapshot::get_successors(){
    // TODO: implement class for probability computations
    vector<myfloat> finish_probs;
    for(auto it = marked.begin(); it!=marked.end(); ++it){
        finish_probs.push_back(((myfloat)1)/(myfloat)marked.size());
    }
    assert(finish_probs.size()==marked.size());
    // then, for each finished threads, compute all possible successors
    for(auto it = marked.begin(); it!=marked.end(); ++it){
        Intree tmp(intree);
        tmp.remove_task(*it);
        vector<pair<task_id,myfloat>> raw_sucs;
        Snapshot::scheduler.get_next_tasks(intree, marked, raw_sucs);
        for(unsigned int i=0; i<raw_sucs.size(); ++i){
            vector<task_id> newmarked(marked);
            for (unsigned int j=0; j<newmarked.size(); ++j){
                if(newmarked[j] == *it){
                    newmarked[j] = raw_sucs[i].first;
                }
            }
            Snapshot news(tmp, newmarked);
            successors.push_back(news);
        }
    }
}

void Snapshot::compile_snapshot_dag(){

}

myfloat Snapshot::expected_runtime(){
    // TODO
    throw 0;
}

ostream& operator<<(ostream& os, const Snapshot& s){
    os << "Snapshot: " << s.intree << " | ";
    for(auto it = s.marked.begin(); it != s.marked.end(); ++it){
        os << *it;
    }
    return os;
}
