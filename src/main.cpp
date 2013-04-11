#include<iostream>
#include<random>
#include<time.h>

#include "intree.h"
#include "info.h"

using namespace std;

void randomEdges(int n, vector<pair<Task,Task>>& target){
    mt19937 rng;
    rng.seed(time(NULL));
    for(int i=0; i<n; ++i){
        int a = rng()%(i+1);
        target.push_back(pair<Task,Task>(Task(i+1),Task(a)));
    }   
}

int main(int argc, char** argv){
    print_version();
    vector<pair<Task,Task>> edges;
    randomEdges(10, edges);
    for(auto it = edges.begin(); it != edges.end(); ++it){
        cout << it->first << " -> " << it->second << endl;
    }
    Intree t(edges);
    cout << t << endl;
    t.remove_task(8);
    cout << t << endl;
    return 0;
}
