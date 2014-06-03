#include "profileexporter.h"

void ProfileExporter::export_single_snapshot_internal(ostream& output,
        const Snapshot& s,
        const task_id t,
        const map<task_id, vector<task_id>>& rt,
        const unsigned int depth,
        const float leftoffset) const {
    output << "\\node[rectangle, scale=0.75] at (0, 0) {";
    vector<unsigned int> const profile = s.intree.get_profile();
    output << "$";
    output << "\\profile{";
    // Tricky condition because of unsignedness!
    for(unsigned int i=profile.size()-1; i<profile.size(); --i){
        output << profile[i];
        if(i!=0){
            output << ", ";
        }
    }
    output << "}";
    output << "$";
    output << "};" << endl;
}

bool ProfileExporter::same_profile(
        const TikzExporter2::TikzNode* a,
        const TikzExporter2::TikzNode *b)
    const{
    return a->snapshot->intree.get_profile()==b->snapshot->intree.get_profile();
}

void ProfileExporter::consolidate_levels(map<unsigned int, vector<const TikzNode*>>& levels) const {
    for(unsigned int l = levels.size()-1; !(l > levels.size()-1); --l)
    {
        bool something_done = true;
        do {
            something_done = false;
            for(unsigned int i=0; i<levels[l].size(); ++i){
                for(unsigned int j=i+1; j<levels[l].size(); ++j){
                    if(same_profile(levels[l][i], levels[l][j])){
                        merge_tikz_nodes(levels, l, levels[l][i], levels[l][j]);
                        something_done = true;
                    }
                }
            }
        } while (something_done == true);
    }
}
