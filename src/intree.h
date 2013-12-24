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
#include<sstream>
#include<string>
#include<boost/dynamic_bitset.hpp>
#if PYTHON_TESTS
#include<Python.h>
#include<boost/python/suite/indexing/vector_indexing_suite.hpp>
#endif

#include "task.h"

using namespace std;

/* An intree stores a collection of threads and their dependencies.
*/
class Intree {
    friend class Probability_Computer;
    friend class Snapshot;
    friend class CommandLineExporter;
    private:
        std::vector<task_id> edges;
#if USE_TASKMAP
        map<task_id, Task> taskmap;
#endif
        class Outtree {
            private:
                mutable string compressedString;
            public:
                task_id id;
                bool marked;
                std::vector<Outtree*> predecessors;
                Outtree(task_id i, bool m);
                Outtree(const Intree& i, const vector<task_id>& marked);
                Outtree(const Outtree& ot);
                ~Outtree();
                void canonicalize();
                Intree toIntree(map<task_id, task_id>& isomorphism) const;
                string getCompressedString() const;
        };
        friend inline bool operator<(const Intree::Outtree& a, const Intree::Outtree& b){
            return a.getCompressedString() > b.getCompressedString();
            if (a.predecessors.size() > b.predecessors.size()){
                return true;
            }
            for(unsigned int i=0; i<min(a.predecessors.size(), b.predecessors.size()); ++i){
                if (a.predecessors[i] < b.predecessors[i]){
                    return true;
                }
            }
            if (a.marked==1 && b.marked==0){
                return true;
            }
            return false;
        };
    public:
        Intree();
        Intree(const Intree& t);
        Intree(const vector<pair<task_id, task_id>>& edges);
        Intree(const vector<pair<Task, Task>>& edges);
        static Intree canonical_intree3(const Intree& _t, 
                const vector<task_id>& _preferred,
                map<task_id, task_id>& isomorphism,
                tree_id& out);
        static Intree canonical_intree2(const Intree& _t, 
                const vector<task_id>& _preferred,
                map<task_id, task_id>& isomorphism,
                tree_id& out);
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
        void get_tasks(vector<task_id>& result) const;
        vector<task_id> get_tasks() const;
        void rename_leaf(task_id original, task_id now);

        unsigned int get_level(const Task& t) const;
        unsigned int get_level(const task_id t) const;

        task_id get_successor(const task_id t) const;
        void get_predecessors(const Task& t, vector<task_id>& target) const;
        void get_predecessors(const task_id t, vector<task_id>& target) const;
        vector<task_id> get_predecessors(const task_id t) const;
        void get_siblings(const task_id t, vector<task_id>& target) const;
        vector<task_id> get_siblings(const task_id t) const;
        void get_leaves(set<task_id>& target) const;
        void get_leaves(vector<task_id>& target) const;
        vector<task_id> get_leaves() const;
        bool is_leaf(const task_id t) const;

        Distribution get_task_distribution(const task_id t) const;
        void remove_task(Task& t);
        void remove_task(task_id t);

        pair<Task, Task> get_edge_from(const Task& t) const;
        pair<Task, Task> get_edge_from(const task_id t) const;

        bool is_chain() const;
        bool is_degenerate_tree() const;
        bool is_parallel_chain() const;
        bool same_chain(const task_id t1, const task_id t2) const;
        unsigned int count_free_chains(vector<task_id>& marked) const;
        void get_chain(const Task& t, vector<task_id>& target) const;
        void get_chain(const task_id t, vector<task_id>& target) const;
        void get_chains(vector<vector<task_id>>& target) const;
        unsigned int longest_chain_length() const;
        void get_profile(vector<unsigned int>& target) const;

        void get_raw_tree_id(tree_id& target) const;

        void get_reverse_tree(map<task_id, vector<task_id>>& rt) const;
        unsigned int get_max_width(task_id tid = 0) const;

        bool operator==(const Intree& t) const;
        friend ostream& operator<<(ostream& os, const Intree& t);
};

#if PYTHON_TESTS
extern "C" void initintree();
#endif

#endif
