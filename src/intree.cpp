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

Intree::Intree(const vector<pair<Task, Task>>& edges){
    for(auto it=edges.begin(); it!=edges.end(); ++it){
        this->edges[it->first.get_id()] = it->second.get_id();
        taskmap[it->first.get_id()] = it->first;
        taskmap[it->second.get_id()] = it->second;
    }
    taskmap[0] = Task(0);
}

Intree Intree::canonical_intree(const Intree& _t, 
        const vector<task_id>& preferred,
        map<task_id, task_id>& isomorphism,
        tree_id& out){
    Intree t(_t);
    map<task_id, task_id> inner_iso_rev;
    inner_iso_rev[0] = 0;
    for(auto it=t.edges.begin(); it!=t.edges.end(); ++it){
        inner_iso_rev[it->first] = it->first;
    }
    // make leafs minimal
    vector<task_id> leaves;
    t.get_leaves(leaves);
    // cout << "Tree: " << t << endl;
    sort(leaves.begin(), leaves.end(),
        [&](const task_id a, const task_id b) -> bool {
            if(t.get_edge_from(a).second == t.get_edge_from(b).second)
                return a < b;
            return t.get_edge_from(a).second < t.get_edge_from(b).second;
        }
    );
    task_id max_tid = 0;
    for(auto it=t.edges.begin(); it!=t.edges.end(); ++it){
        max_tid = max(max_tid, it->first);
    }
    max_tid++;
    for(auto it=leaves.begin(); it!=leaves.end(); ++it){
        // cout << "Leaf: " << *it << ", ";
        ++max_tid;
        t.rename_leaf(*it, max_tid);
        inner_iso_rev[max_tid] = *it;
    }
    // cout << "MAX: " << max_tid;
    // cout << endl;
    // cout << "Inner iso: " << endl;
    // for(auto it=inner_iso_rev.begin(); it!=inner_iso_rev.end(); ++it){
    //     cout << it->first << " -> " << it->second << endl;
    // }
    vector<vector<task_id>> tasks_by_level(t.taskmap.size());
    // store tasks grouped by level
    for(auto it=t.taskmap.begin(); it!=t.taskmap.end(); ++it){
        tasks_by_level[t.get_level(it->first)].push_back(it->first);
    }
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
                    if(canonical_names[a] == canonical_names[b]){
                        auto dist_a = distance(preferred.begin(), 
                            find(preferred.begin(), preferred.end(), a));
                        auto dist_b = distance(preferred.begin(),
                            find(preferred.begin(), preferred.end(), b));
                        if (dist_a == dist_b)
                            return a < b;
                        return dist_a < dist_b;
                    }
                    return canonical_names[a] > canonical_names[b];
                    }
                );
            vector<boost::dynamic_bitset<unsigned short>> canonical_names_predecessors;
            bool tmp = false;
            canonical_name.push_back(tmp);
            for (auto pit=predecessors.begin(); pit!=predecessors.end(); ++pit){
                for(unsigned int i = 0; i<canonical_names[*pit].size(); ++i){
                    boost::dynamic_bitset<unsigned short>& tmp_bs = canonical_names[*pit];
                    bool tmp = tmp_bs.test(i);
                    canonical_name.push_back(tmp);
                }
            }
            tmp = true;
            canonical_name.push_back(tmp);
            canonical_names[*it] = canonical_name;
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
                if(canonical_names[a] == canonical_names[b]){
                    auto dist_a = distance(preferred.begin(), 
                        find(preferred.begin(), preferred.end(), a));
                    auto dist_b = distance(preferred.begin(),
                        find(preferred.begin(), preferred.end(), b));
                    if (dist_a == dist_b)
                        return a < b;
                    return dist_a < dist_b;
                }
                return canonical_names[a] > canonical_names[b];
                }
            );
    }
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
                if(ca==cb){
                    auto dist_a = distance(preferred.begin(),
                        find(preferred.begin(), preferred.end(), a));
                    auto dist_b = distance(preferred.begin(),
                        find(preferred.begin(), preferred.end(), b));
                    if (dist_a == dist_b)
                        return a < b;
                    return dist_a < dist_b;
                }
                return ca > cb;
            }
            return canonical_names[a] > canonical_names[b];
            }
        );
    }
    // assign consecutive numbers to tasks
    task_id consecutive_num = 0;
    for(auto it = tasks_by_level.begin(); it!=tasks_by_level.end(); ++it){
        for(auto tit = it->begin(); tit!=it->end(); ++tit){
            //isomorphism[*tit] = consecutive_num;
            isomorphism[inner_iso_rev[*tit]] = consecutive_num;
            consecutive_num++;
        }
    }
    // cout << "Isomorphism in canonical_intree:" << endl;
    // for(auto it=isomorphism.begin(); it!=isomorphism.end(); ++it){
    //     cout << it->first << " -> " << it->second << endl;
    // }
    vector<pair<Task, Task>> edges;
    for(auto it=_t.edges.begin(); it!=_t.edges.end(); ++it){
        edges.push_back(pair<Task,Task>(Task(isomorphism[it->first]),Task(isomorphism[it->second])));
    }
    // TODO: Expand to more than 64 bits!
    out = 0;
    for(unsigned int i=0; i<canonical_names[0].size(); ++i){
        //out = canonical_names[0].to_ulong();
        if(i>8*sizeof(tree_id)){
            cout << "More bits than can be stored in tree_id." << endl;
            throw 1;
        }
        out <<= 1;
        out = out | (canonical_names[0].test(i) ? 1ul : 0ul);
    }
    // cout << "Original: " << _t << endl;
    // cout << "iso: " << endl;
    // for(auto it=isomorphism.begin(); it!=isomorphism.end(); ++it){
    //     cout << it->first << " -> " << it->second << endl;
    // }
    // cout << "Result:   " << Intree(edges) << endl;
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

bool Intree::contains_task(task_id tid) const{
    return taskmap.find(tid) != taskmap.end();
}

const Task& Intree::get_task_by_id(const task_id tid) const {
    if(taskmap.find(tid) == taskmap.end()){
        cout << "Attempted to get_task_by_id of non-existent task." << endl;
        throw 1;
    }
    return taskmap.find(tid)->second;
}

void Intree::get_tasks(set<task_id>& result) const {
    for(auto it=taskmap.begin(); it!=taskmap.end(); ++it){
        result.insert(it->first);
    }
}

void Intree::rename_leaf(task_id original, task_id now){
    assert(!contains_task(now));
    assert(taskmap.find(original)!=taskmap.end());
    // stuff in taskmap
    auto tmp = taskmap[original];
    taskmap.erase(taskmap.find(original));
    taskmap[now] = tmp;
    // stuff in edges
    auto tmp2 = edges[original];
    edges.erase(edges.find(original));
    edges[now] = tmp2;
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

void Intree::get_leaves(vector<task_id>& target) const{
    for(auto it=edges.begin(); it!=edges.end(); ++it){
        if (get_in_degree(it->first) == 0){
            target.push_back(it->first);
        }
    }
}

bool Intree::is_leaf(const task_id t) const{
    return get_in_degree(t)==0;
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
    if(get_in_degree(t) != 0){
        cout << "Attempted to remove task with predecessor." << endl;
        cout << *this << " - " << t << endl;
        throw 1;
    }
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
    cout << "Attempted to get edge from non-existent task (" << t << " from " << *this << ")." << endl;
    throw 0;
}

bool Intree::is_chain(){
    vector<vector<task_id>> chains;
    // TODO: this can be made more efficient
    get_chains(chains);
    return chains.size() == 1;
}

bool Intree::same_chain(const task_id t1, const task_id t2) const {
    vector<task_id> ch1;
    vector<task_id> ch2;
    get_chain(t1, ch1);
    get_chain(t2, ch2);
    return ch1 == ch2;
}

unsigned int Intree::count_free_chains(vector<task_id>& target) const{
    unsigned int result = 0;
    vector<task_id> leaves;
    get_leaves(leaves);
    for(auto it=leaves.begin(); it!=leaves.end(); ++it){
        if(get_in_degree(*it)==0){
            target.push_back(*it);
            result++;
        }
    }
    return result;
}

void Intree::get_chain(const Task& t, vector<task_id>& target) const {
    get_chain(t.get_id(), target);
}

void Intree::get_chain(const task_id t, vector<task_id>& target) const {
    int current = t;
    target.push_back(current);
    while(current > 0){
        auto edge = get_edge_from(current);
        current = edge.second.get_id();
        target.push_back(current);
    }
}

void Intree::get_chains(vector<vector<task_id>>& target) const {
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
