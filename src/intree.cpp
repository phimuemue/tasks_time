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

Intree::Intree(Intree&& t) :
    edges(std::move(t.edges))
#if USE_TASKMAP
    ,
    taskmap(std::move(t.taskmap))
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

Intree::Intree(const vector<task_id>&& arg_edges) :
    edges(arg_edges)
{
#if USE_TASKMAP
    for(unsigned int i=0; i<edges.size(); ++i){
        taskmap[i] = Task(i);
        taskmap[edges[i]] = Task(edges[i]);
    }
    taskmap[0] = Task(0);
#endif
}

Intree::Outtree::Outtree(task_id i, bool m) :
    id(i),
    marked(m)
{
}

Intree::Outtree::Outtree(const Intree& i, const vector<task_id>& marked) :
    id(0)
{
    vector<Outtree*> outtrees(i.count_tasks() + 1); // outtrees not managing lifetime of elements!
    outtrees[0] = this;
    for(task_id edge = 1; edge < i.edges.size(); ++edge){
        if(i.edges[edge] != NOTASK){
            outtrees[edge] = new Outtree(edge, find(marked.begin(), marked.end(), edge)!=marked.end());
            outtrees[i.edges[edge]]->predecessors.push_back(std::unique_ptr<Outtree>(outtrees[edge]));
        }
    }
}

const vector<char>& Intree::Outtree::getCompressedString() const{
    return compressedString;
}

void Intree::Outtree::canonicalize(){
    for(auto& neighbor : predecessors){
        neighbor->canonicalize();
    }
    sort(predecessors.begin(), predecessors.end(),
            [](auto& outtreeA, auto& outtreeB) -> bool {
                return *outtreeB < *outtreeA;
            }
        );
    if(compressedString.size() == 0){
        if(predecessors.size() == 0){
            if(marked){
                compressedString.push_back('0');
            }
            else{
                compressedString.push_back('1');
            }
        }
        else{
            compressedString.push_back('[');
            for(auto& a : predecessors){
                compressedString.insert(
                    compressedString.end(),
                    a->getCompressedString().begin(),
                    a->getCompressedString().end()
                );
            }
            compressedString.push_back(']');
        }
    }
}

Intree Intree::Outtree::toIntree(map<task_id, task_id>& isomorphism) const {
#if 0
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
            assert(edges.size() == neighbor->id);
            q.push(neighbor);
        }
    }
    return Intree(edges);
#else
    Intree result;
    result.edges.push_back(NOTASK);
    queue<const Outtree*> q;
    q.push(this);
    task_id counter = 0;
    while(!q.empty()){
        const Outtree* current = q.front();
        q.pop();
        for(auto& neighbor : current->predecessors){
            ++counter;
            isomorphism[neighbor->id] = counter;
            neighbor->id = counter;
            // cout << "Edge " << neighbor->id << " " << current->id << endl;
            result.edges.push_back(current->id);
            q.push(neighbor.get());
        }
    }
    return result;
#endif
}

// For some reason, the detour via outtrees works
// faster than canonical_intree3, which is a "1-to-1" translation
// of the AHU algorithm
std::pair<Intree, std::map<task_id, task_id>> Intree::canonical_intree(
    const Intree& _t, 
    const vector<task_id>& _preferred
){
    map<task_id, task_id> isomorphism;
    Outtree ot(_t, _preferred);
    ot.canonicalize();
    Intree result = ot.toIntree(isomorphism);
    return std::make_pair(std::move(result), std::move(isomorphism));
}

std::pair<Intree, std::map<task_id, task_id>> Intree::canonical_intree3(
    const Intree& _t,
    const vector<task_id>& _preferred
){
    map<task_id, task_id> isomorphism;
    // distribute tasks into levels
    map<unsigned int, vector<task_id>> levels;
    levels[0].push_back(0);
    for(task_id i = 1; i<_t.edges.size(); ++i){
        if(_t.get_successor(i) != NOTASK){
            //assert(_t.get_level(i) < 4096);
            levels[_t.get_level(i)].push_back(i);
        }
    }
    map<task_id, int> labels;
    // assign labels to leaves
    vector<task_id> const leaves = _t.get_leaves();
    for(task_id it : leaves){
        labels[it] = find(_preferred.begin(), _preferred.end(), it)==_preferred.end() ? 0 : -1;
    }
    for(unsigned int lev = levels.size() - 2; lev < 4095 && lev >= 0; --lev){
        auto& currentLevel = levels[lev];
        map<task_id, vector<int>> intermediate_labels;
        // construct new labels
        for(task_id it : levels[lev + 1]){
            if(intermediate_labels.count(_t.get_successor(it)) == 0){
                // TODO: why is inserted 4096 at the beginning of intermediate label? Necessary?
                intermediate_labels[_t.get_successor(it)].push_back(4096);
            }
            intermediate_labels[_t.get_successor(it)].push_back(labels[it]);
        }
        for(task_id it : currentLevel){
            // check if it is leaf
            auto const labels_it = labels.find(it);
            if(labels_it != labels.end()){
                intermediate_labels[it].push_back(labels_it->second);
            }
        }
#if 1
        // just for assertions
        for(auto it : currentLevel){
            assert(intermediate_labels.find(it) != intermediate_labels.end());
        }
#endif
        // sort level ...
        sort(currentLevel.begin(), currentLevel.end(), 
                [&intermediate_labels](const task_id a, const task_id b) -> bool {
                    return intermediate_labels[a] < intermediate_labels[b];
                }
            );
        // ... compute (short) labels
        assert(currentLevel.size() > 0);
        if(labels.count(currentLevel[0]) == 0){
            labels[currentLevel[0]] = 2;
        }
        int counter = 3;
        for(unsigned int i = 1; i < currentLevel.size(); ++i){
            assert(intermediate_labels.find(currentLevel[i]) != intermediate_labels.end());
            assert(intermediate_labels.find(currentLevel[i-1]) != intermediate_labels.end());
            if(intermediate_labels[currentLevel[i]] != intermediate_labels[currentLevel[i-1]]){
                counter++;
            }
            labels[currentLevel[i]] = counter;
        }
    }
    // compute isomorphism and return canonical intree
    isomorphism[0] = 0;
    task_id counter = 1;
    vector<task_id> edges = {NOTASK};
    for(unsigned int lev = 1; lev < levels.size(); ++lev){
        for(auto it : levels[lev]){
            isomorphism[it] = counter;
            edges.push_back(isomorphism[_t.get_successor(it)]);
            counter++;
        }
    }
    Intree result = Intree(std::move(edges));
    return std::make_pair(result, isomorphism);
}

std::pair<Intree, std::map<task_id, task_id>> Intree::canonical_intree2(
    const Intree& _t, 
    const vector<task_id>& _preferred
){
    map<task_id, task_id> isomorphism;
    Intree t(_t);
    vector<task_id> preferred(_preferred);
    map<task_id, task_id> inner_iso_rev;
    inner_iso_rev[0] = 0;
    for(task_id it = 1; it<t.edges.size(); ++it){
        inner_iso_rev[it] = it;
    }
    // make leaves minimal
    vector<task_id> leaves = t.get_leaves();
    sort(leaves.begin(), leaves.end(),
        [&](const task_id a, const task_id b) -> bool {
#if 0
            if(t.get_edge_from(a).second == t.get_edge_from(b).second)
                return a < b;
            return t.get_edge_from(a).second < t.get_edge_from(b).second;
#else
            if(t.get_successor(a) == t.get_successor(b))
                return a < b;
            return t.get_successor(a) < t.get_successor(b);
#endif
        }
    );
    task_id max_tid = 0;
    for(task_id it = 1; it<t.edges.size(); ++it){
        max_tid = max(max_tid, it);
    }
    max_tid++;
    for(auto const& it : leaves){
        ++max_tid;
        t.rename_leaf(it, max_tid);
        inner_iso_rev[max_tid] = it;
        for_each(preferred.begin(), preferred.end(),
            [&](task_id& a){
                if(a==it){
                    a = max_tid;
                }
            }
        );
    }
    // initialize marked_count
    map<task_id, unsigned int> marked_count;
    for(auto const& it : preferred){
        marked_count[it] = 1;
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
        for(auto const& it : *rit){
            vector<unsigned short> canonical_name;
            vector<task_id> predecessors = t.get_predecessors(it);
            for(auto pit : predecessors){
                marked_count[it] += marked_count[pit];
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
            all_predecessors[it] = predecessors;
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
            if(find(preferred.begin(), preferred.end(), it) != preferred.end()){
                canonical_name.push_back(2);
            }
            else{
                canonical_name.push_back(1);
            }
            canonical_names[it] = canonical_name;
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
#if USE_TASKMAP
    vector<pair<Task, Task>> edges;
    for(task_id it=1; it < _t.edges.size(); ++it){
        edges.push_back(make_pair(Task(isomorphism[it]),Task(isomorphism[_t.edges[it]])));
    }
#else
    vector<pair<task_id, task_id>> edges;
    for(task_id it=1; it < _t.edges.size(); ++it){
        edges.push_back(make_pair(isomorphism[it],isomorphism[_t.edges[it]]));
    }
#endif
    return std::make_pair(Intree(edges), isomorphism);
}

unsigned int Intree::count_tasks() const{
    // TODO: This is extremely expensive given that it only should count the tasks
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
    // one might be tempted to start at task_id it = t+1,
    // but for some reason, this seemed to be slower in some cases
    // and not much faster in other ones
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
    if(edges.count(tid) == 0){
        cout << "Attempted to get_task_by_id of non-existent task." << endl;
        throw 1;
    }
    return taskmap.at(tid);
}
#endif

vector<task_id> Intree::get_tasks() const{
    vector<task_id> result;
    result.push_back(0);
    for(task_id it = 1; it<edges.size(); ++it){
        if(edges[it]!=NOTASK){
            result.push_back(it);
        }
    }
    return result;
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
    return taskmap.at(t).get_distribution();
}
#endif

unsigned int Intree::get_level(const Task& t) const{
    return get_level(t.get_id());
}

task_id Intree::get_successor(const task_id t) const{
    return edges[t];
}

vector<task_id> Intree::get_predecessors(const Task& t) const{
    return get_predecessors(t.get_id());
}

vector<task_id> Intree::get_predecessors(const task_id t) const{
    vector<task_id> result;
    for(task_id it = 1; it < edges.size(); ++it){
        if(edges[it]==t){
            result.push_back(it);
        }
    }
    return result;
}

vector<task_id> Intree::get_siblings(const task_id t) const {
    vector<task_id> target;
    task_id successor = edges[t];
    for(task_id it = 0; it < edges.size(); ++it){
        if(edges[it] == successor && it != t){
            target.push_back(it);
        }
    }
    return target;
}

vector<task_id> Intree::get_leaves() const{
    vector<task_id> result;
#if !USE_CANONICAL_SNAPSHOT
    for(task_id it = 1; it < edges.size(); ++it){
        if (get_in_degree(it) == 0){
            result.push_back(it);
        }
    }
#else
    vector<bool> leaf_candidates(edges.size(), true);
    for(task_id it : edges){
        if(it != NOTASK){
            leaf_candidates[it] = false;
        }
    }
    leaf_candidates[0] = false;
    for(task_id it = 0; it < leaf_candidates.size(); ++it){
        if(leaf_candidates[it] && edges[it]!=NOTASK){
            result.push_back(it);
        }
    }
#endif
    return result;
}

bool Intree::is_leaf(const task_id t) const{
    return get_in_degree(t)==0;
}

unsigned int Intree::get_level(const task_id t) const{
    task_id current = t;
    unsigned int result = 0;
    while(current > 0
#if !USE_CANONICAL_SNAPSHOT
            && current < NOTASK
#endif
            ){
#if 0
        auto edge = get_edge_from(current);
        current = edge.second.get_id();
#else
        current = get_successor(current);
#endif
        result++;
    }
    return result;
}


task_id Intree::remove_task(Task& t){
    return remove_task(t.get_id());
}

task_id Intree::remove_task(task_id t){
    // only tasks with no predecessor can be removed
    // TODO: this should be an assert
    assert(get_in_degree(t) == 0);
#if USE_TASKMAP
    // remove from taskmap
    auto todel = taskmap.find(t);
    taskmap.erase(todel);
#endif
    auto const result = edges[t];
    edges[t] = NOTASK;
    return result;
}

pair<Task, Task> Intree::get_edge_from(const Task& t) const {
    return get_edge_from(t.get_id());
}

pair<Task, Task> Intree::get_edge_from(const task_id t) const {
    return pair<Task, Task>(Task(t), Task(edges[t]));
    for(task_id it = 1; it < edges.size(); ++it){
        if(t == it){
            return pair<Task, Task>(Task(it), Task(edges[it]));
        }
    }
    cout << "Attempted to get edge from non-existent task (" << t << " from " << *this << ")." << endl;
    throw 0;
}

bool Intree::is_chain() const {
    // TODO: this can be made more efficient, but it is not needed very often
    vector<vector<task_id>> chains = get_chains();
    return chains.size() == 1;
}

bool Intree::is_degenerate_tree() const {
    // store - for each level - the one task that may have predecessors
    map<unsigned int, task_id> cont;
    for(task_id it = 1; it < edges.size(); ++it){
        if(edges[it] == NOTASK){
            continue;
        }
        if(cont.count(get_level(edges[it])) != 0 && cont[get_level(edges[it])] != edges[it]){
            return false;
        }
        cont[get_level(edges[it])] = edges[it];
    }
    return true;
}

bool Intree::is_parallel_chain() const {
    map<task_id, unsigned int> pred_count;
    for(task_id it = 1; it < edges.size(); ++it){
        if(edges[it] == NOTASK){
            continue;
        }
        pred_count[edges[it]]++;
    }
    for(auto it : pred_count){
        if (it.second > 1 && it.first!=0){
            return false;
        }
    }
    return true;
}

bool Intree::same_chain(const task_id t1, const task_id t2) const {
    return get_chain(t1) == get_chain(t2);
}

unsigned int Intree::count_free_chains(vector<task_id>& target) const{
    unsigned int result = 0;
    vector<task_id> const leaves = get_leaves();
    for(auto const& it : leaves){
        if(get_in_degree(it)==0){
            target.push_back(it);
            result++;
        }
    }
    return result;
}

vector<task_id> Intree::get_chain(const Task& t) const {
    return get_chain(t.get_id());
}

vector<task_id> Intree::get_chain(const task_id t) const {
    vector<task_id> result;
    assert(t!=NOTASK);
    task_id current = t;
    result.push_back(current);
    while(current > 0u){
        assert(current!=NOTASK);
#if 0
        auto edge = get_edge_from(current);
        current = edge.second.get_id();
#else
        current = get_successor(current);
#endif
        result.push_back(current);
        if(current == NOTASK){
            result.clear();
        }
    }
    return result;
}

vector<vector<task_id>> Intree::get_chains() const {
    vector<vector<task_id>> result;
    for(task_id it = 1; it < edges.size(); ++it){
#if !USE_CANONICAL_SNAPSHOT
        if(edges[it] != NOTASK)
#endif
        {
            if(get_in_degree(it) == 0){
                result.push_back(get_chain(it));
            }
        }
    }
    return result;
}

unsigned int Intree::longest_chain_length() const{
    if(edges.size()<=1){
        return edges.size()+1;
    }
#if 1
    // NOTE: use the other variant if working with a huge set of intrees!
    vector<vector<task_id>> const chains = get_chains();
    unsigned int result = 0;
    for_each(chains.begin(), chains.end(),
        [&result](const vector<task_id> c){
            result = max(result, (unsigned int)c.size());
        }
    );
#else
    // this can only be used if the input is given in a strictly increasing order
    unsigned int result = 1;
    for(unsigned int idx = edges.size() - 1; idx > 0; idx = edges[idx]){
        result++;
    }
    result = max(result, 1u);
#endif
    return result;
}

vector<unsigned int> Intree::get_profile() const {
    vector<unsigned int> result;
    for(unsigned int i=0; i<longest_chain_length(); ++i){
        result.push_back(0);
    }
    for(task_id it = 1; it < edges.size(); ++it){
        result[get_level(it)]++;
    }
    if(result.size() > 0){
        assert(result[0]==0);
        result[0] = 1;
    }
    else{
        result.push_back(1);
    }
    return result;
}

tree_id Intree::get_raw_tree_id() const {
    // the following can be used to swith between one
    // (I think) quite correct solution and a 
    // (I think) quite correct but more efficient solution
    tree_id target;
#if 1
#if TREE_ID_TYPE==TREE_ID_DEFAULT
    for(task_id it = 1; it < edges.size(); ++it){
        target.push_back(edges[it]);
    }
#elif TREE_ID_TYPE==TREE_ID_NONE
    target = *this;
#endif
#else
    task_id max_id = 0;
    for(task_id it = 1; it<edges.size(); ++it){
        if(edges[it] != NOTASK){
            max_id = it;
        }
    }
    for(unsigned int i=0; i<max_id+1u; ++i){
        target.push_back(NOTASK);
    }
    target.push_back(NOTASK);
    for(task_id it = 1; it < edges.size(); ++it){
        target[it] = edges[it];
    }
#endif
    return target;
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
                if(rt.count(it) == 0){
                    rt[it] = vector<task_id>();
                }
            }
        }
    }
}

unsigned int Intree::get_max_width(task_id tid) const{
    // This is the old implementation
    // unsigned result = 0;
    // map<task_id, vector<task_id>> rt;
    // get_reverse_tree(rt);
    // for(auto it : rt[tid]){
    //     result += get_max_width(it);
    // }
    map<task_id, unsigned int> width;
    for(unsigned int idx = edges.size() - 1; idx < edges.size() && idx>=tid; --idx){
        width[edges[idx]] += max(width[idx], 1u);
    }
    return max(width[tid], 1u);
}

ostream& operator<<(ostream& os, const Intree& t){
    if(t.count_tasks()==1){
        os << "[0]";
        return os;
    }
    vector<vector<task_id>> chains = t.get_chains();
    os << "[" << stringhelper::join(chains, "] [",
        [](vector<task_id> const& chain){
            return stringhelper::join(chain, ", ");
        }
    ) << "]";
    return os;
}

bool Intree::operator==(const Intree& t) const {
    return edges==t.edges;
}

bool Intree::operator<(const Intree& t) const {
    return edges<t.edges;
}

#if PYTHON_TESTS
#include <boost/python.hpp>
using namespace boost::python;
BOOST_PYTHON_MODULE(intree)
{
    class_<std::vector<task_id>>("task_id_vector")
        .def(boost::python::vector_indexing_suite<std::vector<task_id>>());
    class_<Intree>("Intree")
        .def("count_tasks", &Intree::count_tasks)
        .def("get_in_degree", (int (Intree::*)(const task_id) const)&Intree::get_in_degree)
        .def("contains_task", &Intree::contains_task)
        .def("get_tasks", (vector<task_id>(Intree::*)() const)&Intree::get_tasks)
        .def("get_level", (unsigned int(Intree::*)(const task_id) const)&Intree::get_level)
        .def("get_successor", &Intree::get_successor)
        .def("get_predecessors", (vector<task_id>(Intree::*)(const task_id) const)&Intree::get_predecessors)
        .def("get_siblings", (vector<task_id>(Intree::*)(const task_id) const)&Intree::get_siblings)
        .def("get_leaves", (vector<task_id>(Intree::*)() const)&Intree::get_leaves)
        .def("is_leaf", &Intree::is_leaf)
        .def("is_chain", &Intree::is_chain)
        .def("is_degenerate_tree", &Intree::is_degenerate_tree)
        .def("is_parallel_chain", &Intree::is_parallel_chain)
        .def("same_chain", &Intree::same_chain)
        ;
}
#endif
