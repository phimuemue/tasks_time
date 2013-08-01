#ifndef TIKZEXPORTER_H
#define TIKZEXPORTER_H

#include "exporter.h"

class TikzExporter : public Exporter {
    private:
    protected:
        // routines to draw one single snapshot properly
        unsigned int get_subtree_width(const task_id,
                const map<task_id, vector<task_id>>&) const;
        void compute_level_widths(const Snapshot* s,
                map<unsigned int, float>& level_count,
                map<Snapshot*, bool>& done,
                unsigned int depth) const;

        virtual void export_single_snapshot_internal(ostream& output,
                const Snapshot& s,
                const task_id t,
                const map<task_id, vector<task_id>>& rt,
                const unsigned int depth,
                const float leftoffset) const = 0;
        void export_single_snaphot(ostream& output, const Snapshot* s) const;
        void tikz_draw_node(const Snapshot* s,
                const Snapshot* orig,
                ostream& output,
                map<Snapshot*, unsigned int>& consec_num) const ;
        // routines to draw DAG of snaps
        string tikz_node_name(const Snapshot* s) const;
        virtual void tikz_dag_by_levels(const Snapshot* s,
                map<unsigned int, 
                vector<Snapshot*>>& levels,
                unsigned int depth,
                map<Snapshot*, unsigned int>& consec_num
                ) const;
        virtual void tikz_string_dag_compact_internal(const Snapshot* s,
                ostream& output,
                map<Snapshot*, string>& names,
                map<Snapshot*, unsigned int>& consec_num,
                map<unsigned int, float>& level_count,
                myfloat probability,
                unsigned int task_count_limit,
                bool first,
                unsigned int depth) const ;

    public:
        // adjustment variables for TikZ plots
        bool show_reaching_probabilities;
        bool show_probabilities;
        bool show_expectancy;
        unsigned int task_count_limit;
        float level_distance;
        float sibling_distance;
        bool horizontal;

        TikzExporter(bool se=true,
                bool sp=true,
                bool srp=true,
                unsigned int tcl=0);
        ~TikzExporter();
        virtual void export_snapshot_dag(ostream& output, const Snapshot* s) const;
};

#endif
