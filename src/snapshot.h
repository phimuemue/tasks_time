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
    private:
        // snapshots are organized in a pool (no duplicates)
        static map<snapshot_id, Snapshot*> pool;

        // TODO: combine successors and successor_probs into 1 vector!
        vector<Snapshot*> successors;
        vector<myfloat> successor_probs;

    public:
        // TODO: it would be nice if the following were const
        // but then the default operator= is deleted.
        const vector<task_id> marked;
        const Intree intree;

        // Member spaces to offer nice iterators 
        // for probabilities and successors
        struct Successors {
            vector<Snapshot*>::const_iterator begin() const {
                return my_Snapshot->successors.begin();
            }
            vector<Snapshot*>::const_iterator end() const {
                return my_Snapshot->successors.end();
            }
            private:
            friend class Snapshot;
            Successors(Snapshot* s) : my_Snapshot(s) {}
            Snapshot* my_Snapshot;
        } Successors;
        struct SuccessorProbs {
            vector<myfloat>::const_iterator begin() const {
                return my_Snapshot->successor_probs.begin();
            }
            vector<myfloat>::const_iterator end() const {
                return my_Snapshot->successor_probs.end();
            }
            private:
            friend class Snapshot;
            SuccessorProbs(Snapshot* s) : my_Snapshot(s) {}
            Snapshot* my_Snapshot;
        } SuccessorProbs;

        Snapshot();
        Snapshot(const Snapshot& s);
        Snapshot(Intree& t);
        Snapshot(Intree& t, vector<task_id> m);
        ~Snapshot();

        static Snapshot* canonical_snapshot(const Snapshot& s);
        static Snapshot* canonical_snapshot(Intree& t, 
                vector<task_id> m);

        unsigned int count_tasks() const;

        void get_successors(const Scheduler& scheduler);
        void compile_snapshot_dag(const Scheduler& scheduler);
        size_t get_successor_count();

        myfloat expected_runtime() const;
        friend ostream& operator<<(ostream& os, const Snapshot& s);

        string markedstring();

        static void clear_pool();
};

#endif
