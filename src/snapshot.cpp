#include "snapshot.h"

Snapshot::Snapshot(Intree& t) :
    intree(t)
{

}

Snapshot::Snapshot(Intree& t, vector<task_id> m) :
    marked(m),
    intree(t)
{

}

void Snapshot::get_successors(const Scheduler& scheduler){
    // we only want to compute the successors once
    if(successors.size()>0)
        return;
    if(intree.count_tasks()==1)
        return;
    vector<myfloat> finish_probs;
    Probability_Computer().compute_finish_probs(*this,
        finish_probs);
    assert(finish_probs.size()==marked.size());
    // then, for each finished threads, compute all possible successors
    auto finish_prob_it = finish_probs.begin();
    for(auto it = marked.begin(); it!=marked.end(); ++it, ++finish_prob_it){
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
                Snapshot news(tmp, newmarked);
                successors.push_back(news);
                successor_probs.push_back(*finish_prob_it * raw_sucs[i].second);
            }
        }
        else {
            vector<task_id> newmarked(marked);
            newmarked.erase(remove_if(newmarked.begin(), newmarked.end(),
                        [it](const task_id& a){
                        return a==*it;
                        }), newmarked.end());
            Snapshot news(tmp, newmarked);
            successors.push_back(news);
            successor_probs.push_back(*finish_prob_it);
        }
    }
}

void Snapshot::compile_snapshot_dag(const Scheduler& scheduler){
    get_successors(scheduler);
    for(unsigned int i=0; i<successors.size(); ++i){
        successors[i].compile_snapshot_dag(scheduler);
    }
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
        suc_expected_runtimes[i] = successors[i].expected_runtime();
    }
    for(unsigned int i=0; i<successors.size(); ++i){
        result += successor_probs[i] * suc_expected_runtimes[i];
    }
    return result;
}

string Snapshot::tikz_string_internal(const task_id t, 
        map<task_id,vector<task_id>>& rt) const {
    stringstream output;
    output << "node[fill";
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
        output << "{" << tikz_string_internal(*it, rt) << "}" << endl;
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

string Snapshot::tikz_string_dag(bool first){
    stringstream output;
    if(first){
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
    for(unsigned int i=0; i < intree.edges.size(); ++i){
        output << "\\tikzstyle{level " << i+1 << "} = " <<
            "[sibling distance = " << 8./(i+1) << "cm, " <<
            "level distance = " << 2. - i/5. << "cm" << "]" << endl;
    }   
    output << tikz_string() << endl;
    output << "\\end{tikzpicture}" << endl;
    output << "}" << endl;
    output << "\\usebox\\nodebox" << endl;
    output << "}" << endl;
    for(auto it=successors.begin(); it!=successors.end(); ++it){
        output << "child";
        output << "{" << it->tikz_string_dag(false) << "}";
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
        it->print_snapshot_dag(depth+1);
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
