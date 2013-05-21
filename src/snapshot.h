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
    // TODO: I dont want no friends!!!
    friend class Probability_Computer;
    friend class TikzExporter;
    friend class DagviewExporter;
    friend class SimpleExporter;
    private:
        // snapshots are organized in a pool (no duplicates)
        static map<snapshot_id, Snapshot*> pool;

        vector<task_id> marked;
        Intree intree;

        // TODO: combine successors and successor_probs into 1 vector!
        vector<Snapshot*> successors;
        vector<myfloat> successor_probs;

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

        myfloat expected_runtime() const;
        friend ostream& operator<<(ostream& os, const Snapshot& s);

        string markedstring();

        static void clear_pool();
};

#endif
