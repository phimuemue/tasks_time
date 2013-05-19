#include "snapshot.h"

map<snapshot_id, Snapshot*> Snapshot::pool;

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

Snapshot::Snapshot(const Snapshot& s) :
    marked(s.marked),
    intree(s.intree)
    {
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
            cout << t << endl;
            for(auto it=m.begin(); it!=m.end(); ++it){
                cout << *it << endl;
            }
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

Snapshot* Snapshot::canonical_snapshot(const Snapshot& s){
    Intree intreecopy(s.intree);
    vector<task_id> mcopy(s.marked);
    return canonical_snapshot(intreecopy, mcopy);
}

Snapshot* Snapshot::canonical_snapshot(Intree& t, vector<task_id> m){
    vector<task_id> original_m(m);
#if USE_SIMPLE_OPENMP
    cout << "Warning! Using openmp with canonical snapshots!" << endl;
#endif

    map<task_id, task_id> isomorphism;
    tree_id tid;
    Intree tmp = Intree::canonical_intree(t, m, isomorphism, tid);

    // adjust m properly (i.e. always "lowest possible task" for 'iso-snap')
    transform(m.begin(), m.end(), m.begin(),
        [&](const task_id a) -> task_id {
            return isomorphism[a];
        }
    );
    m.erase(remove_if(m.begin(), m.end(),
        [&tmp](const task_id a) -> bool {
            return !tmp.contains_task(a);
        }), 
        m.end()
    );
    map<task_id, unsigned int> counts;
    for(auto it=m.begin(); it!=m.end(); ++it){
        if(*it!=0){
            counts[tmp.get_edge_from(*it).second.get_id()]++;
        }
    }
    map<task_id, vector<task_id>> predecessor_collection;
    for(auto it=counts.begin(); it!=counts.end(); ++it){
        tmp.get_predecessors(it->first, predecessor_collection[it->first]);
        // remove non-leaf tasks from predecessors
        predecessor_collection[it->first].erase(
            remove_if(predecessor_collection[it->first].begin(),
                predecessor_collection[it->first].end(),
                [&tmp](const task_id a) -> bool {
                    return !tmp.is_leaf(a);
                }
            )
            , predecessor_collection[it->first].end()
        );
        sort(predecessor_collection[it->first].begin(),
             predecessor_collection[it->first].end());
    }
    m.clear();
    for(auto it=counts.begin(); it!=counts.end(); ++it){
        for(auto ii=predecessor_collection[it->first].begin();
            ii != predecessor_collection[it->first].end();
            ++ii){
        }
        assert(predecessor_collection[it->first].size() >= it->second);
        for(unsigned int i=0; i<it->second; ++i){
            m.push_back(predecessor_collection[it->first][i]);
        }
    }

    // construct newmarked
    vector<task_id> newmarked;
    for(auto it=m.begin(); it!=m.end(); ++it){
        newmarked.push_back(*it);
    }
    sort(newmarked.begin(), newmarked.end());
    auto find_key = snapshot_id(tid, newmarked);
    auto correct_pool = 
        Snapshot::pool.find(find_key);
    if(correct_pool == Snapshot::pool.end()){
        Snapshot::pool[find_key] = new Snapshot(tmp, newmarked);
    }
    isomorphism.clear();
    tid = 0;
    Intree::canonical_intree(Snapshot::pool.find(find_key)->second->intree, newmarked, isomorphism, tid);
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

void Snapshot::dag_view_string_internal(ostringstream& output,
        unsigned int task_count_limit,
        myfloat probability,
        unsigned int depth){
    for(unsigned int i=0; i<depth; ++i){
        output << " ";
    }
    output << *this << " " << expected_runtime() << " " << probability << endl;
    if(intree.count_tasks() > task_count_limit){
        if(!intree.is_chain()){
            auto pit = successor_probs.begin();
            for(auto it = successors.begin(); it!=successors.end(); ++it, ++pit){
                (*it)->dag_view_string_internal(output, task_count_limit, *pit, depth+1);
            }
        }
    }
}

string Snapshot::dag_view_string(unsigned int task_count_limit, unsigned int depth, myfloat probability){
    ostringstream output;
    dag_view_string_internal(output, task_count_limit);
    return output.str();
}

unsigned int Snapshot::width_of_task(const task_id t, 
        map<task_id,vector<task_id>>& rt) const {
    // TODO: make it useful!
    unsigned int res = 0;
    res = rt[t].size();
    unsigned int max_sub=0;
    for(auto it=rt[t].begin(); it!=rt[t].end(); ++it){
        auto tmp = width_of_task(*it, rt);
        max_sub = max(tmp, max_sub);
    }
    return res + max_sub;
}

unsigned int Snapshot::get_subtree_width(const task_id tid,
        map<task_id,vector<task_id>>& rt) const {
    unsigned int result = 0;
    for(auto it=rt[tid].begin(); it!=rt[tid].end(); ++it){
        result = result + get_subtree_width(*it, rt);
    }
    result = max(result, 1u);
    return result;
}

string Snapshot::tikz_string_internal(const task_id t, 
        map<task_id,vector<task_id>>& rt, unsigned int depth, float leftoffset) const {
    stringstream output;
    const float mywidth = 1.5f;
    const float myheight = 1.5f;
    output << "\\node[";
    output << "circle, scale=0.75, fill";
    if(marked.size() == 0 || find(marked.begin(), marked.end(), t) != marked.end()){
        output << ", red";
    }
    float complete_width=get_subtree_width(t, rt);
    output << "] (tid" << t << ") at (" 
        << mywidth * (leftoffset + 0.5f * complete_width) 
        << "," << myheight * depth << "){};" << endl;
    float cur_leftoffset = leftoffset;
    // draw "children"
    for(auto it = rt[t].begin(); it!=rt[t].end(); ++it){
        output << tikz_string_internal(*it, rt, depth + 1, cur_leftoffset);
        cur_leftoffset += get_subtree_width(*it, rt);
    }
    // draw arrows from children
    for(auto it = rt[t].begin(); it!=rt[t].end(); ++it){
        output << "\\draw[](tid" << t << ") -- (tid" << *it << ");" << endl;
    }
    return output.str();
}

string Snapshot::tikz_string_internal_qtree(const task_id t, 
        map<task_id,vector<task_id>>& rt, bool first) const {
    cout << "Output via qtree for tikz currently not implemented" << endl;
    throw 1;
}

string Snapshot::tikz_string(){
    map<task_id,vector<task_id>> reverse_tree;
    intree.get_reverse_tree(reverse_tree);
    return tikz_string_internal(0, reverse_tree);
}

string Snapshot::tikz_string_dag(unsigned int task_count_limit,
        bool first,
        unsigned int depth){
    stringstream output;
    if(first){
        output << "% this tree has expected runtime of " << expected_runtime() << endl;
        output << "\\begin{tikzpicture}" << endl;
        for(unsigned int i=0; i < intree.edges.size(); ++i){
            output << "\\tikzstyle{level " << i+1 << "} = " <<
                "[sibling distance = " << 32./(i+1) << "cm, " <<
                "level distance = " << 2. - i/5. << "cm" << "]" << endl;
        }
        output << "\\";
    }
    output << "node[draw=black]{" << endl;
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
    if(intree.count_tasks() > task_count_limit){
        if(!intree.is_chain()){
            auto pit = successor_probs.begin();
            for(auto it=successors.begin(); it!=successors.end(); ++it, ++pit){
                output << "child";
                output << "{"; 
                output << (*it)->tikz_string_dag(task_count_limit, false, depth+1); 
                output << "edge from parent node [left] { " << *pit << "}" << endl;
                output << "}";
            }
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


string Snapshot::tikz_string_dag_compact(
        unsigned int task_count_limit,
        bool show_expectancy,
        bool show_probabilities,
        bool first,
        unsigned int depth
        ){
    ostringstream output;
    map<Snapshot*, string> positions;
    map<unsigned int, float> level_count;
    map<Snapshot*, bool> lv_done;
    compute_level_widths(level_count, lv_done, 1);
    for_each(level_count.begin(), level_count.end(),
            [](pair<const unsigned int, float>& a) {
                a.second = a.second * -0.5f;
            }
            );
    output << "\\begin{tikzpicture}[scale=.2, anchor=south west]" << endl;
    tikz_string_dag_compact_internal(output, 
            positions,
            level_count,
            1,
            task_count_limit,
            first,
            depth,
            show_expectancy,
            show_probabilities);
    output << "\\end{tikzpicture}" << endl;
    output << endl;
    output << "%%% Local Variables:" << endl;
    output << "%%% TeX-master: \"thesis/thesis.tex\"" << endl;
    output << "%%% End: " << endl;
    return output.str();
}

void Snapshot::compute_level_widths(map<unsigned int, float>& level_count,
        map<Snapshot*, bool>& done,
        unsigned int depth) {
    if(done.find(this) == done.end()){
        done[this] = true;
        level_count[depth] = level_count[depth] + intree.get_max_width() * 1.5f + 2;
    }
    for(auto it=successors.begin(); it!=successors.end(); ++it){
        (*it)->compute_level_widths(level_count, done, depth+1);
    }
}

void Snapshot::tikz_string_dag_compact_internal(ostringstream& output,
        map<Snapshot*, string>& names,
        map<unsigned int, float>& level_count,
        myfloat probability,
        unsigned int task_count_limit,
        bool first,
        unsigned int depth,
        bool show_expectancy,
        bool show_probabilities){
    vector<string> tikz_partnames = {
        "two", "three", "four", "five"
    };
    char partindex = 0;
    if(names.find(this) == names.end()){
        // draw current snapshot at proper position
        float width = intree.get_max_width() * 1.5f + 2;
        //width = 9;
        float height = 12.;
        ostringstream tikz_nn;
        tikz_nn << "sn" << this << "W" << int(level_count[depth]) << "";
        string tikz_node_name = tikz_nn.str();
        names[this] = tikz_node_name;
        output << "\\node[draw=black, rectangle split, rectangle split parts=" 
               << (int)show_expectancy+(int)show_probabilities+1
               << "] (" << tikz_node_name << ")"
               << " at (" << level_count[depth] << ", -" << depth*height << ") {" << endl;
        output << "\\begin{tikzpicture}[scale=.2]" << endl;
        output << tikz_string() << endl;
        output << "\\end{tikzpicture}" << endl;
        if(show_expectancy){
            output << "\\nodepart{" << tikz_partnames[partindex++] << "}" << endl
                << "\\footnotesize{"
                << expected_runtime() << "}" << endl;
        }
        if(show_probabilities){
            output << "\\nodepart{" << tikz_partnames[partindex++] << "}" << endl
                << "\\footnotesize{";
            auto old_precision = output.precision();
            output << setprecision(2);
            for(auto pit=successor_probs.begin(); pit!=successor_probs.end(); ++pit){
                output << *pit;
                if(next(pit) != successor_probs.end()){
                    output << " ";
                }
            }
            output << setprecision(old_precision);
            output << "}" << endl;
        }
        output << "};" << endl;
        // draw successors
        auto pit = successor_probs.begin();
        for(auto it=successors.begin(); it!=successors.end(); ++it, ++pit){
            (*it)->tikz_string_dag_compact_internal(output,
                names,
                level_count,
                *pit,
                task_count_limit,
                false,
                depth+1,
                show_expectancy,
                show_probabilities);
        }
        level_count[depth] += width;
        // connect (we have to draw probabilities seperately!)
        pit = successor_probs.begin();
        for(auto it=successors.begin(); it!=successors.end(); ++it, ++pit){
            output << "\\draw (" << tikz_node_name << ".south) -- "
                << "(" 
                << names[*it] << ".north);" << endl;
        }
        pit = successor_probs.begin();
#if 0
        for(auto it=successors.begin(); it!=successors.end(); ++it, ++pit){
            if (*pit != 1.f / successors.size()){
                auto old_precision = output.precision();
                output << "\\draw[draw opacity=0] (" << tikz_node_name << ".south) -- "
                    << "node[draw opacity=1, anchor=base, draw=black, fill=white]{\\footnotesize{" 
                    << setprecision(2) 
                    << *pit 
                    << "}}(" 
                    << names[*it] << ".north);" << endl;
                output << setprecision(old_precision);
            }
        }
#endif
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
