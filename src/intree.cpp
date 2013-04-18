#include "intree.h"

Intree::Intree(){
}

Intree::Intree(const Intree& t){
    for(auto it = t.taskmap.begin(); it!=t.taskmap.end(); ++it){
        taskmap[it->first] = it->second;
    }
    for(auto it = t.edges.begin(); it != t.edges.end(); ++it){
        edges[it->first] = it->second;
    }
}

Intree::Intree(vector<pair<Task, Task>>& edges){
    for(auto it=edges.begin(); it!=edges.end(); ++it){
        this->edges[it->first.get_id()] = it->second.get_id();
        taskmap[it->first.get_id()] = it->first;
        taskmap[it->second.get_id()] = it->second;
    }
}

int Intree::count_tasks() const{
    return edges.size() + 1;
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

const Task& Intree::get_task_by_id(const task_id tid) const {
    return taskmap.find(tid)->second;
}

void Intree::get_tasks(set<task_id>& result) const {
    for(auto it=taskmap.begin(); it!=taskmap.end(); ++it){
        result.insert(it->first);
    }
}

Distribution Intree::get_task_distribution(const task_id t) const {
    return taskmap.find(t)->second.get_distribution();
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
    // only tasks with no predecessor can be removed
    if(get_in_degree(t) != 0)
        throw 1;
    // remove from taskmap
    auto todel = taskmap.find(t);
    taskmap.erase(todel);
    vector<pair<task_id, task_id>> tmp;
    for(auto it = edges.begin(); it != edges.end(); ++it){
        if(!((it->first == t) || (it->second == t))){
            tmp.push_back(*it);
        }
    }
    edges.clear();
    for(auto it = tmp.begin(); it != tmp.end(); ++it){
        edges[it->first] = it->second;
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
    throw 0;
}

bool Intree::is_chain(){
    vector<vector<task_id>> chains;
    // TODO: this can be made more efficient
    get_chains(chains);
    return chains.size() == 1;
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
    if(t.count_tasks()==1){
        os << "[0]";
        return os;
    }
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
