#include<iostream>
#include<random>
#include<time.h>

#include "info.h"
#include "intree.h"
#include "snapshot.h"

using namespace std;

void randomEdges(int n, vector<pair<Task,Task>>& target){
    mt19937 rng;
    rng.seed(time(NULL));
    for(int i=0; i<n; ++i){
        int a = rng()%(i+1);
        target.push_back(pair<Task,Task>(Task(i+1),Task(a)));
    }   
}

#define N 9
// TODO: rule-of-three everywhere!
int main(int argc, char** argv){
    print_version();
    vector<pair<Task,Task>> edges;
    randomEdges(N, edges);
    for(auto it = edges.begin(); it != edges.end(); ++it){
        cout << it->first << " -> " << it->second << endl;
    }
    Intree t(edges);
    vector<task_id> marked;
    marked.push_back(N);
    marked.push_back(N-1);
    Snapshot s(t, marked);
    cout << s << endl;
    s.get_successors();
    return 0;
}
