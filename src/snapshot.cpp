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
    // TODO: implement class for probability computations
    vector<myfloat> finish_probs;
    for(auto it = marked.begin(); it!=marked.end(); ++it){
        finish_probs.push_back(((myfloat)1)/(myfloat)marked.size());
    }
    assert(finish_probs.size()==marked.size());
    // then, for each finished threads, compute all possible successors
    for(auto it = marked.begin(); it!=marked.end(); ++it){
        cout << "Computing successors if " << *it
        << " is finished." << endl;
        Intree tmp(intree);
        tmp.remove_task(*it);
        vector<pair<task_id,myfloat>> raw_sucs;
        scheduler.get_next_tasks(tmp, marked, raw_sucs);
        // we have to check if the scheduler even found a new task to schedule
        if(raw_sucs.size() > 0){
            cout << "Found >= 1 task to schedule." << endl;
            for(auto rsit=raw_sucs.begin(); rsit!=raw_sucs.end(); ++rsit){
                cout << (*rsit).first << ", " << (*rsit).second << endl;
            }
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
            cout << "No other task to schedule found." << endl;
            vector<task_id> newmarked(marked);
            newmarked.erase(remove_if(newmarked.begin(), newmarked.end(),
                        [it](const task_id& a){
                        return a==*it;
                        }), newmarked.end());
            Snapshot news(tmp, newmarked);
            successors.push_back(news);
        }
    }
    cout << "Done computing successors" << endl;
    for(auto it=successors.begin(); it!=successors.end(); ++it){
        cout << *it << endl;
    }
}

void Snapshot::compile_snapshot_dag(){

}

myfloat Snapshot::expected_runtime(){
    // TODO
    throw 0;
}

ostream& operator<<(ostream& os, const Snapshot& s){
    os << "Snapshot: " << s.intree << " | [";
    
    for(auto it = s.marked.begin(); it != s.marked.end(); ++it){
        os << *it;
        if (it+1!=s.marked.end()){
            os << ", ";
        }
    }
    os << "]";
    return os;
}
