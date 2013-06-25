#include "chainsideexporter.h"

void ChainSideExporter::export_single_snapshot_internal(ostream& output,
        const Snapshot& s,
        const task_id t,
        const map<task_id, vector<task_id>>& rt,
        const unsigned int depth,
        const float leftoffset) const {
    output << "\\node[rectangle, scale=0.75] at (0, 0) {";
    output << "$" <<
        s.intree.longest_chain_length() <<
        "/" <<
        s.intree.count_tasks() - s.intree.longest_chain_length() <<
        "$";
    output << "};" << endl;
}

bool ChainSideExporter::same_chain_side(
        const TikzExporter2::TikzNode* a,
        const TikzExporter2::TikzNode *b)
    const{
    pair<unsigned int, unsigned int> A(a->snapshot->intree.count_tasks(), a->snapshot->intree.longest_chain_length());
    pair<unsigned int, unsigned int> B(b->snapshot->intree.count_tasks(), b->snapshot->intree.longest_chain_length());
    return A==B;
}

void ChainSideExporter::consolidate_levels(map<unsigned int, vector<const TikzNode*>>& levels) const {
    for(unsigned int l = levels.size()-1; !(l > levels.size()-1); --l)
    // for(unsigned int l = 0; l < levels.size(); ++l)
    {
        bool something_done = true;
        do {
            something_done = false;
            for(unsigned int i=0; i<levels[l].size(); ++i){
                for(unsigned int j=i+1; j<levels[l].size(); ++j){
                    if(same_chain_side(levels[l][i], levels[l][j])){
                        merge_tikz_nodes(levels, l, levels[l][i], levels[l][j]);
                        something_done = true;
                    }
                }
            }
        } while (something_done == true);
    }
}
