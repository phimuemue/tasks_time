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
    private:
        // private constructor which sets *everything*
        Snapshot(const Intree& t, 
            vector<task_id>& m, 
            vector<Snapshot*>& s,
            vector<myfloat>& sp,
            vector<task_id>& ft);

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

        // TODO: put successors, successor_probs, finished_task into 1 vector
        vector<Snapshot*> successors;
        vector<myfloat> successor_probs;
        vector<task_id> finished_task;

    public:
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
        struct Probabilities {
            vector<myfloat>::const_iterator begin() const {
                return my_Snapshot->successor_probs.begin();
            }
            vector<myfloat>::const_iterator end() const {
                return my_Snapshot->successor_probs.end();
            }
            private:
            friend class Snapshot;
            Probabilities(Snapshot* s) : my_Snapshot(s) {}
            Snapshot* my_Snapshot;
        } Probabilities;
        struct SuccessorProbabilities {
            typedef boost::tuple<
                vector<Snapshot*>::const_iterator,
                vector<myfloat>::const_iterator
                > sp_tuple;
            boost::zip_iterator<sp_tuple> begin() const {
                return boost::make_zip_iterator(
                    boost::make_tuple(
                        my_Snapshot->successors.begin(),
                        my_Snapshot->successor_probs.begin())
                );
            };
            boost::zip_iterator<sp_tuple> end() const {
                return boost::make_zip_iterator(
                    boost::make_tuple(
                        my_Snapshot->successors.end(),
                        my_Snapshot->successor_probs.end())
                );
            };
            private:
            friend class Snapshot;
            SuccessorProbabilities(Snapshot* s) : my_Snapshot(s) {}
            Snapshot* my_Snapshot;
        } SuccessorProbabilities;

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

#endif
