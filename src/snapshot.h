#ifndef SNAPSHOT_H
#define SNAPSHOT_H

#if PYTHON_TESTS
#include<Python.h>
#endif
#include<vector>
#include<assert.h>
#include<string>
#include<queue>
#include<map>
#include<algorithm>
#include<sstream>
#include<iomanip>
#include<boost/iterator/zip_iterator.hpp>

#include "config.h"
#include "intree.h"
#include "scheduler.h"
#include "leafscheduler.h"
#include "probability.h"

using namespace std;

class Snapshot {
    // TODO: I dont want no friends!!!
    friend class Probability_Computer;
    public:
        struct SuccessorInfo {
            task_id task; // finishing task leading to successor
            myfloat probability; // corresponding probability
            Snapshot* snapshot; // snapshot itself
            SuccessorInfo(task_id t, myfloat p, Snapshot* s) :
                task(t),
                probability(p),
                snapshot(s)
            {}
        };
    private:
        // private constructor which sets *everything*
        Snapshot(const Intree& t, 
            vector<task_id>& m, 
            vector<SuccessorInfo>& s
        );

        // snapshots are organized in a set of pools (no duplicates)
        // We need different pools for different versions of the snap,
        // such that each pool is "indexed" by one kind of pool.
        enum PoolKind {
            PoolDefault = 0,
            PoolOptimized,
        };
        static map<PoolKind, map<snapshot_id, Snapshot*>> pool;

        // we cache the result of expected_runtime
        mutable myfloat cache_expected_runtime;
        bool finalized;

        vector<SuccessorInfo> successors;

    public:
        const vector<task_id> marked;
        const Intree intree;

        const vector<SuccessorInfo>& Successors() const {
            return successors;
        }
        // constructors and destructors 
        Snapshot();
        Snapshot(const Snapshot& s);
        Snapshot(const Intree& t);
        Snapshot(const Intree& t, vector<task_id> m);
        ~Snapshot();

        // canonical snapshot stuff
        static Snapshot* canonical_snapshot(const Snapshot& s,
                map<task_id, task_id>* isomorphism=NULL,
                Snapshot::PoolKind representant = PoolDefault);
        // TODO: is it necessary to pass m by val?
        static Snapshot* canonical_snapshot(const Intree& t, 
                vector<task_id> m,
                map<task_id, task_id>* isomorphism=NULL,
                Snapshot::PoolKind representant = PoolDefault);

        static Snapshot* find_snapshot_in_pool(const Snapshot& s,
                Snapshot::PoolKind representant = PoolDefault);
        static Snapshot* find_snapshot_in_pool(const Intree& t,
                vector<task_id> m,
                Snapshot::PoolKind representant = PoolDefault);

        Snapshot* optimize() const;

        unsigned int count_tasks() const;

        void get_successors(const Scheduler& scheduler, 
                Snapshot::PoolKind representant = PoolDefault);
        void compile_snapshot_dag(const Scheduler& scheduler,
                Snapshot::PoolKind representant = PoolDefault);
        size_t get_successor_count();
        void consolidate(bool strict = true);
        void finalize();

        const Snapshot* get_next_on_successor_path(const Snapshot* t) const;
        myfloat get_reaching_probability(const Snapshot* target) const;
        myfloat get_reaching_probability(const Snapshot* target,
                const Snapshot* orig) const;

        myfloat expected_runtime() const;

        bool operator== (const Snapshot& s) const ;
        friend ostream& operator<<(ostream& os, const Snapshot& s);

        string markedstring();

        static void clear_pool();

        unsigned long count_snapshots_in_dag() const ;
        unsigned long count_snapshots_in_dag(map<const Snapshot*, bool>&) const;
};

#if PYTHON_TESTS
extern "C" void initsnapshot();
#endif

#endif
