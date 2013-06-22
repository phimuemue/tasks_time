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
