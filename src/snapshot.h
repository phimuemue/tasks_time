#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#include<vector>
#include<assert.h>

#include "intree.h"
#include "scheduler.h"

using namespace std;

class Scheduler;

class Snapshot {
    private:
        // TODO: implement scheduler class
        static Scheduler scheduler;
        vector<task_id> marked;
        Intree intree;
        vector<Snapshot> successors;
        vector<myfloat> probabilities;
        unsigned int num_processors; // should this be in scheduler?
    public:
        Snapshot(Intree& t);
        void get_successors();
        void compile_snapshot_dag();
        myfloat expected_runtime();

        friend ostream& operator<<(ostream& os, const Snapshot& s);
};

#endif
