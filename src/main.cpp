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

#include "alltrees.h"

using namespace std;

void randomEdges(int n, vector<pair<Task,Task>>& target){
    mt19937 rng;
    rng.seed(time(NULL));
    for(int i=0; i<n; ++i){
        int a = rng()%(i+1);
        target.push_back(pair<Task,Task>(Task(i+1),Task(a)));
    }   
}

#ifndef NUM_THREADS
#define NUM_THREADS 4
#endif

#ifndef NUM_PROCESSORS
#define NUM_PROCESSORS 2
#endif

// TODO: rule-of-three everywhere!
int main(int argc, char** argv){
    print_version();

    // command line parsing stuff
    try{
        namespace po = boost::program_options;
        po::options_description desc("Options");
        desc.add_options()
            ("help", "Print help message")
            ("tikz", po::value<string>(), "Generate TikZ-Output of snapshot(s) in file.")
            ("dagview", po::value<string>(), "Generate output for DAG viewer in file.")
            ;
        po::variables_map vm;
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

        // openmp settings - useful?
        omp_set_nested(1);

        // generate tree
        vector<pair<Task,Task>> edges;
        randomEdges(NUM_THREADS, edges);
        Intree t(edges);
        cout << "Intree: " << t << endl;

        // generate all possible initial markings
        Scheduler* sched = new HLFscheduler();
        vector<task_id> marked;
        vector<vector<task_id>> initial_settings;
        sched->get_initial_schedule(t, NUM_PROCESSORS, initial_settings);

        myfloat expected_runtimes[initial_settings.size()];
#pragma omp parallel for num_threads(initial_settings.size())
        for(unsigned int i= 0; i<initial_settings.size(); ++i){
            Snapshot s(t, initial_settings[i]);
#pragma omp critical
            {
                cout << "Compiling snapshot DAG for " << s << endl;
            };
            s.compile_snapshot_dag(*sched);
            expected_runtimes[i] = s.expected_runtime();
            cout << endl;
#pragma omp critical
            {
                if(vm.count("tikz")){
                    ofstream output;
                    output.open(vm["tikz"].as<string>());
                    output << s.tikz_string_dag() << endl;
                    output.close();
                }
                if(vm.count("dagview")){
                    ofstream output;
                    output.open(vm["dagview"].as<string>());
                    output << s.dag_view_string() << endl;
                    output.close();
                }
            }
        }
        myfloat expected_runtime = 0;
        for(unsigned int i= 0; i<initial_settings.size(); ++i){
            cout << expected_runtimes[i] << endl;
            expected_runtime += expected_runtimes[i];
        }
        expected_runtime /= (myfloat)initial_settings.size();
        cout << "Total expected run time: " << expected_runtime << endl;
        // TODO: The following results in undefined behaviour. Why?
        // delete(sched);
    }
    catch(exception& e){
        cout << "Something went wrong:" << endl
             << e.what() << endl;
    }
    return 0;
}
