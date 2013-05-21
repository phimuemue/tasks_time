#ifndef TIKZEXPORTER_H
#define TIKZEXPORTER_H

#include "exporter.h"

class TikzExporter : public Exporter {
    private:
        bool show_probabilities;
        bool show_expectancy;
        unsigned int task_count_limit;
    protected:
        // routines to draw one single snapshot properly
        unsigned int get_subtree_width(const task_id,
                const map<task_id, vector<task_id>>&) const;
        void compute_level_widths(const Snapshot* s,
                map<unsigned int, float>& level_count,
                map<Snapshot*, bool>& done,
                unsigned int depth) const;

        void export_single_snapshot_internal(ostream& output,
                const Snapshot& s,
                const task_id t,
                const map<task_id, vector<task_id>>& rt,
                const unsigned int depth,
                const float leftoffset) const;
        void export_single_snaphot(ostream& output, const Snapshot* s) const;
        void tikz_draw_node(const Snapshot* s,
                ostream& output,
                bool show_expectancy,
                bool show_probabilities,
                map<Snapshot*, unsigned int>& consec_num,
                string right_of,
                float top) const ;
        // routines to draw DAG of snaps
        string tikz_node_name(const Snapshot* s) const;
        void tikz_dag_by_levels(const Snapshot* s,
                map<unsigned int, 
                vector<Snapshot*>>& levels,
                unsigned int depth,
                map<Snapshot*, unsigned int>& consec_num
                ) const;
        void tikz_string_dag_compact_internal(const Snapshot* s,
                ostream& output,
                map<Snapshot*, string>& names,
                map<Snapshot*, unsigned int>& consec_num,
                map<unsigned int, float>& level_count,
                myfloat probability,
                unsigned int task_count_limit,
                bool first,
                unsigned int depth,
                bool show_expectancy,
                bool show_probabilities) const ;

    public:
        TikzExporter();
        TikzExporter(bool se=true, bool sp=true, unsigned int tcl=0);
        ~TikzExporter();
        void export_snapshot_dag(ostream& output, const Snapshot* s) const;
};

#endif
