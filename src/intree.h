#ifndef INTREE_H
#define INTREE_H

#include<utility>
#include<vector>
#include<iostream>
#include<set>
#include<map>
#include<functional>
#include<algorithm>
#include<assert.h>

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
        Intree(vector<pair<Task, Task>>& edges);

        int count_tasks() const;
        int get_in_degree(const Task& t) const ;
        int get_in_degree(const task_id t) const ;
        const Task& get_task_by_id(task_id tid) const;
        void get_tasks(set<task_id>& result) const;
        int get_level(const Task& t) const;
        int get_level(const task_id t) const;

        Distribution get_task_distribution(const task_id t) const;
        void remove_task(Task& t);
        void remove_task(task_id t);

        pair<Task, Task> get_edge_from(const Task& t) const;
        pair<Task, Task> get_edge_from(const task_id t) const;

        bool is_chain();
        void get_chain(const Task& t, vector<int>& target) const;
        void get_chain(const task_id t, vector<int>& target) const;
        void get_chains(vector<vector<int>>& target) const;

        friend ostream& operator<<(ostream& os, const Intree& t);
};

#endif
