#ifndef INTREE_H
#define INTREE_H

#include<utility>
#include<vector>
#include<iostream>
#include<set>
#include<map>
#include<queue>
#include<functional>
#include<algorithm>
#include<assert.h>
#include<boost/dynamic_bitset.hpp>

#include "task.h"

using namespace std;

/* An intree stores a collection of threads and their dependencies.
*/
class Intree {
    friend class Probability_Computer;
    friend class Snapshot;
    private:
        map<task_id, task_id> edges;
        map<task_id, Task> taskmap;
    public:
        Intree();
        Intree(const Intree& t);
        Intree(const vector<pair<Task, Task>>& edges);
        static Intree canonical_intree(const Intree& t, 
                                       const vector<task_id>& preferred,
                                       map<task_id, task_id>& isomorphism, 
                                       tree_id& out);

        unsigned int count_tasks() const;

        int get_in_degree(const Task& t) const ;
        int get_in_degree(const task_id t) const ;

        bool contains_task(task_id tid) const;
        const Task& get_task_by_id(task_id tid) const;
        void get_tasks(set<task_id>& result) const;
        void rename_leaf(task_id original, task_id now);

        int get_level(const Task& t) const;
        int get_level(const task_id t) const;

        void get_predecessors(const Task& t, vector<task_id>& target) const;
        void get_predecessors(const task_id t, vector<task_id>& target) const;
        void get_leaves(vector<task_id>& target) const;
        bool is_leaf(const task_id t) const;

        Distribution get_task_distribution(const task_id t) const;
        void remove_task(Task& t);
        void remove_task(task_id t);

        pair<Task, Task> get_edge_from(const Task& t) const;
        pair<Task, Task> get_edge_from(const task_id t) const;

        bool is_chain() const;
        bool same_chain(const task_id t1, const task_id t2) const;
        unsigned int count_free_chains(vector<task_id>& marked) const;
        void get_chain(const Task& t, vector<task_id>& target) const;
        void get_chain(const task_id t, vector<task_id>& target) const;
        void get_chains(vector<vector<task_id>>& target) const;
        unsigned int longest_chain_length() const;

        void get_reverse_tree(map<task_id, vector<task_id>>& rt) const;
        unsigned int get_max_width(task_id tid = 0) const;

        friend ostream& operator<<(ostream& os, const Intree& t);
};

#endif
