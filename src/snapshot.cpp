#include "snapshot.h"

map<pair<tree_id,vector<task_id>>, Snapshot*> Snapshot::pool;

void Snapshot::clear_pool(){
    // We have to ensure that we don't double-delete some pointers
    // TODO: Why does it not work this way?
    map<Snapshot*, bool> done;
    for(auto it=Snapshot::pool.begin(); it!=Snapshot::pool.end(); ++it){
        if(done.find(it->second)!=done.end()){
            delete(it->second);
            done[it->second] = true;
        }
    }
}

Snapshot::Snapshot(){

}

Snapshot::Snapshot(const Snapshot& s){
    throw "Copy constructor for snapshot to be implemented";
}

Snapshot::Snapshot(Intree& t) :
    intree(t)
{

}

Snapshot::Snapshot(Intree& t, vector<task_id> m) :
    intree(t)
{
    sort(m.begin(), m.end());
    marked = m;
    for(auto it=m.begin(); it!=m.end(); ++it){
        vector<task_id> tmp;
        t.get_predecessors(*it, tmp);
        if(tmp.size()!=0){
            cout << "Trying to construct snapshot with non-leaf marked tasks." << endl;
            throw 1;
        }
    }
}

Snapshot::~Snapshot(){
#if USE_CANONICAL_SNAPSHOT
    Snapshot::pool.clear();
#else
    for(auto it=successors.begin(); it!=successors.end(); ++it){
        delete(*it);
    }
#endif
}

Snapshot* Snapshot::canonical_snapshot(Intree& t, vector<task_id> m){
    //vector<task_id> m(_m);
#if USE_SIMPLE_OPENMP
    cout << "Warning! Using openmp with canonical snapshots!" << endl;
#endif
    cout << "Canonical snapshot for " << t << " | ";
    for(auto it=m.begin(); it!=m.end(); ++it){
        cout << *it << ", ";
    }
    cout << endl;

    map<task_id, task_id> isomorphism;
    tree_id tid;
    Intree tmp = Intree::canonical_intree(t, m, isomorphism, tid);
    for(auto it=isomorphism.begin(); it!=isomorphism.end(); ++it){
        cout << it->first << " -> " << it->second << endl;
    }
    // adjust m properly
    transform(m.begin(), m.end(), m.begin(),
        [&](const task_id a) -> task_id {
            return isomorphism[a];
        }
    );
    m.erase(remove_if(m.begin(), m.end(),
        [&t](const task_id a) -> bool {
            return !t.contains_task(a);
        }), 
        m.end()
    );
    map<task_id, unsigned int> counts;
    for(auto it=m.begin(); it!=m.end(); ++it){
        cout << "Test: " << *it << endl;
        if(*it!=0){
            if(counts.find(t.get_edge_from(*it).second.get_id())==counts.end()){
                counts[t.get_edge_from(*it).second.get_id()] = 0;
            }
            counts[t.get_edge_from(*it).second.get_id()]++;
        }
    }
    map<task_id, vector<task_id>> predecessor_collection;
    for(auto it=counts.begin(); it!=counts.end(); ++it){
        t.get_predecessors(it->first, predecessor_collection[it->first]);
        sort(predecessor_collection[it->first].begin(),
             predecessor_collection[it->first].end());
    }
    m.clear();
    for(auto it=counts.begin(); it!=counts.end(); ++it){
        for(unsigned int i=0; i<it->second; ++i){
            m.push_back(predecessor_collection[it->first][i]);
        }
    }
    cout << "New m: " << endl;
    for(auto it=m.begin(); it!=m.end(); ++it){
        cout << *it << ", ";
    }
    cout << endl;
    // construct newmarked
    vector<task_id> newmarked;
    for(auto it=m.begin(); it!=m.end(); ++it){
        cout << "pushing " << isomorphism[*it] << "(from " << *it << ")" << endl;
        newmarked.push_back(isomorphism[*it]);
    }
    sort(newmarked.begin(), newmarked.end());
    cout << "looking for " << tmp << "(" << tid << ") | ";
    for(auto it=newmarked.begin(); it!=newmarked.end(); ++it){
        cout << *it << ", ";
    }
    cout << endl;
    auto find_key = pair<tree_id,vector<task_id>>(tid, newmarked);
    auto correct_pool = 
        Snapshot::pool.find(find_key);
    if(correct_pool == Snapshot::pool.end()){
        Snapshot::pool[find_key] = new Snapshot(tmp, newmarked);
    }
    cout << *(Snapshot::pool.find(find_key)->second) << endl;
    isomorphism.clear();
    tid = 0;
    Intree::canonical_intree(Snapshot::pool.find(find_key)->second->intree, newmarked, isomorphism, tid);
    cout << tid << endl;
    return Snapshot::pool.find(find_key)->second;
}

void Snapshot::get_successors(const Scheduler& scheduler){
    // we only want to compute the successors once
    if(successors.size()>0)
        return;
    // or maybe there aren't even successors
    if(intree.count_tasks()==1)
        return;
    vector<myfloat> finish_probs;
    Probability_Computer().compute_finish_probs(intree, marked,
        finish_probs);
    assert(finish_probs.size()==marked.size());
    // then, for each finished threads, compute all possible successors
    auto finish_prob_it = finish_probs.begin();
    cout << "Successors of " << *this << endl;
    for(auto it = marked.begin(); it!=marked.end(); ++it, ++finish_prob_it){
#if SIMPLE_ISOMORPHISM_CHECK
        if(*finish_prob_it == 0)
            continue;
#endif
        Intree tmp(intree);
        tmp.remove_task(*it);
        vector<pair<task_id,myfloat>> raw_sucs;
        scheduler.get_next_tasks(tmp, marked, raw_sucs);
        // we have to check if the scheduler even found a new task to schedule
        if(raw_sucs.size() > 0){
            for(unsigned int i=0; i<raw_sucs.size(); ++i){
                vector<task_id> newmarked(marked);
                newmarked.erase(remove_if(newmarked.begin(), newmarked.end(),
                            [it](const task_id& a){
                            return a==*it;
                            }), newmarked.end());
                // TODO: Is this needed?
                if(raw_sucs[i].first != NOTASK)
                    newmarked.push_back(raw_sucs[i].first);
                sort(newmarked.begin(), newmarked.end());
                // TODO: every "new" needs a "delete"
#if USE_CANONICAL_SNAPSHOT
                Snapshot* news = Snapshot::canonical_snapshot(tmp, newmarked);
#else
                Snapshot* news = new Snapshot(tmp, newmarked);
#endif
                successors.push_back(news);
                successor_probs.push_back(*finish_prob_it * raw_sucs[i].second);
            }
        }
        else {
            vector<task_id> newmarked(marked);
            sort(newmarked.begin(), newmarked.end());
            newmarked.erase(remove_if(newmarked.begin(), newmarked.end(),
                        [it](const task_id& a){
                        return a==*it;
                        }), newmarked.end());
            // TODO: every "new" needs a "delete"
#if USE_CANONICAL_SNAPSHOT
            Snapshot* news = Snapshot::canonical_snapshot(tmp, newmarked);
#else
            Snapshot* news = new Snapshot(tmp, newmarked);
#endif
            successors.push_back(news);
            successor_probs.push_back(*finish_prob_it);
        }
    }
    cout << "RAW_SUCCESSORS:" << endl;
    for(auto it=successors.begin(); it!=successors.end(); ++it){
        cout << **it << endl;
    }
    // remove duplicate successors
    assert(successors.size() == successor_probs.size());
    for(unsigned int i=0; i<successors.size(); ++i){
        for(unsigned int j=i+1; j<successors.size(); ++j){
            if(successors[i] == successors[j]){
                successors[j] = NULL;
                successor_probs[i] += successor_probs[j];
                successor_probs[j] = (myfloat)0;
            }
        }
    }
    successors.erase(remove_if(successors.begin(), successors.end(),
        [](const Snapshot* a) -> bool {
            return a == NULL;
        }
    ), successors.end());
    successor_probs.erase(remove_if(successor_probs.begin(), successor_probs.end(),
        [](const myfloat& a) -> bool {
            return a == (myfloat)0;
        }
    ), successor_probs.end());
    cout << "Normalized successors:" << endl;
    for(auto it=successors.begin(); it!=successors.end(); ++it){
        cout << **it << endl;
    }
}

void Snapshot::compile_snapshot_dag(const Scheduler& scheduler){
    get_successors(scheduler);
    for(unsigned int i=0; i<successors.size(); ++i){
        successors[i]->compile_snapshot_dag(scheduler);
    }
}

size_t Snapshot::get_successor_count(){
    return successors.size();
}

myfloat Snapshot::expected_runtime(){
    if (successors.size() == 0){
        return intree.get_task_by_id(0).get_expected_remaining_time();
    }
    assert(successor_probs.size() == successors.size());
    // TODO: compute expected minimum runtime of marked threads dynamically
    myfloat expected_runtime_of_min_task = ((myfloat)1)/(myfloat)marked.size();
    myfloat result = expected_runtime_of_min_task;
    myfloat suc_expected_runtimes[successors.size()];
    for(unsigned int i=0; i<successors.size(); ++i){
        suc_expected_runtimes[i] = successors[i]->expected_runtime();
    }
    for(unsigned int i=0; i<successors.size(); ++i){
        result += successor_probs[i] * suc_expected_runtimes[i];
    }
    return result;
}

string Snapshot::dag_view_string(unsigned int depth, myfloat probability){
    stringstream output;
    for(unsigned int i=0; i<depth; ++i){
        output << " ";
    }
    output << *this << " " << expected_runtime() << " " << probability << endl;
    if(!intree.is_chain()){
        auto pit = successor_probs.begin();
        for(auto it = successors.begin(); it!=successors.end(); ++it, ++pit){
            output << (*it)->dag_view_string(depth+1, *pit);
        }
    }
    return output.str();
}

string Snapshot::tikz_string_internal(const task_id t, 
        map<task_id,vector<task_id>>& rt, bool first) const {
    stringstream output;
    output << "node[circle,scale=0.75,fill";
    if(find(marked.begin(), marked.end(), t) != marked.end()){
        output << ",red";
    }
    output << "]";
    output << "{"; 
#if 0
    output << t;
#endif
    output << "}[grow=up]\n";
    for(auto it = rt[t].begin(); it!=rt[t].end(); ++it){
        output << "child";
        output << "{" << tikz_string_internal(*it, rt, false) << "}" << endl;
    }
    return output.str();
}

string Snapshot::tikz_string(){
    queue<task_id> q;
    q.push(0);
    map<task_id,vector<task_id>> reverse_tree;
    reverse_tree[0] = {};
    while (q.size() > 0){
        task_id current = q.front();
        q.pop();
        for(auto it = intree.edges.begin(); it!=intree.edges.end(); ++it){
            if(it->second == current){
                q.push(it->first);
                reverse_tree[it->second].push_back(it->first);
                if(reverse_tree.find(it->first) == reverse_tree.end()){
                    reverse_tree[it->first] = {};
                }
            }
        }
    }
    return "\\" + tikz_string_internal(0, reverse_tree) + ";";
}

string Snapshot::tikz_string_dag(bool first, unsigned int depth){
    stringstream output;
    if(first){
        output << "% this tree has expected runtime of " << expected_runtime() << endl;
        output << "\\begin{tikzpicture}" << endl;
        for(unsigned int i=0; i < intree.edges.size(); ++i){
            output << "\\tikzstyle{level " << i+1 << "} = " <<
                "[sibling distance = " << 8./(i+1) << "cm, " <<
                "level distance = " << 2. - i/5. << "cm" << "]" << endl;
        }
        output << "\\";
    }
    output << "node[draw=black]{" << endl;
    // output << "\\newsavebox{\\nodebox}" << endl;
    output << "\\sbox{\\nodebox}{" << endl;
    output << "\\begin{tikzpicture}[scale=.2]" << endl;
    for(unsigned int i=0; i < intree.edges.size()+depth; ++i){
        output << "\\tikzstyle{level " << i+1 << "} = " <<
            "[sibling distance = " << 2 << "cm, " <<
            "level distance = " << 2 << "cm" << "]" << endl;
    }   
    output << tikz_string() << endl;
    output << "\\end{tikzpicture}" << endl;
    output << "}" << endl;
    output << "\\usebox\\nodebox" << endl;
    output << "}" << endl;
    if(!intree.is_chain()){
        auto pit = successor_probs.begin();
        for(auto it=successors.begin(); it!=successors.end(); ++it, ++pit){
            output << "child";
            output << "{"; 
            output << (*it)->tikz_string_dag(false, depth+1); 
            output << "edge from parent node [left] { " << *pit << "}" << endl;
            output << "}";
        }
    }
    if(first){
        output << ";" << endl;
        output << "\\end{tikzpicture}" << endl;
    }
    return output.str();
}

void Snapshot::print_snapshot_dag(int depth){
    for(int i=-1; i<depth; ++i){
        cout << "*";
    } 
    cout << " ";
    cout << *this;
    // cout << " (" << expected_runtime() << ") ";
    cout << endl;
    for(auto it=successors.begin(); it!=successors.end(); ++it){
        (*it)->print_snapshot_dag(depth+1);
    }
}

ostream& operator<<(ostream& os, const Snapshot& s){
    os << "<" << s.intree << " | [";
    
    for(auto it = s.marked.begin(); it != s.marked.end(); ++it){
        os << *it;
        if (it+1!=s.marked.end()){
            os << ", ";
        }
    }
    os << "]>";
    return os;
}

string Snapshot::markedstring(){
    stringstream os;
    os << "[";
    
    for(auto it = marked.begin(); it != marked.end(); ++it){
        os << *it;
        if (it+1!=marked.end()){
            os << ", ";
        }
    }
    os << "]";
    return os.str();
}
