#include<iostream>
#include<random>
#include<time.h>

#include "info.h"
#include "intree.h"
#include "snapshot.h"
#include "hlfscheduler.h"

using namespace std;

void randomEdges(int n, vector<pair<Task,Task>>& target){
    mt19937 rng;
    rng.seed(time(NULL));
    for(int i=0; i<n; ++i){
        int a = rng()%(i+1);
        target.push_back(pair<Task,Task>(Task(i+1),Task(a)));
    }   
}

#define NUM_THREADS 12
#define NUM_PROCESSORS 2

// TODO: rule-of-three everywhere!
int main(int argc, char** argv){
    print_version();

    // generate tree
    vector<pair<Task,Task>> edges;
    randomEdges(NUM_THREADS, edges);
    for(auto it = edges.begin(); it != edges.end(); ++it){
        cout << it->first << " -> " << it->second << endl;
    }
    Intree t(edges);

    // generate all possible initial markings
    Scheduler* sched = new HLFscheduler();
    vector<task_id> marked;
    vector<vector<task_id>> initial_settings;
    sched->get_initial_schedule(t, NUM_PROCESSORS, initial_settings);

    myfloat expected_runtimes[initial_settings.size()];
#pragma omp parallel for
    for(unsigned int i= 0; i<initial_settings.size(); ++i){
        Snapshot s(t, initial_settings[i]);
#pragma omp critical
        {
            cout << "Compiling snapshot DAG for " << s << endl;
        };
        s.compile_snapshot_dag(*sched);
        expected_runtimes[i] = s.expected_runtime();
        cout << endl;
    }
    for(unsigned int i= 0; i<initial_settings.size(); ++i){
        cout << expected_runtimes[i] << endl;
    }
    delete(sched);
    return 0;
}
