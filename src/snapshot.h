#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include<vector>
#include<assert.h>
#include<string>
#include<queue>
#include<map>
#include<algorithm>
#include<sstream>

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
        string tikz_string_internal(const task_id,
                map<task_id,vector<task_id>>&, bool = true) const;
        static map<pair<tree_id,vector<task_id>>, Snapshot*> pool;
        void dag_view_string_internal(ostringstream& oss,
                unsigned int task_count_limit=0,
                myfloat probability=(myfloat)1,
                unsigned int depth=0);
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
        string tikz_string();
        string tikz_string_dag(unsigned int task_count_limit=0,
                bool first=true,
                unsigned int = 1);
        myfloat expected_runtime();
        void print_snapshot_dag(int depth=0);
        friend ostream& operator<<(ostream& os, const Snapshot& s);

        string markedstring();

        static void clear_pool();
};

#endif
