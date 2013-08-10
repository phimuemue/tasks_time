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

Intree::Intree(const vector<pair<task_id, task_id>>& edges){
    for(auto it=edges.begin(); it!=edges.end(); ++it){
        this->edges[it->first] = it->second;
        taskmap[it->first] = Task(it->first);
        taskmap[it->second] = Task(it->second);
    }
    taskmap[0] = Task(0);
}

Intree::Intree(const vector<pair<Task, Task>>& edges){
    for(auto it=edges.begin(); it!=edges.end(); ++it){
        this->edges[it->first.get_id()] = it->second.get_id();
        taskmap[it->first.get_id()] = it->first;
        taskmap[it->second.get_id()] = it->second;
    }
    taskmap[0] = Task(0);
}

Intree::Outtree::Outtree(task_id i, bool m) :
    id(i),
    marked(m)
{
}

Intree::Outtree::Outtree(const Outtree& ot) :
    id(ot.id),
    marked(ot.marked),
    predecessors(ot.predecessors)
{
}

Intree::Outtree::Outtree(const Intree& i, const vector<task_id>& marked) :
    id(0)
    {
    map<task_id, Outtree*> outtrees;
    outtrees[0] = this;
    for(const auto& edge : i.edges){
        outtrees[edge.first] = new Outtree(edge.first, find(marked.begin(), marked.end(), edge.first)!=marked.end());
        outtrees[edge.second]->predecessors.push_back(outtrees[edge.first]);
    }
}

Intree::Outtree::~Outtree(){
    for(Outtree* o : predecessors){
        delete o;
    }
}

string Intree::Outtree::getCompressedString() const{
    stringstream ss;
    vector<string> predecessor_strings;
    for(const auto it : predecessors){
        predecessor_strings.push_back(it->getCompressedString());
    }
    sort(predecessor_strings.begin(), predecessor_strings.end());
    ss << "[";
    for(const auto it : predecessor_strings){
        ss << it;
    }
    if(predecessor_strings.size() == 0){
        if(marked){
            ss << "0";
        }
        else{
            ss << "1";
        }
    }
    ss << "]";
    return ss.str();
}

void Intree::Outtree::canonicalize(){
    // cout << "Canonicalizing " << id << endl;
    // cout << id << " before: ";
    // for(auto& it : predecessors){
    //     cout << it->id << " ";
    // }
    // cout << endl;
    for(Outtree* neighbor : predecessors){
        neighbor->canonicalize();
    }
    sort(predecessors.begin(), predecessors.end(),
            [](const Outtree* a, const Outtree* b) -> bool {
                return *a < *b;
            }
        );
    // cout << id << " after: ";
    // for(auto& it : predecessors){
    //     cout << it->id << " ";
    // }
    // cout << endl;
}

Intree Intree::Outtree::toIntree(map<task_id, task_id>& isomorphism) const{
    vector<pair<task_id, task_id>> edges;
    queue<const Outtree*> q;
    q.push(this);
    task_id counter = 0;
    while(!q.empty()){
        const Outtree* current = q.front();
        q.pop();
        for(Outtree* neighbor : current->predecessors){
            ++counter;
            isomorphism[neighbor->id] = counter;
            neighbor->id = counter;
            // cout << "Edge " << neighbor->id << " " << current->id << endl;
            edges.push_back(pair<task_id, task_id>(neighbor->id, current->id));
            q.push(neighbor);
        }
    }
    return Intree(edges);
}

Intree Intree::canonical_intree(const Intree& _t, 
        const vector<task_id>& _preferred,
        map<task_id, task_id>& isomorphism,
        tree_id& out){
    Outtree ot(_t, _preferred);
    ot.canonicalize();
    Intree result = ot.toIntree(isomorphism);
    // cout << "Isomorphism:" << endl;
    // for(auto it : isomorphism){
    //     cout << it.first << " -> " << it.second << endl;
    // }
    // cout << ot.getCompressedString() << endl;
    result.get_raw_tree_id(out);
    return result;
}

Intree Intree::canonical_intree2(const Intree& _t, 
        const vector<task_id>& _preferred,
        map<task_id, task_id>& isomorphism,
        tree_id& out){
    Intree t(_t);
    vector<task_id> preferred(_preferred);
    map<task_id, task_id> inner_iso_rev;
    inner_iso_rev[0] = 0;
    for(auto it=t.edges.begin(); it!=t.edges.end(); ++it){
        inner_iso_rev[it->first] = it->first;
    }
    // make leaves minimal
    vector<task_id> leaves;
    t.get_leaves(leaves);
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
        for_each(preferred.begin(), preferred.end(),
                [&](task_id& a){
                    if(a==*it){
                        a = max_tid;
                    }
                }
                );
    }
    // initialize marked_count
    map<task_id, unsigned int> marked_count;
    for(auto it=preferred.begin(); it!=preferred.end(); ++it){
        marked_count[*it] = 1;
    }
    vector<vector<task_id>> tasks_by_level(t.taskmap.size());
    // store tasks grouped by level
    for(auto it=t.taskmap.begin(); it!=t.taskmap.end(); ++it){
        tasks_by_level[t.get_level(it->first)].push_back(it->first);
    }
    // traverse tasks levelwise (high to low) and comput canonical names
    map<task_id, vector<task_id>> all_predecessors;
    map<task_id, vector<unsigned short>> canonical_names;
    for(auto rit = tasks_by_level.rbegin(); rit!=tasks_by_level.rend(); ++rit){
        for(auto it=rit->begin(); it!=rit->end(); ++it){
            vector<unsigned short> canonical_name;
            vector<task_id> predecessors;
            t.get_predecessors(*it, predecessors);
            for(auto pit : predecessors){
                marked_count[*it] += marked_count[pit];
            }
            sort(predecessors.begin(), predecessors.end(),
                    [&](const task_id& a, const task_id& b) -> bool {
                    if(canonical_names[a].size() < canonical_names[b].size()) {
                        return false;
                    }
                    else if (canonical_names[a].size() > canonical_names[b].size()) {
                        return true;
                    }
                    if(canonical_names[a] == canonical_names[b]){
                        auto mc_a = marked_count[a];
                        auto mc_b = marked_count[b];
                        if(mc_a != mc_b)
                            return mc_b < mc_a;
                        auto dist_a = distance(preferred.begin(), 
                            find(preferred.begin(), preferred.end(), a));
                        auto dist_b = distance(preferred.begin(),
                            find(preferred.begin(), preferred.end(), b));
                        if (dist_a != dist_b)
                            return a < b;
                        return dist_a < dist_b;
                    }
                    return canonical_names[a] > canonical_names[b];
                    }
                );
            all_predecessors[*it] = predecessors;
            vector<vector<unsigned short>> canonical_names_predecessors;
            canonical_name.push_back(0);
            for(auto pit : predecessors){
                for(unsigned int i = 0; i<canonical_names[pit].size(); ++i){
                    vector<unsigned short>& tmp_bs =
                        canonical_names[pit];
                    unsigned short tmp = tmp_bs[i];
                    canonical_name.push_back(tmp);
                }
            }
            if(find(preferred.begin(), preferred.end(), *it) != preferred.end()){
                canonical_name.push_back(2);
            }
            else{
                canonical_name.push_back(1);
            }
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
                    auto mc_a = marked_count[a];
                    auto mc_b = marked_count[b];
                    if(mc_a != mc_b)
                        return mc_b < mc_a;
                    auto dist_a = distance(preferred.begin(), 
                        find(preferred.begin(), preferred.end(), a));
                    auto dist_b = distance(preferred.begin(),
                        find(preferred.begin(), preferred.end(), b));
                    if (dist_a != dist_b)
                        return a < b;
                    return dist_a < dist_b;
                }
                return canonical_names[a] > canonical_names[b];
                }
            );
    }
    // assign consecutive numbers to tasks
    task_id consecutive_num = 0;
    deque<task_id> q;
    q.push_back(0);
    while(q.size() > 0)
    {
        task_id _tit = q.front();
        q.pop_front();
        task_id* tit = &_tit;
        for_each(all_predecessors[*tit].begin(), all_predecessors[*tit].end(),
                [&](const task_id tid){
                q.push_back(tid);
                }
                );
        isomorphism[inner_iso_rev[*tit]] = consecutive_num;
        consecutive_num++;
    }
    vector<pair<Task, Task>> edges;
    for(auto it=_t.edges.begin(); it!=_t.edges.end(); ++it){
        edges.push_back(pair<Task,Task>(Task(isomorphism[it->first]),Task(isomorphism[it->second])));
    }
    // TODO: Expand to more than 64 bits!
    out.clear();
    for(unsigned int i=0; i<canonical_names[0].size(); ++i){
        out.push_back(canonical_names[0][i] > 0 ? 1u : 0u);
    }
    return Intree(edges);
}

unsigned int Intree::count_tasks() const{
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

bool Intree::is_chain() const {
    vector<vector<task_id>> chains;
    // TODO: this can be made more efficient
    get_chains(chains);
    return chains.size() == 1;
}

bool Intree::is_degenerate_tree() const {
    // store - for each level - the one task that may have predecessors
    map<unsigned int, task_id> cont;
    for(const auto& it : edges){
        if(cont.find(get_level(it.second)) != cont.end() && cont[get_level(it.second)] != it.second){
            return false;
        }
        cont[get_level(it.second)] = it.second;
    }
    return true;
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

unsigned int Intree::longest_chain_length() const{
    if(edges.size()==0){
        return 1;
    }
    vector<vector<task_id>> chains;
    get_chains(chains);
    unsigned int result = 0;
    for_each(chains.begin(), chains.end(),
            [&result](const vector<task_id> c){
                result = max(result, (unsigned int)c.size());
            }
            );
    return result;
}

void Intree::get_profile(vector<unsigned int>& target) const {
    assert(target.size()==0);
    for(unsigned int i=0; i<longest_chain_length(); ++i){
        target.push_back(0);
    }
    for(auto it : edges){
        target[get_level(it.first)]++;
    }
    assert(target[0]==0);
    target[0] = 1;
}

void Intree::get_raw_tree_id(tree_id& target){
    task_id max_id = (max_element(edges.begin(), edges.end())->first);
    for(unsigned int i=0; i<max_id+1u; ++i){
        target.push_back((task_id)-1);
    }
    for(const pair<task_id, task_id>& it : edges){
        target[it.first] = it.second;
    }
}
    
void Intree::get_reverse_tree(map<task_id, vector<task_id>>& rt) const{
    queue<task_id> q;
    q.push(0);
    rt[0] = vector<task_id>();
    while (q.size() > 0){
        task_id current = q.front();
        q.pop();
        for(auto it = edges.begin(); it!=edges.end(); ++it){
            if(it->second == current){
                q.push(it->first);
                rt[it->second].push_back(it->first);
                if(rt.find(it->first) == rt.end()){
                    rt[it->first] = vector<task_id>();
                }
            }
        }
    }
}

unsigned int Intree::get_max_width(task_id tid) const{
    // TODO: This method is very inefficient
    unsigned result = 0;
    map<task_id, vector<task_id>> rt;
    get_reverse_tree(rt);
    for(auto it=rt[tid].begin(); it!=rt[tid].end(); it++){
        result += get_max_width(*it);
    }
    return max(result, 1u);
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

bool Intree::operator==(const Intree& t) const {
    return edges==t.edges;
}
