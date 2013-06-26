#include "newtikzexporter.h"

TikzExporter2::TikzExporter2(bool se,
        bool sp,
        bool srp,
        unsigned int tcl) : 
    show_reaching_probabilities(srp),
    show_probabilities(sp),
    show_expectancy(se),
    task_count_limit(tcl),
    horizontal(false)
{
}

TikzExporter2::~TikzExporter2(){
}

void TikzExporter2::generate_tikz_nodes(const Snapshot* s,
        const Snapshot* orig,
        map<const Snapshot*, TikzNode*>& target) const {
    if(target.find(s) != target.end()){
        return;
    }
    for(auto it : s->Successors){
        generate_tikz_nodes(it, orig, target);
    }
    TNSucs tikz_sucs;
    for(auto it : s->SuccessorProbabilities){
        tikz_sucs.push_back(TNSuc(target[it.get<0>()],it.get<1>()));
    }
    target[s] = new TikzNode(s, orig->get_reaching_probability(s), tikz_sucs);
}

void TikzExporter2::tikz_dag_by_levels(const TikzNode* s,
        map<unsigned int, vector<const TikzNode*>>& levels,
        unsigned int depth,
        const map<const Snapshot*, TikzNode*>& tikz_nodes,
        map<const TikzNode*, TikzNode*>& tikz_representants,
        map<const TikzNode, unsigned int>& consec_num
        ) const {
    for(auto suc : s->successors){
        tikz_dag_by_levels(
                suc.first,
                levels,
                depth+1,
                tikz_nodes,
                tikz_representants,
                consec_num);
    }
    if(consec_num.find(*s)==consec_num.end()){
        levels[depth].push_back(s);
        consec_num[*s] = consec_num.size();
        tikz_representants[s] = const_cast<TikzNode*>(s);
    }
}

void TikzExporter2::consolidate_levels(map<unsigned int, vector<const TikzNode*>>& levels) const{
    // this can be used in inherited classes to modify the level structure!
    return;
}

void TikzExporter2::export_snapshot_dag(ostream& output, const Snapshot* s) const {
    map<const Snapshot*, TikzNode*> tikz_nodes;
    generate_tikz_nodes(s, s, tikz_nodes);
    assert(tikz_nodes.size() == s->count_snapshots_in_dag());
    map<const TikzNode, unsigned int> consec_num;
    map<unsigned int, vector<const TikzNode*>> levels;
    map<const TikzNode*, TikzNode*> tikz_representants;
    for(auto it : tikz_nodes){
        tikz_representants[it.second] = it.second;
    }
    tikz_dag_by_levels(tikz_nodes[s], levels, 1, tikz_nodes, tikz_representants, consec_num);
    consolidate_levels(levels);
    for(auto& it : levels){
        sort(it.second.begin(), it.second.end(),
                [](const TikzNode* a, const TikzNode* b) -> bool {
                map<task_id, task_id> iso;
                tree_id ta, tb;
                Intree::canonical_intree(
                    a->snapshot->intree, a->snapshot->marked, iso, ta);
                Intree::canonical_intree(
                    b->snapshot->intree, b->snapshot->marked, iso, tb);
                return ta < tb;
                }
            );
    }
    // export to TikZ
    export_snapshot_dag_begin(output, s);
    map<const TikzNode*, string> names;
    for(auto& tn : tikz_nodes){
        names[tn.second] = tikz_node_name(tn.second->snapshot);
    }
    tikz_string_dag_compact_internal(tikz_nodes[s],
            output,
            tikz_representants,
            tikz_nodes,
            levels,
            names,
            consec_num,
            (myfloat)1,
            task_count_limit,
            true,
            0);
    export_snapshot_dag_end(output, s);
}

void TikzExporter2::tikz_draw_node(const TikzNode* s,
        const Snapshot* orig,
        ostream& output,
        map<const Snapshot*, TikzNode*>& tikz_nodes,
        map<const TikzNode, unsigned int>& consec_num) const {
    unsigned partindex = 0;
    vector<string> tikz_partnames = {
        "two", "three", "four", "five"
    };
    string tikz_this_nodes_name = tikz_node_name(s->snapshot);
    output << "\\node[draw=black, rectangle split,  rectangle split parts=" 
        << (int)show_expectancy +
           (int)show_probabilities +
           (int)show_reaching_probabilities+1;
    output << "] (" << tikz_this_nodes_name << ")";
    output << "{" << endl;
    if(show_reaching_probabilities){
        output << "\\footnotesize{" 
            << s->reaching_prob * 100
            // << orig->get_reaching_probability(s->snapshot) * 100
            << "}" << endl;
        output << "\\nodepart{" 
            << tikz_partnames[partindex++] << "}" << endl;
    }
    output << "\\begin{tikzpicture}[scale=.2";
    output << "]" << endl;
    export_single_snaphot(output, s->snapshot);
    output << "\\end{tikzpicture}" << endl;
    // draw expected value
    if(show_expectancy){
        output << "\\nodepart{" << tikz_partnames[partindex++] << "}" << endl
            << "\\footnotesize{"
            << s->snapshot->expected_runtime() << "}" << endl;
    }
    // draw probabilities
    if(show_probabilities){
        vector<pair<TikzNode*,myfloat>> successor_probs_in_order;
        //for(auto sit : s->snapshot->SuccessorProbabilities)
        for(auto sit : s->successors)
        {
            successor_probs_in_order.push_back(
                    pair<TikzNode*, myfloat>(sit.first, sit.second)
                    );
        }
        sort(successor_probs_in_order.begin(), successor_probs_in_order.end(),
                [](const pair<TikzNode*,myfloat>& a, const pair<TikzNode*,myfloat>& b) -> bool {
                map<task_id, task_id> iso;
                tree_id ta, tb;
                Intree::canonical_intree(
                    a.first->snapshot->intree, a.first->snapshot->marked, iso, ta);
                Intree::canonical_intree(
                    b.first->snapshot->intree, b.first->snapshot->marked, iso, tb);
                return ta < tb;
                }
                // The following does not coincide with levels
                // [&](const pair<TikzNode*,myfloat>& a, const pair<TikzNode*,myfloat>& b) -> bool {
                // return consec_num[*a.first] < consec_num[*b.first];
                // }
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

void TikzExporter2::tikz_string_dag_compact_internal(const TikzNode* s,
        ostream& output,
        map<const TikzNode*, TikzNode*>& tikz_representants,
        map<const Snapshot*, TikzNode*>& tikz_nodes,
        map<unsigned int, vector<const TikzNode*>>& levels,
        map<const TikzNode*, string>& names,
        map<const TikzNode, unsigned int>& consec_num,
        myfloat probability,
        unsigned int task_count_limit,
        bool first,
        unsigned int depth) const {
    if(first){
        // draw all snaps
        for(unsigned int l=1; l<s->snapshot->intree.count_tasks()+2-task_count_limit; ++l){
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
                        s->snapshot,
                        output,
                        tikz_nodes,
                        consec_num);
                names[*it] = tikz_node_name((*it)->snapshot);
                if(it!=levels[l].end()){
                    if(horizontal){
                        output << " \\\\ " << endl;
                    }
                    else{
                        output << " & " << endl;
                    }
                }
            }
            output << "\\\\" << endl << "};" << endl;
            output << "\\end{scope}" << endl;
        }
        // connect (we have to draw probabilities seperately!)
        for(unsigned int l=1; l<s->snapshot->intree.count_tasks()+1-task_count_limit; ++l){
            for(auto it=levels[l].begin(); it!=levels[l].end(); ++it){
                for(auto sit:(*it)->successors){
                    names[sit.first] = tikz_node_name(sit.first->snapshot);
                    output << "\\draw ("
                        << tikz_node_name((*it)->snapshot);
                    if(horizontal){
                        output << ".east) -- ";
                    }
                    else{
                        output << ".south) -- ";
                    }
                    output << "(" << tikz_node_name((sit.first)->snapshot);
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

    if(s->snapshot->intree.count_tasks() < task_count_limit){
        return;
    }
}

void TikzExporter2::export_snapshot_dag_begin(ostream& output, const Snapshot* s) const{
    map<Snapshot*, string> positions;
    map<unsigned int, float> level_count;
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
}


void TikzExporter2::export_snapshot_dag_end(ostream& output, const Snapshot* s) const{
    output << "\\end{tikzpicture}" << endl;
    output << endl;
    output << "%%% Local Variables:" << endl;
    output << "%%% TeX-master: \"thesis/thesis.tex\"" << endl;
    output << "%%% End: " << endl;
}

void TikzExporter2::merge_tikz_nodes(map<unsigned int, vector<const TikzNode*>>& levels,
        unsigned int l,
        const TikzNode* a,
        const TikzNode* b) const {
    assert(find(levels[l].begin(), levels[l].end(), a)!=levels[l].end());
    assert(find(levels[l].begin(), levels[l].end(), b)!=levels[l].end());
    if(a->snapshot->expected_runtime() != b->snapshot->expected_runtime()){
        cerr << "Warning: Snapshots to be merged have different run times." << endl;
    }
    TikzExporter2::TNSucs combined_sucs(a->successors);
    for(auto it : b->successors){
        for(auto& f : combined_sucs){
            if(f.first == it.first){
                f.second = f.second + it.second;
            }
        }
    }
    for(auto& it : combined_sucs){
        it.second = it.second / (myfloat)2;
    }
    // merge TikzNodes in current level
    // TODO: Need a delete after the new!
    TikzNode* combined_tn = new TikzNode(a->snapshot,
            a->reaching_prob + b->reaching_prob, combined_sucs
            );
    levels[l].erase(remove(levels[l].begin(), levels[l].end(), a), levels[l].end());
    levels[l].erase(remove(levels[l].begin(), levels[l].end(), b), levels[l].end());
    levels[l].push_back(combined_tn);
    
    // update successor in parent levels
    for(unsigned int idx = 0; idx < levels[l-1].size(); ++idx){
        TikzNode* tn = const_cast<TikzNode*>(levels[l-1][idx]);
        // first: substitute a and b by combined_tn
        for(unsigned int i = 0; i < tn->successors.size(); ++i){
            if(tn->successors[i].first == a || tn->successors[i].first == b){
                tn->successors[i].first = combined_tn;
            }
        }
        for(unsigned int i = 0; i < tn->successors.size(); ++i){
            for(unsigned int j = i+1; j < tn->successors.size(); ++j){
                // we have to properly decide which one we want to keep
                if(tn->successors[i].first == combined_tn && tn->successors[j].first == combined_tn){
                    tn->successors[i].first = combined_tn;
                    tn->successors[i].second += tn->successors[j].second;
                    tn->successors.erase(tn->successors.begin() + j);
                }
            }
        }
    }
}
