#include "snapshot.h"

Snapshot::Snapshot(Intree& t) :
    intree(t)
{

}

Snapshot::Snapshot(Intree& t, vector<task_id> m) :
    marked(m),
    intree(t)
{

}

void Snapshot::get_successors(const Scheduler& scheduler){
    // we only want to compute the successors once
    if(successors.size()>0)
        return;
    if(intree.count_tasks()==1)
        return;
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
        scheduler.get_next_tasks(tmp, marked, raw_sucs);
        // we have to check if the scheduler even found a new task to schedule
        if(raw_sucs.size() > 0){
            for(unsigned int i=0; i<raw_sucs.size(); ++i){
                vector<task_id> newmarked(marked);
                newmarked.erase(remove_if(newmarked.begin(), newmarked.end(),
                            [it](const task_id& a){
                            return a==*it;
                            }), newmarked.end());
                // TODO: Is this needed?
                if(raw_sucs[i].first != NOTASK)
                    newmarked.push_back(raw_sucs[i].first);
                Snapshot news(tmp, newmarked);
                successors.push_back(news);
            }
        }
        else {
            vector<task_id> newmarked(marked);
            newmarked.erase(remove_if(newmarked.begin(), newmarked.end(),
                        [it](const task_id& a){
                        return a==*it;
                        }), newmarked.end());
            Snapshot news(tmp, newmarked);
            successors.push_back(news);
        }
    }
}

void Snapshot::compile_snapshot_dag(const Scheduler& scheduler){
    get_successors(scheduler);
    for(auto it=successors.begin(); it!=successors.end(); ++it){
        it->compile_snapshot_dag(scheduler);
    }
}

myfloat Snapshot::expected_runtime(){
    // TODO
    throw 0;
}

void Snapshot::print_snapshot_dag(int depth){
    for(int i=0; i<depth; ++i){
        cout << "*";
    } 
    cout << " ";
    cout << *this << endl;
    for(auto it=successors.begin(); it!=successors.end(); ++it){
        it->print_snapshot_dag(depth+1);
    }
}

ostream& operator<<(ostream& os, const Snapshot& s){
    os << "<" << s.intree << " | [";
    
    for(auto it = s.marked.begin(); it != s.marked.end(); ++it){
        os << *it;
        if (it+1!=s.marked.end()){
            os << ", ";
        }
    }
    os << "]>";
    return os;
}
