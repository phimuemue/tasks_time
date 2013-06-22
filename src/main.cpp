#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#else
#include<sys/ioctl.h>
#endif

#include<time.h>

#include<iostream>
#include<random>
#include<string>
#include<fstream>

#include<boost/program_options.hpp>

#include "info.h"
#include "intree.h"
#include "snapshot.h"

// schedulers
#include "hlfscheduler.h"
#include "hlfnfcscheduler.h"
#include "hlfdeterministicscheduler.h"
#include "hlfrandomscheduler.h"

// exporters
#include "exporter.h"
#include "tikzexporter.h"
#include "newtikzexporter.h"
#include "chainsideexporter.h"
#include "dagviewexporter.h"

#include "alltrees.h"

using namespace std;
namespace po = boost::program_options;

#ifndef NUM_THREADS_DEFAULT
#define NUM_THREADS_DEFAULT 5
#endif

map<string, Scheduler*> schedulers = 
{
    // "all possibilities" scheduler
    {"leaf", new Leafscheduler()}, 
    // HLF schedulers (variants)
    {"hlf", new HLFscheduler()},
    {"hlfnfc", new HLFNFCscheduler()},
    {"hlfdet", new HLFDeterministicScheduler()},
    {"hlfrand", new HLFRandomScheduler()},
};
// TODO: can we solve this more elegant?
#define SCHEDULERS_AVAILABLE "leaf, hlf, hlfnfc, hlfdet, hlfrand"

void randomEdges(int n, vector<pair<Task,Task>>& target){
    mt19937 rng;
    rng.seed(time(NULL));
    ofstream output;
    output.open(".last.intree");
    for(int i=0; i<n; ++i){
        int a = rng()%(i+1);
        output << a << " ";
        target.push_back(pair<Task,Task>(Task(i+1),Task(a)));
    }   
    output.close();
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

#define LINE_LENGTH get_terminal_line_length()

unsigned short get_terminal_line_length(){
#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
    return 70
#else
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col;
#endif
}

int read_variables_map_from_args(int argc, 
        char** argv, 
        po::variables_map& vm){
    // generic options
    po::options_description generic_options("Generic", LINE_LENGTH);
    generic_options.add_options()
        ("help,h", "Print help message");
    // output options
    po::options_description dv_output_options("Dagview output", LINE_LENGTH);
    dv_output_options.add_options()
        ("dv", po::value<string>()->implicit_value(""), 
         "Generate output for DAG viewer in file.")
        ("dvlimit", po::value<unsigned int>()->default_value(0), 
         "Only show snapshots with a certain amount of tasks in dagview.")
        ("dvonlybest", po::value<bool>()->default_value(false)->zero_tokens(), 
         "Only show best schedule in dagview.")
        ;
    po::options_description tikz_output_options("Tikz output", LINE_LENGTH);
    tikz_output_options.add_options()
        ("tikz", po::value<string>()->implicit_value(""), 
         "Generate raw TikZ-Output of snapshot(s) in file.")
        ("tikzchainside", po::value<string>()->implicit_value(""), 
         "Generate condensed (longest chain/side nodes) TikZ-Output of snapshot(s) in file.")
        ("tikzhorizontal", po::value<bool>()->default_value(false)->zero_tokens(), 
         "Draw horizontal TikZ-DAG instead of vertical TikZ-DAG.")
        ("tikzlimit", po::value<unsigned int>()->default_value(0), 
         "Only show snapshots with a certain amount of tasks in TikZ.")
        ("tikzexp", po::value<bool>()->default_value(true), 
         "Draw expected values in TikZ.")
        ("tikzprobs", po::value<bool>()->default_value(true), 
         "Draw transition probabilities in TikZ.")
        ("tikzreachprobs", po::value<bool>()->default_value(true), 
         "Draw reaching probabilities in TikZ.")
        ("tikzld", po::value<float>()->default_value(15.f), 
         "Determines the level distance in the Snap-DAG.")
        ("tikzsd", po::value<float>()->default_value(1.f), 
         "Determines the sibling distance in the Snap-DAG.")
        ("tikzonlybest", po::value<bool>()->default_value(false)->zero_tokens(), 
         "Only show best schedule in TikZ output.")
        ;
    // input options
    po::options_description input_options("Input", LINE_LENGTH);
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
    po::options_description config_options("Config", LINE_LENGTH);
    config_options.add_options()
        ("processors,p", po::value<int>()->default_value(2), 
         "Number of processors to use.")
        ("scheduler,s", po::value<string>()->default_value("hlf"), 
         "Scheduler type to use (" SCHEDULERS_AVAILABLE ").")
        ("optimize", po::value<bool>()->default_value(false)->zero_tokens(), 
         "Generate optimal schedule out of computed schedule by picking best successors.");
    po::options_description desc("Options", LINE_LENGTH);
    desc
        .add(config_options)
        .add(input_options)
        .add(dv_output_options)
        .add(tikz_output_options)
        .add(generic_options)
        ;
    try{
        po::store(po::parse_command_line(argc, argv, desc), vm);
        if(vm.count("help")){
            print_version(true);
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
        vector<Snapshot*>& s,
        vector<myfloat>& expected_runtimes){
    cout << "Warning: We are currently not considering "
            "probabilities for initial settings!" << endl;
    // generate all possible initial markings
    vector<task_id> marked;
    sched->get_initial_schedule(t, vm["processors"].as<int>(), initial_settings);
    cout << "Generating initial settings." << endl;
    s = vector<Snapshot*>(initial_settings.size());
    for(unsigned int i= 0; i<initial_settings.size(); ++i){
        s[i] = Snapshot::canonical_snapshot(Snapshot(t, initial_settings[i]));
    }
#if USE_CANONICAL_SNAPSHOT
    vector<Snapshot*> p_s;
    for(auto it=s.begin(); it!=s.end(); ++it){
        p_s.push_back(Snapshot::canonical_snapshot(**it));
    }
    sort(p_s.begin(), p_s.end());
    p_s.erase(unique(p_s.begin(), p_s.end()), p_s.end());
    s.clear();
    for(auto it=p_s.begin(); it!=p_s.end(); ++it){
        s.push_back(*it);
    }
#endif
#if USE_SIMPLE_OPENMP
#pragma omp parallel for num_threads(initial_settings.size())
#endif
    cout << "Compiling snapshot DAGs." << endl;
    for(unsigned int i= 0; i<s.size(); ++i){
        s[i]->compile_snapshot_dag(*sched);
    }
    expected_runtimes = vector<myfloat>(s.size());
}

void generate_output(const po::variables_map& vm,
        const vector<Snapshot*>& s,
        const vector<Snapshot*>& best,
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
        TikzExporter2 tikz_exporter(
                vm["tikzexp"].as<bool>(),
                vm["tikzprobs"].as<bool>(),
                vm["tikzreachprobs"].as<bool>(),
                vm["tikzlimit"].as<unsigned int>()
                );
        tikz_exporter.level_distance = vm["tikzld"].as<float>();
        tikz_exporter.sibling_distance = vm["tikzsd"].as<float>();
        tikz_exporter.horizontal = vm["tikzhorizontal"].as<bool>();
        bool onlybest = vm["tikzonlybest"].as<bool>();
        for(Snapshot* it : (onlybest ? best : s)){
            tikz_exporter.export_snapshot_dag(tikz_output, it);
        }
        tikz_output.close();
    }
    if(vm.count("tikzchainside")){
        ofstream tikz_output;
        string filename = vm["tikzchainside"].as<string>();
        if(filename==""){
            filename = "default_chain.tex";
        }
        cout << "Writing tikz to " << filename << endl;
        tikz_output.open(filename);
        ChainSideExporter tikz_exporter;
        tikz_exporter.task_count_limit = vm["tikzlimit"].as<unsigned int>();
        tikz_exporter.show_probabilities = vm["tikzprobs"].as<bool>();
        tikz_exporter.show_reaching_probabilities = vm["tikzreachprobs"].as<bool>(); 
        tikz_exporter.show_expectancy = vm["tikzexp"].as<bool>();
        tikz_exporter.level_distance = vm["tikzld"].as<float>();
        tikz_exporter.sibling_distance = vm["tikzsd"].as<float>();
        tikz_exporter.horizontal = vm["tikzhorizontal"].as<bool>();
        bool onlybest = vm["tikzonlybest"].as<bool>();
        for(Snapshot* it : (onlybest ? best : s)){
            tikz_exporter.export_snapshot_dag(tikz_output, it);
        }
        tikz_output.close();
    }
    if(vm.count("dv")){
        ofstream dagview_output;
        string filename = vm["dv"].as<string>();
        if(filename==""){
            filename = "dag.dagview";
        }
        cout << "Writing dagview to " << filename << endl;
        dagview_output.open(filename);
        DagviewExporter dagview_exporter(vm["dvlimit"].as<unsigned int>());
        bool onlybest = vm["dvonlybest"].as<bool>();
        for(Snapshot* it : (onlybest ? best : s)){
            dagview_exporter.export_snapshot_dag(dagview_output, it);
        }
        dagview_output.close();
    }
}

void generate_stats(const po::variables_map& vm,
        const vector<Snapshot*>& s,
        const vector<Snapshot*>& best,
        const vector<vector<task_id>>& initial_settings
        ){
    for(unsigned int i= 0; i<s.size(); ++i){
        if(find(best.begin(), best.end(), s[i]) != best.end()){
            cout << "* ";
        }
        else{
            cout << "  ";
        }
        cout << s[i]->markedstring() << ":\t";
        cout << s[i]->expected_runtime();
#if MYFLOAT==GNUMP_RATIONAL
        cout << "(" << s[i]->expected_runtime().get_d() << ")";
#endif
        cout << "\t(" << s[i]->expected_time_for_n_processors(3) << "/";
        cout << s[i]->expected_time_for_n_processors(2) << "/";
        cout << s[i]->expected_time_for_n_processors(1) << ")";
        cout << "\t(" << s[i]->count_snapshots_in_dag() << " snaps)";
        cout << endl;
    }
}

// TODO: rule-of-three everywhere!
int main(int argc, char** argv){
#if USE_SIMPLE_OPENMP // openmp settings - useful?
    omp_set_nested(1);
#endif

    print_version(false);

    // vector<pair<Task, Task>> e1;
    // tree_from_string("0 0 0 1 1 2 2 3", e1);
    // Intree t1(e1);

    // vector<task_id> marked1 = {4, 8};
    // vector<task_id> marked2 = {6, 8};

    // Snapshot s1 = *Snapshot::canonical_snapshot(Snapshot(t1, marked1));
    // Snapshot s2 = *Snapshot::canonical_snapshot(Snapshot(t1, marked2));

    // cout << s1 << endl;
    // cout << s2 << endl;

    // cout << t1.longest_chain_length() << endl;

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
        tree_id tid;
        cout << "Normalized:  \t" 
             << Intree::canonical_intree(t, 
                     vector<task_id>(),
                     isomorphism, tid) 
             << endl;
        
        // compute snapshot dags
        vector<vector<task_id>> initial_settings;
        vector<Snapshot*> s;
        vector<myfloat> expected_runtimes;

        if(schedulers.find(vm["scheduler"].as<string>()) == schedulers.end()){
            cout << "Scheduler " << vm["scheduler"].as<string>() << " not found." << endl;
            return 1;
        }
        create_snapshot_dags(vm,
                t,
                schedulers[vm["scheduler"].as<string>()],
                initial_settings,
                s,
                expected_runtimes);

        // optimize current snapshot
        if(vm["optimize"].as<bool>()){
            cout << "Optimizing scheduling policies." << endl;
            for(unsigned int i= 0; i<s.size(); ++i){
                s[i] = s[i]->optimize();
            }
        }

        // compute expected runtimes
        cout << "Computing expected runtimes." << endl;
        assert(expected_runtimes.size() == s.size());
        for(unsigned int i= 0; i<s.size(); ++i){
            expected_runtimes[i] = s[i]->expected_runtime();
        }
        myfloat expected_runtime = 0;
        vector<Snapshot*> best {s[0]};
        for(unsigned int i= 0; i<s.size(); ++i){
            if(i>0 && s[i]->expected_runtime() <= best[0]->expected_runtime()){
                if(s[i]->expected_runtime() < best[0]->expected_runtime()){
                    best.clear();
                }
                best.push_back(s[i]);
            }
            expected_runtime += expected_runtimes[i];
        }
        expected_runtime /= (myfloat)s.size();

        // output stats
        generate_stats(vm, s, best, initial_settings);

        cout << "Total expected run time: " << expected_runtime 
            << " (Warning: This number does not consider probabilities"
            << " of initial settings (thus is wrong)!)" << endl;

        // output stuff to files
        generate_output(vm, s, best, initial_settings);
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
