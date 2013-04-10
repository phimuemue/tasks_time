#include "intree.h"

Intree::Intree(vector<pair<Task, Task>>& edges){
    this->edges = edges;
}


int Intree::get_in_degree(const Task& t) const {
    get_in_degree(t.get_id());
}

int Intree::get_in_degree(const int t) const {
    int result = 0;
    for(auto it = edges.begin(); it != edges.end(); ++it){
        if(t == it->second.get_id())
            result++;
    }
    return result;
}

set<Task> Intree::get_tasks(){
    set<Task> result;
    for(auto it = edges.begin(); it != edges.end(); ++it){
        result.insert(it->first);
        result.insert(it->second);
    }
    return result;
}

void Intree::remove_task(Task& t){
    remove_task(t.get_id());
}

void Intree::remove_task(int t){
    remove_if(edges.begin(), edges.end(),
                [t](pair<Task, Task>& e){
                    return (e.first.get_id() == t || e.second.get_id() == t);
                });
}

pair<Task, Task> Intree::get_edge_from(const Task& t) const {
    return get_edge_from(t.get_id());
}

pair<Task, Task> Intree::get_edge_from(const int t) const {
    for(auto it = edges.begin(); it != edges.end(); ++it){
        if(t == it->first){
            return *it;
        }
    }
}

void Intree::get_chain(const Task& t, vector<int>& target) const {
    get_chain(t.get_id(), target);
}

void Intree::get_chain(const int t, vector<int>& target) const {
    int current = t;
    target.push_back(current);
    while(current > 0){
        auto edge = get_edge_from(current);
        target.push_back(edge.second.get_id());
    }
    target.push_back(0);
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
        for(auto it = i1->begin(); it != i1->end(); ++it){
            os << *it;
        }
    }
    return os;
}
