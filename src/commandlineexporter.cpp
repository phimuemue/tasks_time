#include "commandlineexporter.h"

void CommandLineExporter::export_snapshot_dag(ostream& output, const Snapshot* s) const {
    output << "Exporting snapshots to command line is not yet implemented. " << endl
        << "CommandLineExporter is maily useful to print trees!" << endl;
    assert(false);
}

void CommandLineExporter::export_tree(ostream& output, const Intree& t) const {
    // output << "Intree " << t << endl;
    Intree::Outtree outtree(t, vector<task_id>());
    vector<pair<Intree::Outtree*, unsigned int>> stack;
    stack.push_back(pair<Intree::Outtree*, unsigned int>(&outtree, 0));
    map<unsigned int, vector<task_id>> tasks_by_level;
    vector<string> lines(t.longest_chain_length());

    export_helper(tasks_by_level, lines, 0, 0, t, stack[0].first);
    
    for(auto l=lines.rbegin(); l!=lines.rend(); ++l){
        cout << *l << endl;
    }
}

void CommandLineExporter::export_helper(
        map<unsigned int, vector<task_id>>& tasks_by_level, 
        vector<string>& lines, 
        task_id tid,
        unsigned int level,
        const Intree& t,
        const Intree::Outtree* tmp) const {

        int numwidth = floor(log10(t.count_tasks())) + 1;
        cout << setw(numwidth);

        tasks_by_level[t.get_level(tid)].push_back(tid);

        //cout << "Processing " << tid << "(" << t.get_level(tid) << ", " << t.get_max_width(tid) << ")" << endl;
        auto l = t.get_level(tid);

        for(unsigned int sp = 0; sp < t.get_max_width(tid) * numwidth; ++sp){
            lines[l] = lines[l] + " ";
        }
        lines[l] = lines[l] + to_string(tid);
        if(!t.is_leaf(tid)){
            for(unsigned int sp = 0; sp < t.get_max_width(tid) * numwidth; ++sp){
                lines[l] = lines[l] + " ";
            }
        }
        if(t.is_leaf(tid)){
            for(unsigned int l2 = l+1; l2 < tasks_by_level.size(); ++l2){
                while(lines[l2].size() < lines[l].size()){
                    lines[l2] = lines[l2] + " ";
                }
            }
        }

        // cout << "Processed " << tid << endl;
        // for(auto line : lines){
        //     if(line.size() > 1){
        //         cout << line << "|" << endl;
        //     }
        // }

        // recursion!
        for(auto p : tmp->predecessors){
            export_helper(tasks_by_level, lines, p->id, level+1, t, p);
        }

        if(l>0){
            for(unsigned int l2 = lines.size() - 2; l2 >= l && l2 < NOTASK; --l2){
                while(lines[l2+1].size() > lines[l2].size()){
                    lines[l2] = lines[l2] + " ";
                }
            }
        }
        for(unsigned int l2 = l+1; l2 < lines.size(); ++l2){
            while(lines[l2].size() < lines[l].size()){
                lines[l2] = lines[l2] + " ";
            }
        }
}
