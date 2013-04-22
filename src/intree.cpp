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

Intree Intree::canonical_intree(const Intree& t){
    cout << "Computing canonical tree of " << t << "" << endl;
    vector<vector<task_id>> tasks_by_level(t.taskmap.size());
    // store tasks grouped by level
    for(auto it=t.taskmap.begin(); it!=t.taskmap.end(); ++it){
        tasks_by_level[t.get_level(it->first)].push_back(it->first);
    }
    cout << "Traversing tasks levelwise and computing canonical names." << endl;
    // traverse tasks levelwise (high to low) and comput canonical names
    map<task_id, boost::dynamic_bitset<unsigned short>> canonical_names;
    for(auto rit = tasks_by_level.rbegin(); rit!=tasks_by_level.rend(); ++rit){
        for(auto it=rit->begin(); it!=rit->end(); ++it){
            boost::dynamic_bitset<unsigned short> canonical_name;
            vector<task_id> predecessors;
            t.get_predecessors(*it, predecessors);
            sort(predecessors.begin(), predecessors.end(),
                    [&](const task_id& a, const task_id& b) -> bool {
                    if(canonical_names[a].size() < canonical_names[b].size()) {
                        return false;
                    }
                    else if (canonical_names[a].size() > canonical_names[b].size()) {
                        return true;
                    }
                    return canonical_names[a] > canonical_names[b];
                    }
                );
            vector<boost::dynamic_bitset<unsigned short>> canonical_names_predecessors;
            bool tmp = true;
            canonical_name.push_back(tmp);
            for (auto pit=predecessors.begin(); pit!=predecessors.end(); ++pit){
                for(unsigned int i = 0; i<canonical_names[*pit].size(); ++i){
                    boost::dynamic_bitset<unsigned short>& tmp_bs = canonical_names[*pit];
                    bool tmp = tmp_bs.test(i);
                    canonical_name.push_back(tmp);
                }
            }
            tmp = false;
            canonical_name.push_back(tmp);
            cout << "Setting canonical names for " << *it << " to " << canonical_name << endl;
            canonical_names[*it] = canonical_name;
            cout << "Canonical names complete: " << endl;
            for(auto it=canonical_names.begin(); it!=canonical_names.end(); ++it){
                cout << it->first << ": " << it->second << endl;
            }
        }
        // sort tasks according to their canonical name
        sort(rit->begin(), rit->end(),
                [&](const task_id& a, const task_id& b) -> bool {
                if(canonical_names[a].size() < canonical_names[b].size()) {
                    return false;
                }
                else if (canonical_names[a].size() > canonical_names[b].size()) {
                    return true;
                }
                return canonical_names[a] > canonical_names[b];
                }
            );
        cout << "Level: " << endl;
        for(auto it=rit->begin(); it!=rit->end(); ++it){
            cout << " " << *it << ": " << canonical_names[*it] << endl;
        }
    }
    cout << "Sorting tasks." << endl;
    //cout << "Bitsets per level" << endl;
    for(auto rit = tasks_by_level.rbegin(); rit!=tasks_by_level.rend(); ++rit){
        sort(rit->begin(), rit->end(),
            [&](const task_id& a, const task_id& b) -> bool {
            if(canonical_names[a].size() < canonical_names[b].size()) {
                return false;
            }
            else if (canonical_names[a].size() > canonical_names[b].size()) {
                return true;
            }
            else if (canonical_names[a] == canonical_names[b]){
                auto ca = canonical_names[t.get_edge_from(a).second.get_id()];
                auto cb = canonical_names[t.get_edge_from(b).second.get_id()];
                if (ca.size() < cb.size())
                    return false;
                else if(ca.size() > cb.size())
                    return true;
                return ca > cb;
            }
            return canonical_names[a] > canonical_names[b];
            }
        );
    }
    //cout << "End of bitsets" << endl;
    cout << "Assigning consecutive numbers." << endl;
    // assign consecutive numbers to tasks
    task_id consecutive_num = 0;
    map<task_id,task_id> isomorphism;
    for(auto it = tasks_by_level.begin(); it!=tasks_by_level.end(); ++it){
        for(auto tit = it->begin(); tit!=it->end(); ++tit){
            cout << "Assigning for " << *tit << " (namely " << consecutive_num << ")" << endl;
            isomorphism[*tit] = consecutive_num;
            consecutive_num++;
        }
    }
    // cout << "DEBUG OUTPUT:" << endl;
    // // debug output
    // for(auto it=isomorphism.begin(); it!=isomorphism.end(); ++it){
    //     cout << it->first << ", " << it->second << endl;
    // }
    // cout << "End of DEBUT OUTPUT" << endl;
    vector<pair<Task, Task>> edges;
    for(auto it=t.edges.begin(); it!=t.edges.end(); ++it){
        edges.push_back(pair<Task,Task>(Task(isomorphism[it->first]),Task(isomorphism[it->second])));
    }
    return Intree(edges);
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

void Intree::get_predecessors(const Task& t, vector<task_id>& target) const{
    get_predecessors(t.get_id(), target);
}

void Intree::get_predecessors(const task_id t, vector<task_id>& target) const{
    for(auto it=edges.begin(); it!=edges.end(); ++it){
        if(it->second==t){
            target.push_back(it->first);
        }
    }
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
