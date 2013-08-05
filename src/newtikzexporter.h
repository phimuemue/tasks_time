#ifndef NEWTIKZEXPORTER_H
#define NEWTIKZEXPORTER_H

#include "tikzexporter.h"

class TikzExporter2 : public TikzExporter {
    protected:
        class TikzNode;
        typedef pair<TikzNode*, myfloat> TNSuc;
        typedef vector<TNSuc> TNSucs;
        class TikzNode {
            public:
                const Snapshot* snapshot;
                myfloat reaching_prob;
                TNSucs successors;
                TikzNode(const Snapshot* s, myfloat rp, TNSucs sucs) :
                    snapshot(s), reaching_prob(rp), successors(sucs)
                {
                }
                bool operator< (const TikzNode& b) const {
                    return snapshot < b.snapshot;
                }
        };
        // routines to draw one single snapshot properly are in TikzExporter

        // routines to draw DAG of snaps
        virtual void export_single_snapshot_internal(ostream& output,
                const Snapshot& s,
                const task_id t,
                const map<task_id, vector<task_id>>& rt,
                const unsigned int depth,
                const float leftoffset) const;
        void generate_tikz_nodes(const Snapshot* s,
                const Snapshot* orig,
                map<const Snapshot*, TikzNode*>& target) const ;
        void tikz_dag_by_levels(const TikzNode* s,
                map<unsigned int, vector<const TikzNode*>>& levels,
                unsigned int depth,
                const map<const Snapshot*, TikzNode*>& tikz_nodes,
                map<const TikzNode*, TikzNode*>& tikz_representants,
                map<const TikzNode, unsigned int>& consec_num
                ) const;
        void tikz_string_dag_compact_internal(const TikzNode* s,
                ostream& output,
                map<const TikzNode*, TikzNode*>& tikz_representants,
                map<const Snapshot*, TikzNode*>& tikz_nodes,
                map<unsigned int, vector<const TikzNode*>>& levels,
                map<const TikzNode*, string>& names,
                map<const TikzNode, unsigned int>& consec_num,
                myfloat probability,
                unsigned int task_count_limit,
                bool first,
                unsigned int depth) const ;
        void tikz_draw_node(const TikzNode* s,
                const Snapshot* orig,
                ostream& output,
                map<const Snapshot*, TikzNode*>& tikz_nodes,
                map<const TikzNode, unsigned int>& consec_num) const ;
        void export_snapshot_dag_begin(ostream& output, const Snapshot* s) const;
        void export_snapshot_dag_end(ostream& output, const Snapshot* s) const;
        void merge_tikz_nodes(map<unsigned int, vector<const TikzNode*>>& levels,
                unsigned int level,
                const TikzNode* a,
                const TikzNode* b) const;
        virtual void consolidate_levels(map<unsigned int, vector<const TikzNode*>>& levels) const ;
    public:
        // adjustment variables for TikZ plots
        bool show_reaching_probabilities;
        bool show_probabilities;
        bool show_expectancy;
        unsigned int task_count_limit;
        float level_distance;
        float sibling_distance;
        bool horizontal;
        bool show_labels;

        TikzExporter2(bool se=true,
                bool sp=true,
                bool srp=true,
                unsigned int tcl=0);
        ~TikzExporter2();
        virtual void export_snapshot_dag(ostream& output, const Snapshot* s) const;
};

#endif
