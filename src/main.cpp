#include<iostream>
#include<random>
#include<time.h>
#include<string>
#include<fstream>

#include<boost/program_options.hpp>

#include "info.h"
#include "intree.h"
#include "snapshot.h"
#include "hlfscheduler.h"
#include "hlfnfcscheduler.h"

#include "alltrees.h"

using namespace std;
namespace po = boost::program_options;

void randomEdges(int n, vector<pair<Task,Task>>& target){
    mt19937 rng;
    rng.seed(time(NULL));
    for(int i=0; i<n; ++i){
        int a = rng()%(i+1);
        target.push_back(pair<Task,Task>(Task(i+1),Task(a)));
    }   
}

void tree_from_string(string raw, vector<pair<Task,Task>>& target){
    istringstream iss(raw);
    vector<string> raw_tokens;
    copy(istream_iterator<string>(iss),
            istream_iterator<string>(),
            back_inserter<vector<string>>(raw_tokens));
    for(unsigned int i=0; i<raw_tokens.size(); ++i){
        task_id tmp;
        stringstream tmps(raw_tokens[i]);
        tmps >> tmp;
        target.push_back(pair<Task,Task>(Task((task_id)i+1), tmp));
    }
}

void read_raw_tree_from_file(string path, vector<pair<Task,Task>>& target){
    // TODO: I assume (=am sure) that this can be done better.
    // For now, we assume all tasks exponendially iid
    ifstream a(path);
    // read description from file
    string raw((istreambuf_iterator<char>(a)),
            istreambuf_iterator<char>());
    tree_from_string(raw, target);
}

#ifndef NUM_THREADS_DEFAULT
#define NUM_THREADS_DEFAULT 5
#endif

int NUM_PROCESSORS = 2;

int read_variables_map_from_args(int argc, 
        char** argv, 
        po::variables_map& vm){
    po::options_description desc("Options");
    desc.add_options()
        // help message
        ("help", "Print help message")
        // configurational things
        ("processors,p", po::value<int>(), "Number of processors to use.")
        // output stuff
        ("tikz", po::value<string>(), "Generate TikZ-Output of snapshot(s) in file.")
        ("dagview", po::value<string>(), "Generate output for DAG viewer in file.")
        // input stuff
        ("direct", po::value<string>(), "Direct input of tree (sequence of edge targets from sequentially numbered tasks).")
        ("input", po::value<string>(), "Name of input file.")
        ("random", po::value<int>(), "Number of tasks in a random graph. Only used if no input file is given.")
        ;
    try{
        po::store(po::parse_command_line(argc, argv, desc), vm);

        if(vm.count("help")){
            cout << "Help." << endl
                << desc << endl;
            return 0;
        }
    }
    catch(po::error& e){
        cout << "Command line options not recognized." << endl
            << e.what() << endl
            << desc << endl;
        return 1;
    }
    return 0;
}

Intree generate_tree(po::variables_map vm){
    vector<pair<Task,Task>> edges;
    if (vm.count("direct")){
        tree_from_string(vm["direct"].as<string>(), edges);
    }
    else if(vm.count("input")){
        read_raw_tree_from_file(vm["input"].as<string>(), edges);
    }
    else{
        int num_threads = (vm.count("random") ? vm["random"].as<int>() : NUM_THREADS_DEFAULT);
        randomEdges(num_threads, edges);
    }
    return Intree(edges);
}

void create_snapshot_dags(const po::variables_map& vm,
        Intree& t,
        const Scheduler* sched,
        vector<vector<task_id>>& initial_settings,
        vector<Snapshot>& s,
        vector<myfloat>& expected_runtimes){
    // generate all possible initial markings
    if(vm.count("processors")){
        NUM_PROCESSORS = vm["processors"].as<int>();
    }
    vector<task_id> marked;
    sched->get_initial_schedule(t, NUM_PROCESSORS, initial_settings);

    expected_runtimes = vector<myfloat>(initial_settings.size());

    //Snapshot s[initial_settings.size()];
    s = vector<Snapshot>(initial_settings.size());
    cout << "Compiling snapshot DAGs." << endl;
    for(unsigned int i= 0; i<initial_settings.size(); ++i){
        s[i] = Snapshot(t, initial_settings[i]);
    }
#if USE_SIMPLE_OPENMP
#pragma omp parallel for num_threads(initial_settings.size())
#endif
    for(unsigned int i= 0; i<initial_settings.size(); ++i){
        s[i].compile_snapshot_dag(*sched);
    }
}

void generate_output(const po::variables_map& vm,
        vector<Snapshot>& s,
        const vector<vector<task_id>>& initial_settings
        ){ // output stuff
    ofstream tikz_output;
    ofstream dagview_output;
    if(vm.count("tikz")){
        tikz_output.open(vm["tikz"].as<string>());
    }
    if(vm.count("dagview")){
        dagview_output.open(vm["dagview"].as<string>());
    }
    for(unsigned int i= 0; i<initial_settings.size(); ++i){
        {
            if(vm.count("tikz")){
                tikz_output << s[i].tikz_string_dag() << endl;
            }
            if(vm.count("dagview")){
                dagview_output << s[i].dag_view_string() << endl;
            }
        }
    }
    cout << endl;
    // clean up
    if(vm.count("tikz")){
        tikz_output.close();
    }
    if(vm.count("dagview")){
        dagview_output.close();
    }
}

// TODO: rule-of-three everywhere!
int main(int argc, char** argv){
#if USE_SIMPLE_OPENMP // openmp settings - useful?
    omp_set_nested(1);
#endif

    print_version();

    // vector<pair<Task, Task>> e1;
    // vector<pair<Task, Task>> e2;
    // 
    // tree_from_string("0 0 0 1 2 1 2 3", e1);
    // tree_from_string("0 0 0 1 1 2 2 3", e2);
    // Intree t1(e1);
    // Intree t2(e2);
    // cout << t1 << endl;
    // cout << t2 << endl;
    // tree_id out;
    // vector<task_id> pref;
    // map<task_id,task_id> isom;
    // Intree t1n = Intree::canonical_intree(t1, pref, isom, out);
    // Intree t2n = Intree::canonical_intree(t2, pref, isom, out);
    // cout << t1n << endl;
    // cout << t2n << endl;
    // 
    // return 1;

    try{
        // command line parsing stuff
        po::variables_map vm;
        if(read_variables_map_from_args(argc, argv, vm)!=0){
            cout << "Error reading args" << endl;
            return 1;
        }

        // read tree and print it for user
        Intree t = generate_tree(vm);
        cout << "Raw form:\t" << t << endl;
        map<task_id, task_id> isomorphism;
        tree_id tid = 0;
        cout << "Normalized:  \t" 
             << Intree::canonical_intree(t, 
                     vector<task_id>(),
                     isomorphism, tid) 
             << endl;
        
        // compute snapshot dags
        vector<vector<task_id>> initial_settings;
        vector<Snapshot> s;
        vector<myfloat> expected_runtimes;

        vector<Scheduler*> scheds = 
        {
            new HLFscheduler(),
            new HLFNFCscheduler(),
        };

        create_snapshot_dags(vm,
                t,
                scheds[0],
                initial_settings,
                s,
                expected_runtimes);

        for(unsigned int i= 0; i<initial_settings.size(); ++i){
            expected_runtimes[i] = s[i].expected_runtime();
        }

        myfloat expected_runtime = 0;
        for(unsigned int i= 0; i<initial_settings.size(); ++i){
            cout << s[i].markedstring() << ":\t";
            cout << expected_runtimes[i] << endl;
            expected_runtime += expected_runtimes[i];
        }
        expected_runtime /= (myfloat)initial_settings.size();
        cout << "Total expected run time: " << expected_runtime << endl;

        // output stuff
        generate_output(vm, s, initial_settings);
#if USE_CANONICAL_SNAPSHOT
        Snapshot::clear_pool();
#endif
    }
    catch(exception& e){
        cout << "Something went wrong:" << endl
             << e.what() << endl;
    }
    return 0;
}
