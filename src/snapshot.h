#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include<vector>
#include<assert.h>

#include "intree.h"
#include "scheduler.h"
#include "leafscheduler.h"

using namespace std;

class Snapshot {
    private:
        vector<task_id> marked;
        vector<myfloat> finish_probs;
        Intree intree;
        vector<Snapshot> successors;
        vector<myfloat> probabilities;
        unsigned int num_processors; // should this be in scheduler?
    public:
        Snapshot(Intree& t);
        Snapshot(Intree& t, vector<task_id> m);
        void get_successors(const Scheduler& scheduler);
        void compile_snapshot_dag(const Scheduler& scheduler);
        myfloat expected_runtime();
        void print_snapshot_dag(int depth=0);
        friend ostream& operator<<(ostream& os, const Snapshot& s);
};

#endif
