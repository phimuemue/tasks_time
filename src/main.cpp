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

#ifndef NUM_THREADS_DEFAULT
#define NUM_THREADS_DEFAULT 5
#endif

int NUM_PROCESSORS = 2;

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

int read_variables_map_from_args(int argc, 
        char** argv, 
        po::variables_map& vm){
    // generic options
    po::options_description generic_options("Generic");
    generic_options.add_options()
        ("help", "Print help message");
    // output options
    po::options_description output_options("Output");
    output_options.add_options()
        ("dagview", po::value<string>()->implicit_value(""), 
         "Generate output for DAG viewer in file.")
        ("tikz", po::value<string>()->implicit_value(""), 
         "Generate TikZ-Output of snapshot(s) in file.");
    // input options
    po::options_description input_options("Input");
    input_options.add_options()
        ("direct", po::value<string>(), 
         "Direct input of tree (sequence of edge "
         "targets from sequentially numbered tasks).")
        ("input", po::value<string>(), 
         "Name of input file.")
        ("random", po::value<int>(), 
         "Number of tasks in a random graph. Only used "
         "if no input file is given.");
    // configurational things
    po::options_description config_options("Config");
    config_options.add_options()
        ("processors,p", po::value<int>(), 
         "Number of processors to use.")
        ("scheduler,s", po::value<string>()->default_value("hlf"), 
         "Scheduler type to use.");
    po::options_description desc("Options");
    desc.add(generic_options)
        .add(input_options)
        .add(output_options)
        .add(config_options)
        ;
    try{
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if(vm.count("help")){
            cout << "Help." << endl
                << desc << endl;
            return 1;
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

    //Snapshot s[initial_settings.size()];
    s = vector<Snapshot>(initial_settings.size());
    for(unsigned int i= 0; i<initial_settings.size(); ++i){
        s[i] = Snapshot(t, initial_settings[i]);
    }
#if USE_CANONICAL_SNAPSHOT
    cout << "Warning: We are currently not considering "
            "probabilities for initial settings!" << endl;
    vector<Snapshot*> p_s;
    for(auto it=s.begin(); it!=s.end(); ++it){
        p_s.push_back(Snapshot::canonical_snapshot(*it));
    }
    sort(p_s.begin(), p_s.end());
    p_s.erase(unique(p_s.begin(), p_s.end()), p_s.end());
    s.clear();
    for(auto it=p_s.begin(); it!=p_s.end(); ++it){
        s.push_back(**it);
    }
#endif
#if USE_SIMPLE_OPENMP
#pragma omp parallel for num_threads(initial_settings.size())
#endif
    cout << "Compiling snapshot DAGs." << endl;
    for(unsigned int i= 0; i<s.size(); ++i){
        s[i].compile_snapshot_dag(*sched);
    }
    expected_runtimes = vector<myfloat>(s.size());
}

void generate_output(const po::variables_map& vm,
        vector<Snapshot>& s,
        const vector<vector<task_id>>& initial_settings
        ){ 
    if(vm.count("tikz")){
        ofstream tikz_output;
        string filename = vm["tikz"].as<string>();
        if(filename==""){
            filename = "default.tex";
        }
        cout << "Writing tikz to " << filename << endl;
        tikz_output.open(filename);
        for(unsigned int i= 0; i<s.size(); ++i){
            tikz_output << s[i].tikz_string_dag() << endl;
        }
        tikz_output.close();
    }
    if(vm.count("dagview")){
        ofstream dagview_output;
        string filename = vm["dagview"].as<string>();
        if(filename==""){
            filename = "dag.dagview";
        }
        cout << "Writing dagview to " << filename << endl;
        dagview_output.open(filename);
        for(unsigned int i= 0; i<s.size(); ++i){
            dagview_output << s[i].dag_view_string() << endl;
        }
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
    // tree_id out;
    // vector<task_id> pref;
    // map<task_id,task_id> isom;
    // Intree t1n = Intree::canonical_intree(t1, pref, isom, out);
    // Intree t2n = Intree::canonical_intree(t2, pref, isom, out);
    // cout << t1n << endl;
    // cout << t2n << endl;
    // 
    // vector<task_id> preds;
    // vector<pair<Task, Task>> edg;
    // randomEdges(5, edg);
    // t1n = Intree(edg);
    // cout << t1n << endl;
    // t1n.get_predecessors(1, preds);
    // for(auto it=preds.begin(); it!=preds.end(); ++it){
    //     cout << *it << endl;
    // }
    // 
    // return 1;

    try{
        // command line parsing stuff
        po::variables_map vm;
        if(read_variables_map_from_args(argc, argv, vm)!=0){
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

        map<string, Scheduler*> scheds = 
        {
            // {"leaf", new Leafscheduler()}, 
            {"hlf", new HLFscheduler()},
            {"hlfnfc", new HLFNFCscheduler()},
        };

        create_snapshot_dags(vm,
                t,
                scheds[vm["scheduler"].as<string>()],
                initial_settings,
                s,
                expected_runtimes);

        assert(expected_runtimes.size() == s.size());
        for(unsigned int i= 0; i<s.size(); ++i){
            expected_runtimes[i] = s[i].expected_runtime();
        }

        myfloat expected_runtime = 0;
        for(unsigned int i= 0; i<s.size(); ++i){
            cout << s[i].markedstring() << ":\t";
            cout << expected_runtimes[i] << endl;
            expected_runtime += expected_runtimes[i];
        }
        expected_runtime /= (myfloat)s.size();
        cout << "Total expected run time: " << expected_runtime 
            << " (Warning: This number does not consider probabilities"
            << " of initial settings (this is wrong)!)" << endl;

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
