#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include<vector>
#include<assert.h>
#include<string>
#include<queue>
#include<map>
#include<algorithm>
#include<sstream>
#include<iomanip>

#include "config.h"
#include "intree.h"
#include "scheduler.h"
#include "leafscheduler.h"
#include "probability.h"

using namespace std;

class Snapshot {
    friend class Probability_Computer;
    private:
        vector<task_id> marked;
        Intree intree;
        vector<Snapshot*> successors;
        vector<myfloat> successor_probs;
        vector<myfloat> probabilities;
        string tikz_string_internal_qtree(const task_id,
                map<task_id,vector<task_id>>&, bool = true) const;
        unsigned int get_subtree_width(const task_id,
                map<task_id,vector<task_id>>&) const;
        string tikz_string_internal(const task_id,
                map<task_id,vector<task_id>>&, 
                unsigned int depth=0,
                float leftoffset=0) const;
        string tikz_node_name() const;
        void tikz_draw_node(ostream& output,
                bool show_expectancy,
                bool show_probabilities,
                map<Snapshot*, unsigned int>& consec_num,
                float left,
                float top) const;
        static map<snapshot_id, Snapshot*> pool;
        void compute_level_widths(map<unsigned int, float>& level_count,
                map<Snapshot*, bool>& done,
                unsigned int depth = 1);
        void dag_view_string_internal(ostringstream& oss,
                unsigned int task_count_limit=0,
                myfloat probability=(myfloat)1,
                unsigned int depth=0);
        unsigned int width_of_task(const task_id t, 
                map<task_id,vector<task_id>>& rt) const;
    public:
        Snapshot();
        Snapshot(const Snapshot& s);
        Snapshot(Intree& t);
        Snapshot(Intree& t, vector<task_id> m);
        ~Snapshot();
        static Snapshot* canonical_snapshot(const Snapshot& s);
        static Snapshot* canonical_snapshot(Intree& t, vector<task_id> m);

        void get_successors(const Scheduler& scheduler);
        void compile_snapshot_dag(const Scheduler& scheduler);
        size_t get_successor_count();

        string dag_view_string(unsigned int task_count_limit=0,
                unsigned int depth=0,
                myfloat probability=(myfloat)1);
        string tikz_string() const;
        string tikz_string_dag(unsigned int task_count_limit=0,
                bool first=true,
                unsigned int = 1);
        string tikz_string_dag_compact(unsigned int task_count_limit=0,
                bool show_expectancy=true,
                bool show_probabilities=true,
                bool first=true,
                unsigned int depth=1
                );
        void tikz_string_dag_compact_internal(ostringstream& output,
                map<Snapshot*, string>& names,
                map<Snapshot*, unsigned int>& consec_num,
                map<unsigned int, float>& levelcount,
                myfloat probability,
                unsigned int task_count_limit,
                bool first,
                unsigned int depth,
                bool show_expectancy,
                bool show_probabilities);


        myfloat expected_runtime() const;
        void print_snapshot_dag(int depth=0);
        friend ostream& operator<<(ostream& os, const Snapshot& s);

        string markedstring();

        static void clear_pool();
};

#endif
