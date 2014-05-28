#include "taskdag.h"

TaskDAG::TaskDAG(){
    assert(false);
}

TaskDAG::TaskDAG(const TaskDAG& t) :
    edges(t.edges)
{
}

TaskDAG::TaskDAG(const vector<pair<task_id, vector<task_id>>>& _edges){
    for (auto& e : _edges) {
        edges[e.first].insert(edges[e.first].end(), e.second.begin(), e.second.end());
    }
}

TaskDAG::TaskDAG(const map<task_id, vector<task_id>>& _edges) :
    edges(_edges)
{
}

/*static*/ TaskDAG TaskDAG::canonical_taskdag(const TaskDAG& t, 
        const vector<task_id>& preferred,
        map<task_id, task_id>& isomorphism, 
        tree_id& out){
    // TODO: 
    // Implement this method so we return a TaskDAG that is a canonical
    // variant (i.e. up to isomorphism the same) to t.
    // preferred contains the tasks that are currently scheduled
    // isomorphism is an output parameter. Write the mapping from the original
    // tasks (in t) to the canonical tasks (in the result) into this vector
    // tree_id out holds some "small representation" of the DAG (you've
    // to invent some)
    assert(false);
}


unsigned int TaskDAG::count_tasks() const{
    return edges.size();
}


int TaskDAG::get_in_degree(const task_id t) const {
    int result = 0;
    for (auto const& e : edges) {
        auto sucs = e.second;
        if(find(sucs.begin(), sucs.end(), t) != sucs.end()){
            result++;
        }
    }
    return result;
}


bool TaskDAG::contains_task(task_id tid) const{
    return edges.find(tid)!=edges.end();
}

void TaskDAG::get_tasks(set<task_id>& result) const{
    for (auto& e : edges){
        result.insert(e.first);
    }
}

void TaskDAG::get_tasks(vector<task_id>& result) const{
    for (auto& e : edges){
        result.push_back(e.first);
    }
}

vector<task_id> TaskDAG::get_tasks() const{
    vector<task_id> result;
    get_tasks(result);
    return result;
}

void TaskDAG::rename_leaf(task_id original, task_id now){
    assert(contains_task(original));
    assert(is_leaf(original));
    // TODO: 
    // The following is probably quite inefficient due to 
    // copying and re-inserting. There must be a smarter way that is
    // supported by std::map to change the key of a value.
    auto tmp = edges[original];
    edges.erase(original);
    edges[now] = tmp;
}


unsigned int TaskDAG::get_level(const task_id t) const{
    // TODO: Is the notation of a level useful for a DAG?
    // If not, simply remove method.
    assert(false);
    return -1;
}


vector<task_id> TaskDAG::get_successors(const task_id t) const{
    return edges.find(t)->second;
}

void TaskDAG::get_predecessors(const task_id t, vector<task_id>& target) const{
    for (auto const& e : edges) {
        auto sucs = e.second;
        if(find(sucs.begin(), sucs.end(), t) != sucs.end()){
            if(find(target.begin(), target.end(), t)==target.end()){
                target.push_back(t);
            }
        }
    }
}

vector<task_id> TaskDAG::get_predecessors(const task_id t) const{
    vector<task_id> result;
    get_predecessors(t, result);
    return result;
}

void TaskDAG::get_siblings(const task_id t, vector<task_id>& target) const{
    // TODO: Is it useful to have the notation of siblings in DAGs?
    assert(false);
}

vector<task_id> TaskDAG::get_siblings(const task_id t) const{
    // TODO: Is it useful to have the notation of siblings in DAGs?
    assert(false);
    return vector<task_id>();
}

void TaskDAG::get_leaves(set<task_id>& target) const{
    // TODO: This is probably inefficient
    for(auto e : edges){
        if(0 == get_predecessors(e.first).size()){
            target.insert(e.first);
        }
    }
}

void TaskDAG::get_leaves(vector<task_id>& target) const{
    // TODO: This is probably inefficient
    for(auto e : edges){
        if(0 == get_predecessors(e.first).size()){
            target.push_back(e.first);
        }
    }
}

vector<task_id> TaskDAG::get_leaves() const{
    vector<task_id> result;
    get_leaves(result);
    return result;
}

bool TaskDAG::is_leaf(const task_id t) const{
    return 0 == get_predecessors(t).size();
}


void TaskDAG::remove_task(task_id t){
    assert(is_leaf(t));
    edges.erase(t);
}

void TaskDAG::get_raw_tree_id(tree_id& target) const{
    // TODO: a tree_id is a (in principle) short identification
    // number/string/whatever of the whole tree. Implement it if needed.
    assert(false);
}

bool TaskDAG::operator==(const TaskDAG& t) const{
    return edges==t.edges;
}

ostream& operator<<(ostream& os, const TaskDAG& t){
    os << "{";
    for (auto& e : t.edges) {
        os << "(";
        os << e.first << ": ";
        for (auto& s: e.second){
            os << s << ", ";
        }
        os << ")";
    }
    os << "}";
    return os;
}

