#include "tikzexporter.h"

TikzExporter::TikzExporter(bool se, bool sp, bool srp, unsigned int tcl) :
    show_reaching_probabilities(srp),
    show_probabilities(sp), 
    show_expectancy(se),
    task_count_limit(tcl),
    horizontal(false)
{
}

TikzExporter::~TikzExporter(){
}

unsigned int TikzExporter::get_subtree_width(const task_id tid,
        const map<task_id,vector<task_id>>& rt) const {
    unsigned int result = 0;
    for(auto it=rt.find(tid)->second.begin(); it!=rt.find(tid)->second.end(); ++it){
        result = result + get_subtree_width(*it, rt);
    }
    result = max(result, 1u);
    return result;
}

void TikzExporter::export_single_snapshot_internal(ostream& output,
        const Snapshot& s,
        const task_id t,
        const map<task_id, vector<task_id>>& rt,
        const unsigned int depth,
        const float leftoffset) const{
    const float mywidth = 1.5f;
    const float myheight = 1.5f;
    const vector<task_id>& marked = s.marked;
    output << "\\node[";
    output << "circle, scale=0.75, fill";
    if(marked.size() == 0 || find(marked.begin(), marked.end(), t) != marked.end()){
        output << ", task_scheduled";
    }
    float complete_width=get_subtree_width(t, rt);
    output << "] (tid" << t << ") at (" 
        << mywidth * (leftoffset + 0.5f * complete_width) 
        << "," << myheight * depth << "){};" << endl;
    float cur_leftoffset = leftoffset;
    // draw "children"
    for(auto it = rt.at(t).begin(); it!=rt.at(t).end(); ++it){
        export_single_snapshot_internal(output, s, *it, rt, depth + 1, cur_leftoffset);
        cur_leftoffset += get_subtree_width(*it, rt);
    }
    // draw arrows from children
    for(auto it = rt.at(t).begin(); it!=rt.at(t).end(); ++it){
        output << "\\draw[](tid" << t << ") -- (tid" << *it << ");" << endl;
    }
}

void TikzExporter::export_single_snaphot(ostream& output, const Snapshot* s) const{
    map<task_id, vector<task_id>> rt;
    s->intree.get_reverse_tree(rt);
    export_single_snapshot_internal(output, 
            *s,
            0, 
            rt, 
            1, 
            0);
}

void TikzExporter::compute_level_widths(const Snapshot* s,
        map<unsigned int, float>& level_count,
        map<Snapshot*, bool>& done,
        unsigned int depth) const {
    if(done.find(const_cast<Snapshot*>(s)) == done.end()){
        done[const_cast<Snapshot*>(s)] = true;
        level_count[depth] = level_count[depth] + s->intree.get_max_width() * 1.5f + 2;
    }
    for(auto it=s->Successors.begin(); it!=s->Successors.end(); ++it){
        compute_level_widths(*it, level_count, done, depth+1);
    }
}


void TikzExporter::tikz_dag_by_levels(const Snapshot* s,
        map<unsigned int, vector<Snapshot*>>& levels,
        unsigned int depth,
        map<Snapshot*, unsigned int>& consec_num
        ) const {
    if(find(levels[depth].begin(), levels[depth].end(), const_cast<Snapshot*>(s)) == levels[depth].end()){
        // sorry for const_cast, TODO: can this be done better?
        Snapshot* s_tmp = const_cast<Snapshot*>(const_cast<Snapshot*>(s));
        levels[depth].push_back(s_tmp);
        if(consec_num.find(s_tmp)==consec_num.end()){
            consec_num[s_tmp] = consec_num.size();
        }
    }
    for_each(s->Successors.begin(), s->Successors.end(),
            [&](const Snapshot* x){
                tikz_dag_by_levels(x, levels, depth+1, consec_num);
            }
            );
}

void TikzExporter::tikz_string_dag_compact_internal(const Snapshot* s,
        ostream& output,
        map<Snapshot*, string>& names,
        map<Snapshot*, unsigned int>& consec_num,
        map<unsigned int, float>& level_count,
        myfloat probability,
        unsigned int task_count_limit,
        bool first,
        unsigned int depth) const {
    if(first){
        map<unsigned int, vector<Snapshot*>> levels;
        tikz_dag_by_levels(s, levels, 1, consec_num);
        for(auto& it : levels){
            sort(it.second.begin(), it.second.end(),
                [](const Snapshot* a, const Snapshot* b) -> bool {
                    map<task_id, task_id> iso;
                    tree_id ta, tb;
                    Intree::canonical_intree(
                        a->intree, a->marked, iso, ta);
                    Intree::canonical_intree(
                        b->intree, b->marked, iso, tb);
                    return ta < tb;
                }
            );
        }
        // draw all snaps
        for(unsigned int l=1; l<s->intree.count_tasks()+2-task_count_limit; ++l){
            output << "\\begin{scope}[yshift=\\leveltop"; 
            for(unsigned int i=0; i<l; ++i){
                output << "I";
            }
            output << " cm";
            if(horizontal){
                output << ", anchor = center";
            }
            output << "]" << endl;
            output << "\\matrix (line" << l << ")";
            if(horizontal){
                output << "[row sep=";
            }
            else {
                output << "[column sep=";
            }
            output << sibling_distance << "cm] {" << endl;
            for(auto it=levels[l].begin(); it!=levels[l].end(); ++it){
                tikz_draw_node(*it, 
                        s,
                        output,
                        consec_num);
                names[*it] = tikz_node_name(*it);
                if(it!=levels[l].end()){
                    if(horizontal){
                        output << " \\\\ " << endl;
                    }
                    else{
                        output << " & " << endl;
                    }
                }
            }
            output << "\\\\" << endl;
            output << "};" << endl;
            output << "\\end{scope}" << endl;
        }
        // connect (we have to draw probabilities seperately!)
        for(unsigned int l=1; l<s->intree.count_tasks()+1-task_count_limit; ++l){
            for(auto it=levels[l].begin(); it!=levels[l].end(); ++it){
                for(auto sit:(*it)->Successors){
                    output << "\\draw ("
                        << tikz_node_name(*it);
                    if(horizontal){
                        output << ".east) -- ";
                    }
                    else{
                        output << ".south) -- ";
                    }
                    output << "(" << names[sit];
                    if(horizontal){
                        output << ".west);" << endl;
                    }
                    else{
                        output << ".north);" << endl;
                    }
                }
            }
        }
    }
    
    if(s->intree.count_tasks() < task_count_limit){
        return;
    }
    if(names.find(const_cast<Snapshot*>(s)) == names.end()){
        // draw current snapshot at proper position
        // connect (we have to draw probabilities seperately!)
        if(s->intree.count_tasks() > task_count_limit){
            for(auto it : s->Successors){
                output << "\\draw ("
                    << tikz_node_name(s)
                    << ".south) -- "
                    << "(" 
                    << names[it] << ".north);" << endl;
            }
        }
    }
}

void TikzExporter::tikz_draw_node(const Snapshot* s,
        const Snapshot* orig,
        ostream& output,
        map<Snapshot*, unsigned int>& consec_num) const {
    unsigned partindex = 0;
    vector<string> tikz_partnames = {
        "two", "three", "four", "five"
    };
    string tikz_this_nodes_name = tikz_node_name(s);
    output << "\\node[draw=black, rectangle split,  rectangle split parts=" 
        << (int)show_expectancy +
           (int)show_probabilities +
           (int)show_reaching_probabilities+1;
    output << "] (" << tikz_this_nodes_name << ")";
    output << "{" << endl;
    if(show_reaching_probabilities){
        output << "\\footnotesize{" 
            << orig->get_reaching_probability(s) * 100
            << "}" << endl;
        output << "\\nodepart{" 
            << tikz_partnames[partindex++] << "}" << endl;
    }
    output << "\\begin{tikzpicture}[scale=.2]" << endl;
    export_single_snaphot(output, s);
    output << "\\end{tikzpicture}" << endl;
    // draw expected value
    if(show_expectancy){
        output << "\\nodepart{" << tikz_partnames[partindex++] << "}" << endl
            << "\\footnotesize{"
            << s->expected_runtime() << "}" << endl;
    }
    // draw probabilities
    if(show_probabilities){
        vector<pair<Snapshot*,myfloat>> successor_probs_in_order;
        for(auto sit : s->SuccessorProbabilities){
            successor_probs_in_order.push_back(
                    pair<Snapshot*, myfloat>(sit.get<0>(), sit.get<1>())
                    );
        }
        sort(successor_probs_in_order.begin(), successor_probs_in_order.end(),
                [&](const pair<Snapshot*,myfloat>& a, const pair<Snapshot*,myfloat>& b) -> bool {
                return consec_num[a.first] < consec_num[b.first];
                }
            );
        output << "\\nodepart{" << tikz_partnames[partindex++] << "}" << endl
            << "\\footnotesize{$";
        auto old_precision = output.precision();
        output << setprecision(2);
        for(auto pit=successor_probs_in_order.begin(); pit!=successor_probs_in_order.end(); ++pit){
            output << (pit->second < 1 ? (pit->second)*100 : pit->second);
            if(next(pit) != successor_probs_in_order.end()){
                output << "\\:";
            }
        }
        output << setprecision(old_precision);
        output << "$}" << endl;
    }
    output << "};" << endl;
}

string TikzExporter::tikz_node_name(const Snapshot* s) const {
    ostringstream tikz_nn;
    tikz_nn << "sn" << s;
    return tikz_nn.str();
}


void TikzExporter::export_snapshot_dag(ostream& output, const Snapshot* s) const{
    map<Snapshot*, string> positions;
    map<unsigned int, float> level_count;
    map<Snapshot*, unsigned int> consec_num;
    map<Snapshot*, bool> lv_done;
    compute_level_widths(s, level_count, lv_done, 1);
    for_each(level_count.begin(), level_count.end(),
            [](pair<const unsigned int, float>& a) {
                a.second = a.second * -0.5f;
            }
            );
    for(unsigned int l=1; l<s->intree.count_tasks()+1; ++l){
        output << "\\renewcommand{\\leveltop"; 
        for(unsigned int i=0; i<l; ++i){
            output << "I";
        }
        output << "}" << "{-" << level_distance << "cm + \\leveltop";
        for(unsigned int i=1; i<l; ++i){
            output << "I";
        }
        output << "}" << endl;
    }
    output << "\\begin{tikzpicture}[scale=.2, anchor=south";
    if(horizontal){
        output << ", rotate=90";
    }
    output << "]" << endl;
    tikz_string_dag_compact_internal(s, 
            output, 
            positions,
            consec_num,
            level_count,
            1,
            task_count_limit,
            true,
            1);
    output << "\\end{tikzpicture}" << endl;
    output << endl;
    output << "%%% Local Variables:" << endl;
    output << "%%% TeX-master: \"thesis/thesis.tex\"" << endl;
    output << "%%% End: " << endl;
}

