#include "intree.h"

Intree::Intree(){
}

Intree::Intree(const Intree& t) :
    edges(t.edges)
#if USE_TASKMAP
    ,
    taskmap(t.taskmap)
#endif
{
}

Intree::Intree(const vector<pair<task_id, task_id>>& edges):
    edges(edges.size() + 1, NOTASK)
{
    for(auto it : edges){
        this->edges[it.first] = it.second;
#if USE_TASKMAP
        taskmap[it.first] = Task(it.first);
        taskmap[it.second] = Task(it.second);
#endif
    }
#if USE_TASKMAP
    taskmap[0] = Task(0);
#endif
}

Intree::Intree(const vector<pair<Task, Task>>& edges) :
    edges(edges.size() + 1, NOTASK)
{
    for(auto it : edges){
        this->edges[it.first.get_id()] = it.second.get_id();
#if USE_TASKMAP
        taskmap[it.first.get_id()] = it.first;
        taskmap[it.second.get_id()] = it.second;
#endif
    }
#if USE_TASKMAP
    taskmap[0] = Task(0);
#endif
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
    for(task_id edge = 1; edge < i.edges.size(); ++edge){
        if(i.edges[edge] != NOTASK){
            outtrees[edge] = new Outtree(edge, find(marked.begin(), marked.end(), edge)!=marked.end());
            outtrees[i.edges[edge]]->predecessors.push_back(outtrees[edge]);
        }
    }
}

Intree::Outtree::~Outtree(){
    for(Outtree* o : predecessors){
        delete o;
    }
}

string Intree::Outtree::getCompressedString() const{
    return compressedString;
}

void Intree::Outtree::canonicalize(){
    for(Outtree* neighbor : predecessors){
        neighbor->canonicalize();
    }
    sort(predecessors.begin(), predecessors.end(),
            [](const Outtree* a, const Outtree* b) -> bool {
                return *b < *a;
            }
        );
    if(compressedString.size() == 0){
        if(predecessors.size() == 0){
            if(marked){
                compressedString += "0";
            }
            else{
                compressedString += "1";
            }
        }
        else{
            compressedString += "[";
            for(auto a : predecessors){
                compressedString += a->getCompressedString();
            }
            compressedString += "]";
        }
    }
}

Intree Intree::Outtree::toIntree(map<task_id, task_id>& isomorphism) const {
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
    for(task_id it = 1; it<t.edges.size(); ++it){
        inner_iso_rev[it] = it;
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
    for(task_id it = 1; it<t.edges.size(); ++it){
        max_tid = max(max_tid, it);
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
    vector<vector<task_id>> tasks_by_level(t.edges.size() + 1);
    // store tasks grouped by level
    for(task_id it = 1; it<t.edges.size(); ++it){
        tasks_by_level[t.get_level(it)].push_back(it);
    }
    tasks_by_level[0].push_back(0); // 0 is not present in edges
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
    for(task_id it=1; it < _t.edges.size(); ++it){
        edges.push_back(pair<Task,Task>(Task(isomorphism[it]),Task(isomorphism[_t.edges[it]])));
    }
    // TODO: Expand to more than 64 bits!
    out.clear();
    for(unsigned int i=0; i<canonical_names[0].size(); ++i){
        out.push_back(canonical_names[0][i] > 0 ? 1u : 0u);
    }
    return Intree(edges);
}

unsigned int Intree::count_tasks() const{
    unsigned int result = 1;
    for(task_id it = 1; it < edges.size(); ++it){
        if (edges[it]!=NOTASK){
            result++;
        }
    }
    return result;
}

int Intree::get_in_degree(const Task& t) const {
    return get_in_degree(t.get_id());
}

int Intree::get_in_degree(const task_id t) const {
    int result = 0;
    for(task_id it = 1; it<edges.size(); ++it){
        if(t == edges[it])
            result++;
    }
    return result;
}

bool Intree::contains_task(task_id tid) const{
    return tid<edges.size() && edges[tid]!=NOTASK;
}

#if USE_TASKMAP
const Task& Intree::get_task_by_id(const task_id tid) const {
    if(edges.find(tid) == edges.end()){
        cout << "Attempted to get_task_by_id of non-existent task." << endl;
        throw 1;
    }
    return taskmap.find(tid)->second;
}
#endif

void Intree::get_tasks(set<task_id>& result) const {
    result.insert(0);
    for(task_id it = 1; it<edges.size(); ++it){
        if(edges[it]!=NOTASK){
            result.insert(it);
        }
    }
}

void Intree::rename_leaf(task_id original, task_id now){
#if USE_TASKMAP
    assert(taskmap.find(original)!=taskmap.end());
    // stuff in taskmap
    auto tmp = taskmap[original];
    taskmap.erase(taskmap.find(original));
    taskmap[now] = tmp;
#endif
    // stuff in edges
    auto tmp2 = edges[original];
    edges[original] = NOTASK;
    edges[now] = tmp2;
}

#if USE_TASKMAP
Distribution Intree::get_task_distribution(const task_id t) const {
    return taskmap.find(t)->second.get_distribution();
}
#endif

int Intree::get_level(const Task& t) const{
    return get_level(t.get_id());
}

void Intree::get_predecessors(const Task& t, vector<task_id>& target) const{
    get_predecessors(t.get_id(), target);
}

void Intree::get_predecessors(const task_id t, vector<task_id>& target) const{
    for(task_id it = 1; it < edges.size(); ++it){
        if(edges[it]==t){
            target.push_back(it);
        }
    }
}

void Intree::get_leaves(vector<task_id>& target) const{
    for(task_id it = 1; it < edges.size(); ++it){
        if (get_in_degree(it) == 0){
            target.push_back(it);
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
#if USE_TASKMAP
    // remove from taskmap
    auto todel = taskmap.find(t);
    taskmap.erase(todel);
#endif
    edges[t] = NOTASK;
    // vector<pair<task_id, task_id>> tmp;
    // for(task_id it = 1; it < edges.size(); ++it){
    //     if(!((it == t) || (edges[it] == t))){
    //         cout << "Adding " << it << " - " << edges[it] << endl;
    //         tmp.push_back(pair<task_id, task_id>(it, edges[it]));
    //     }
    // }
    // edges.clear();
    // for(auto it = tmp.begin(); it != tmp.end(); ++it){
    //     edges[it->first] = it->second;
    // }
    // edges[0] = NOTASK;
}

pair<Task, Task> Intree::get_edge_from(const Task& t) const {
    return get_edge_from(t.get_id());
}

pair<Task, Task> Intree::get_edge_from(const task_id t) const {
    for(task_id it = 1; it < edges.size(); ++it){
        if(t == it){
            return pair<Task, Task>(Task(it), Task(edges[it]));
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
    for(task_id it = 1; it < edges.size(); ++it){
        if(cont.find(get_level(edges[it])) != cont.end() && cont[get_level(edges[it])] != edges[it]){
            return false;
        }
        cont[get_level(edges[it])] = edges[it];
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
    for(task_id it = 1; it < edges.size(); ++it){
        if(get_in_degree(it) == 0){
            vector<task_id> nv;
            get_chain(it, nv);
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
    for(task_id it = 1; it < edges.size(); ++it){
        target[get_level(it)]++;
    }
    assert(target[0]==0);
    target[0] = 1;
}

void Intree::get_raw_tree_id(tree_id& target){
    task_id max_id = 0;
    for(task_id it = 1; it<edges.size(); ++it){
        if(edges[it] != NOTASK){
            max_id = it;
        }
    }
    for(unsigned int i=0; i<max_id+1u; ++i){
        target.push_back(NOTASK);
    }
    for(task_id it = 1; it < edges.size(); ++it){
        target[it] = edges[it];
    }
}
    
void Intree::get_reverse_tree(map<task_id, vector<task_id>>& rt) const{
    queue<task_id> q;
    q.push(0);
    rt[0] = vector<task_id>();
    while (q.size() > 0){
        task_id current = q.front();
        q.pop();
        for(task_id it = 1; it<edges.size(); ++it){
            if(edges[it] == current){
                q.push(it);
                rt[edges[it]].push_back(it);
                if(rt.find(it) == rt.end()){
                    rt[it] = vector<task_id>();
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
    vector<vector<task_id>> chains;
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
