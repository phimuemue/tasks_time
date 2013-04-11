#ifndef INTREE_H
#define INTREE_H

#include<utility>
#include<vector>
#include<iostream>
#include<set>
#include<functional>
#include<algorithm>
#include<assert.h>

#include "task.h"

using namespace std;

/* An intree stores a collection of threads and their dependencies.
*/
class Intree {
private:
    vector<pair<Task, Task>> edges;
public:
    Intree();
    Intree(const Intree& t);
    Intree(vector<pair<Task, Task>>& edges);

    int get_in_degree(const Task& t) const ;
    int get_in_degree(const task_id t) const ;
    void get_tasks(set<Task>& result);
    void remove_task(Task& t);
    void remove_task(task_id t);

    // TODO: Make methods more efficient by working with references/pointers
    pair<Task, Task> get_edge_from(const Task& t) const;
    pair<Task, Task> get_edge_from(const task_id t) const;

    void get_chain(const Task& t, vector<int>& target) const;
    void get_chain(const task_id t, vector<int>& target) const;
    void get_chains(vector<vector<int>>& target) const;
    
    friend ostream& operator<<(ostream& os, const Intree& t);
};

#endif
