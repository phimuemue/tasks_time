#include "chainsideexporter.h"

void ChainSideExporter::export_single_snapshot_internal(ostream& output,
        const Snapshot& s,
        const task_id t,
        const map<task_id, vector<task_id>>& rt,
        const unsigned int depth,
        const float leftoffset) const {
    output << "\\node[circle, scale=0.75] at (0, 0) {";
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
    for(unsigned int l = 0; l<levels.size(); ++l){
    }
}
