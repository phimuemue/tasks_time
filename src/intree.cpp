#include "intree.h"

Intree::Intree(){
}

Intree::Intree(const Intree& t){
    for(auto it = t.edges.begin(); it != t.edges.end(); ++it){
        edges.push_back(*it);
    }
}

Intree::Intree(vector<pair<Task, Task>>& edges){
    this->edges = edges;
}

int Intree::get_in_degree(const Task& t) const {
    return get_in_degree(t.get_id());
}

int Intree::get_in_degree(const task_id t) const {
    int result = 0;
    for(auto it = edges.begin(); it != edges.end(); ++it){
        if(t == it->second)
            result++;
    }
    return result;
}

void Intree::get_tasks(set<task_id>& result) const {
    for(auto it = edges.begin(); it != edges.end(); ++it){
        result.insert(it->first.get_id());
        result.insert(it->second.get_id());
    }
}

int Intree::get_level(const Task& t) const{
    return get_level(t.get_id());
}

int Intree::get_level(const task_id t) const{
    int current = t;
    int result = 0;
    while(current > 0){
        auto edge = get_edge_from(current);
        current = edge.second.get_id();
        result++;
    }
    return result;
}


void Intree::remove_task(Task& t){
    remove_task(t.get_id());
}

void Intree::remove_task(task_id t){
    if(get_in_degree(t) != 0)
        return;
    // TODO: Why does the following apparently not work?
    // remove_if(edges.begin(), edges.end(),
    //             [t](pair<Task, Task> e){
    //                 return ((e.first == t) || (e.second == t));
    //             });
    vector<pair<Task, Task>> tmp;
    for(auto it = edges.begin(); it != edges.end(); ++it){
        if(!((it->first == t) || (it->second == t))){
            tmp.push_back(*it);
        }
    }
    edges.clear();
    for(auto it = tmp.begin(); it != tmp.end(); ++it){
        edges.push_back(*it);        
    }
}

pair<Task, Task> Intree::get_edge_from(const Task& t) const {
    return get_edge_from(t.get_id());
}

pair<Task, Task> Intree::get_edge_from(const task_id t) const {
    for(auto it = edges.begin(); it != edges.end(); ++it){
        if(t == it->first){
            return *it;
        }
    }
    // TODO: remove dummy
    return pair<Task,Task>(Task(0),Task(0));
}

void Intree::get_chain(const Task& t, vector<int>& target) const {
    get_chain(t.get_id(), target);
}

void Intree::get_chain(const task_id t, vector<int>& target) const {
    int current = t;
    target.push_back(current);
    while(current > 0){
        auto edge = get_edge_from(current);
        current = edge.second.get_id();
        target.push_back(current);
    }
}

void Intree::get_chains(vector<vector<int>>& target) const {
    for(auto it = edges.begin(); it != edges.end(); ++it){
        if(get_in_degree(it->first) == 0){
            vector<int> nv;
            get_chain(it->first, nv);
            target.push_back(nv);
        }
    }
}
    
ostream& operator<<(ostream& os, const Intree& t){
    vector<vector<int>> chains;
    t.get_chains(chains);
    for(auto i1 = chains.begin(); i1 != chains.end(); ++i1){
        os << "[";
        for(auto it = i1->begin(); it != i1->end(); ++it){
            os << *it;
            if(it + 1 != i1->end())
                os << ", ";
        }
        os << "]";
        if(i1 + 1 != chains.end())
            os << " ";
    }
    return os;
}
