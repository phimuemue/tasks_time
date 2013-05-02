#include "snapshot.h"

map<tree_id, map<vector<task_id>,Snapshot*>> Snapshot::pool;

void Snapshot::clear_pool(){
    // We have to ensure that we don't double-delete some pointers
    // TODO: Why does it not work this way?
    map<Snapshot*, bool> done;
    for(auto it=Snapshot::pool.begin(); it!=Snapshot::pool.end(); ++it){
        for(auto it2=it->second.begin(); it2!=it->second.end(); ++it2){
            if(done.find(it2->second)!=done.end()){
                delete(it2->second);
                done[it2->second] = true;
            }
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
    assert(m.size()>0);
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
    map<task_id, task_id> isomorphism;
    tree_id tid;
    Intree tmp = Intree::canonical_intree(t, m, isomorphism, tid);
    vector<task_id> newmarked;
#pragma omp critical
    {
        auto correct_pool = Snapshot::pool.find(tid);
        for(auto it=m.begin(); it!=m.end(); ++it){
            newmarked.push_back(isomorphism[*it]);
        }
        sort(newmarked.begin(), newmarked.end());
        if(correct_pool == Snapshot::pool.end()){
            Snapshot::pool[tid] = map<vector<task_id>,Snapshot*>();
        }
        if(correct_pool->second.find(newmarked) != correct_pool->second.end()){
            correct_pool->second.find(newmarked)->second;
        }
        else {
            Snapshot::pool[tid][newmarked] = new Snapshot(tmp, newmarked);
        }
    }
    return Snapshot::pool.find(tid)->second.find(newmarked)->second;
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
