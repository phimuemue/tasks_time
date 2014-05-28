#ifndef TASKDAG_H
#define TASKDAG_H

#if PYTHON_TESTS
#include<Python.h>
#include<boost/python/suite/indexing/vector_indexing_suite.hpp>
#endif
#include<utility>
#include<vector>
#include<iostream>
#include<set>
#include<map>
#include<queue>
#include<functional>
#include<algorithm>
#include<assert.h>
#include<sstream>
#include<string>
#include<boost/dynamic_bitset.hpp>

#include "task.h"

using namespace std;

/* A taskdag stores a collection of threads and their dependencies.
*/
#if USE_TASKMAP
#error "DAGs not yet implemented with taskmaps"
#else
class TaskDAG {
    friend class Probability_Computer;
    friend class Snapshot;
    friend class CommandLineExporter;
    private:
        std::map<task_id, std::vector<task_id>> edges;
    public:
        TaskDAG();
        TaskDAG(const TaskDAG& t);
        TaskDAG(const vector<pair<task_id, vector<task_id>>>& edges);
        TaskDAG(const map<task_id, vector<task_id>>& edges);
        static TaskDAG canonical_taskdag(const TaskDAG& t, 
                const vector<task_id>& preferred,
                map<task_id, task_id>& isomorphism, 
                tree_id& out);

        unsigned int count_tasks() const;

        int get_in_degree(const task_id t) const ;

        bool contains_task(task_id tid) const;
        void get_tasks(set<task_id>& result) const;
        void get_tasks(vector<task_id>& result) const;
        vector<task_id> get_tasks() const;
        void rename_leaf(task_id original, task_id now);

        unsigned int get_level(const task_id t) const;

        // TODO: Provide overload to get_successors returning const reference
        vector<task_id> get_successors(const task_id t) const;
        void get_predecessors(const task_id t, vector<task_id>& target) const;
        vector<task_id> get_predecessors(const task_id t) const;
        void get_siblings(const task_id t, vector<task_id>& target) const;
        vector<task_id> get_siblings(const task_id t) const;
        void get_leaves(set<task_id>& target) const;
        void get_leaves(vector<task_id>& target) const;
        vector<task_id> get_leaves() const;
        bool is_leaf(const task_id t) const;

        void remove_task(task_id t);

        void get_raw_tree_id(tree_id& target) const;

        bool operator==(const TaskDAG& t) const;
        friend ostream& operator<<(ostream& os, const TaskDAG& t);
};

#if PYTHON_TESTS
extern "C" void inittaskdag();
#endif

#endif

#endif
