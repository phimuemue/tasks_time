#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include<vector>
#include<assert.h>
#include<omp.h>
#include<string>
#include<queue>
#include<map>
#include<algorithm>
#include<sstream>

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
        vector<Snapshot> successors;
        vector<myfloat> successor_probs;
        vector<myfloat> probabilities;
        string tikz_string_internal(const task_id,
                map<task_id,vector<task_id>>&, bool = true) const;
    public:
        Snapshot(Intree& t);
        Snapshot(Intree& t, vector<task_id> m);
        void get_successors(const Scheduler& scheduler);
        void compile_snapshot_dag(const Scheduler& scheduler);

        string tikz_string();
        string tikz_string_dag(bool first=true, unsigned int = 1);
        myfloat expected_runtime();
        void print_snapshot_dag(int depth=0);
        friend ostream& operator<<(ostream& os, const Snapshot& s);
};

#endif
