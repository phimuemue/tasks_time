#if PYTHON_TESTS
#include "pythontester.h"
#endif
#include "info.h"
#include "intree.h"
#include "taskdag.h"
#include "snapshot.h"
#include "treegenerator.h"

#include "stringhelper.h"

// schedulers
#include "hlfscheduler.h"
#include "hlfnfcscheduler.h"
#include "hlfdeterministicscheduler.h"
#include "hlfrandomscheduler.h"
#include "specialcaseleafscheduler.h"
#include "topmostsurescheduler.h"

// exporters
#include "exporter.h"
#include "tikzexporter.h"
#include "newtikzexporter.h"
#include "chainsideexporter.h"
#include "profileexporter.h"
#include "dagviewexporter.h"
#include "commandlineexporter.h"

//testers
#include "tester.h"
#include "hlftester.h"
#include "onlyonenonscheduledsiblingtester.h"
#include "sameruntimetester.h"
#include "unscheduledhlf1tester.h"
#include "runtimetester.h"
#include "singlesuccessortester.h"
#include "topmosttester.h"

#include "tikztopdf.h"

#include "alltrees.h"

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32) && !defined(__CYGWIN__)
#else
#include<sys/ioctl.h>
#endif

#include<time.h>

#include<iostream>
#include<random>
#include<string>
#include<fstream>
#include<memory>

#include<boost/program_options.hpp>
#include<boost/spirit/include/qi.hpp>

using namespace std;
namespace po = boost::program_options;

#ifndef NUM_THREADS_DEFAULT
#define NUM_THREADS_DEFAULT 5
#endif

const map<string, Scheduler*> schedulers = 
{
    // "all possibilities" scheduler
    {"leaf", new Leafscheduler()}, 
    // // for now, we disable all other schedulers
    // // leaf scheduler that schedules as many topmost tasks as possible
    // {"tms", new TopMostSureScheduler()},
    // // leaf scheduler with known special cases
    // {"scleaf", new SpecialCaseLeafscheduler()}, 
    // // HLF schedulers (variants)
    // {"hlf", new HLFscheduler()},
    // {"hlfnfc", new HLFNFCscheduler()},
    // {"hlfdet", new HLFDeterministicScheduler()},
    // {"hlfrand", new HLFRandomScheduler()},
};

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

void randomEdgesPerLevel(string levelstring, vector<pair<Task,Task>>& target){
    mt19937 rng;
    rng.seed(time(NULL));
    istringstream iss(levelstring);
    vector<string> raw_tokens;
    copy(istream_iterator<string>(iss),
            istream_iterator<string>(),
            back_inserter<vector<string>>(raw_tokens));
    vector<vector<unsigned int>> tasks_per_level(raw_tokens.size() + 1);
    unsigned int counter = 0;
    tasks_per_level[0].push_back(0);
    for(unsigned int i=0; i<raw_tokens.size(); ++i){
        task_id tmp;
        stringstream tmps(raw_tokens[i]);
        tmps >> tmp;
        for(unsigned int n=0; n<tmp; ++n){
            tasks_per_level[i+1].push_back(++counter);
        }
    }
    for(unsigned int level = 1; level < tasks_per_level.size(); ++level){
        auto it = tasks_per_level[level];
        auto prev = tasks_per_level[level-1];
        for(auto it2 : it){
            target.push_back(pair<Task, Task>(
                            Task(it2),
                            Task(prev[rng()%(prev.size())])
                        ));
            cout << it2 << " ";
        }
        cout << endl;
    }
}

vector<task_id> tree_from_string(string raw){
    vector<task_id> target{NOTASK};
    boost::spirit::qi::phrase_parse(
        raw.begin(),
        raw.end(),
        *boost::spirit::qi::uint_,
        boost::spirit::ascii::space,
        target
    );
    return target;
}

vector<Intree> read_raw_tree_from_file(string path, vector<pair<Task,Task>>& target){
    vector<Intree> result;
    ifstream a(path);
    // read description from file
    string line;
    while(std::getline(a, line)){
        Intree tmp(tree_from_string(line));
        result.push_back(tmp);
    }
    return result;
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
        ("help,h", "Print help message")
        ("verbosity", po::value<int>()->default_value(1), 
         "The higher this number, the less information is printed.")
        ;
    // output options
    po::options_description dv_output_options("Dagview output", LINE_LENGTH);
    dv_output_options.add_options()
        ("dv", po::value<string>()->implicit_value("dag.dagview"), 
         "Generate output for DAG viewer in file.")
        ("dvlimit", po::value<unsigned int>()->default_value(0), 
         "Only show snapshots with a certain amount of tasks in dagview.")
        ("dvonlybest", po::value<bool>()->default_value(false)->zero_tokens(), 
         "Only show best schedule in dagview.")
        ;
    po::options_description tikz_output_options("Tikz output", LINE_LENGTH);
    tikz_output_options.add_options()
        ("tikz", po::value<string>()->implicit_value("default"), 
         "Generate raw TikZ-Output of snapshot(s) in file.")
        ("tikzchainside", po::value<string>()->implicit_value("default_chain"), 
         "Generate condensed (longest chain/side nodes) TikZ-Output of snapshot(s) in file.")
        ("tikzprofile", po::value<string>()->implicit_value("default_profile"), 
         "Generate profile graph TikZ-Output of snapshot(s) in file.")
        ("tikzlabel", po::value<bool>()->default_value(false)->zero_tokens(), 
         "Print task numbers.")
        ("tikzhorizontal", po::value<bool>()->default_value(false)->zero_tokens(), 
         "Draw horizontal TikZ-DAG instead of vertical TikZ-DAG.")
        ("tikzlimit", po::value<unsigned int>()->default_value(1), 
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
        ("tikzinteractive", po::value<bool>()->default_value(false)->zero_tokens(), 
         "Only show selected schedules in TikZ output.")
        ("tikznopdf", po::value<bool>()->default_value(false)->zero_tokens(), 
         "Prevent TikZ compilation to PDF.")
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
         "Number of tasks in a random graph. Only used if no input file is given.")
        ("randp", po::value<string>(), 
         "Specify the number of tasks level wise. Bottom level is implicitly 0.");
    // configurational things
    string schedulers_available = stringhelper::join(schedulers, ", ", [](decltype(*schedulers.begin()) s){return s.first;});
    schedulers_available = 
        "Scheduler type to use (" + 
        stringhelper::join(schedulers, ", ", 
            [](decltype(*schedulers.begin()) s){return s.first;}
        ) + 
        ")";
    po::options_description config_options("Config", LINE_LENGTH);
    config_options.add_options()
        ("processors,p", po::value<int>()->default_value(2), 
         "Number of processors to use.")
        ("scheduler,s", po::value<string>()->default_value("leaf"), 
         schedulers_available.c_str())
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

vector<Intree> generate_tree(po::variables_map const& vm){
    vector<pair<Task,Task>> edges;
    vector<Intree> result;
    if (vm.count("direct")){
        result.push_back(Intree(tree_from_string(vm["direct"].as<string>())));
    }
    else if(vm.count("randp")){
        randomEdgesPerLevel(vm["randp"].as<string>(), edges);
        result.push_back(Intree(edges));
    }
    else if(vm.count("input")){
        result = read_raw_tree_from_file(vm["input"].as<string>(), edges);
    }
    else{
        int num_threads = (vm.count("random") ? vm["random"].as<int>() : NUM_THREADS_DEFAULT);
        randomEdges(num_threads, edges);
        result.push_back(Intree(edges));
    }
    // write last intree to file for later reuse
    ofstream output;
    output.open(".last.intree.new");
    for(auto& x: edges){
        output << x.second.get_id() << " ";
    }   
    output.close();
    return result;
}

void create_snapshot_dags(const po::variables_map& vm,
        Intree& t,
        Scheduler* sched,
        vector<vector<task_id>>& initial_settings,
        vector<map<task_id, task_id>>& isomorphisms,
        vector<Snapshot*>& s,
        vector<myfloat>& probs,
        vector<myfloat>& expected_runtimes){
    // generate all possible initial markings
    vector<task_id> marked;
    sched->set_processorcount(vm["processors"].as<int>());
    sched->get_initial_schedule(t, initial_settings);
    if(vm["verbosity"].as<int>() < 2){
        cout << "Generating initial settings" << endl;
    }
    s = vector<Snapshot*>(initial_settings.size());
    isomorphisms = vector<map<task_id, task_id>>(initial_settings.size());
#if USE_CANONICAL_SNAPSHOT
    for(unsigned int i= 0; i<initial_settings.size(); ++i){
        std::tie(s[i], isomorphisms[i]) = Snapshot::canonical_snapshot(t, initial_settings[i]);
    }
#else
    for(unsigned int i= 0; i<initial_settings.size(); ++i){
        s[i] = Snapshot::find_snapshot_in_pool(Snapshot(t, initial_settings[i]));
    }
#endif
    vector<pair<Snapshot*, myfloat>> p_s;
    for(auto const& it : s){
        p_s.push_back(pair<Snapshot*, myfloat>(it,(myfloat)1/(myfloat)s.size()));
    }
#if USE_CANONICAL_SNAPSHOT
    for(unsigned int i=0; i<p_s.size(); ++i){
        for(unsigned int j=i+1; j<p_s.size(); ++j){
            if(p_s[i].first->intree == p_s[j].first->intree && p_s[i].first->marked == p_s[j].first->marked){
                p_s[i].second += p_s[j].second;
                p_s.erase(p_s.begin() + j);
                j--;
                initial_settings.erase(initial_settings.begin() + j);
            }
        }
    }
#endif
    s.clear();
    probs.clear();
    for(auto it : p_s){
        s.push_back(it.first);
        probs.push_back(it.second);
    }
#if USE_SIMPLE_OPENMP
#pragma omp parallel for num_threads(initial_settings.size())
#endif

    if(vm["verbosity"].as<int>() < 2){
        cout << "Compiling snapshot DAGs" << endl;
    }
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
    // TikZ export
    map<string, unique_ptr<TikzExporter2>> exporters; 
    exporters["tikz"] = unique_ptr<TikzExporter2>(new TikzExporter2());
    exporters["tikzchainside"] = unique_ptr<TikzExporter2>(new ChainSideExporter());
    exporters["tikzprofile"] = unique_ptr<TikzExporter2>(new ProfileExporter());
    bool globaldrawme = true;
    for(auto& it : exporters){
        string raw_filename;
        string filename;
        if(vm.count(it.first)){
            ofstream tikz_output;
            raw_filename = vm[it.first].as<string>();
            // convert spaces to underscores
            transform(raw_filename.begin(), raw_filename.end(), raw_filename.begin(), 
                    [](char& a) -> char {if (a==' ') return '_'; return a;}
                    );
            filename = raw_filename + ".tex";
            cout << "Writing " << it.first << " to " << filename << endl;
            tikz_output.open(filename);
            TikzExporter2& exporter = *it.second;
            exporter.task_count_limit = vm["tikzlimit"].as<unsigned int>();
            exporter.show_probabilities = vm["tikzprobs"].as<bool>();
            exporter.show_reaching_probabilities = vm["tikzreachprobs"].as<bool>(); 
            exporter.show_expectancy = vm["tikzexp"].as<bool>();
            exporter.level_distance = vm["tikzld"].as<float>();
            exporter.sibling_distance = vm["tikzsd"].as<float>();
            exporter.horizontal = vm["tikzhorizontal"].as<bool>();
            exporter.show_labels = vm["tikzlabel"].as<bool>();
            bool onlybest = vm["tikzonlybest"].as<bool>();
            for(Snapshot* it : (onlybest ? best : s)){
                if(globaldrawme){
                    bool drawme = true;
                    if (vm["tikzinteractive"].as<bool>()){
                        cout << "Export " << it->markedstring() << " ['y': yes, 'n': no, 'N': all following no]?" << endl;
                        string answer;
                        cin >> answer;
                        drawme = answer == "y";
                        globaldrawme = answer != "N";
                    }
                    if (drawme){
                        exporter.export_snapshot_dag(tikz_output, it);
                    }
                }
            }
            tikz_output.close();
            if(!vm["tikznopdf"].as<bool>()){
                cout << "Compiling to PDF." << endl;
                TikzToPdf ttp;
                ttp.convertTikzToPdf(filename, raw_filename);
            }
        }
    }
    // dagview export
    if(vm.count("dv")){
        ofstream dagview_output;
        string filename = vm["dv"].as<string>();
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

string get_expected_runtime_string(const Snapshot* s){
    stringstream ss;
    ss << s->expected_runtime();
    return ss.str();
}

string get_count_snaps_string(const Snapshot* s){
    stringstream ss;
    ss << "(";
    ss << s->count_snapshots_in_dag();
    ss << " snaps";
    ss << ")";
    return ss.str();
}

void generate_stats(const po::variables_map& vm,
        const vector<Snapshot*>& s,
        const vector<Snapshot*>& best,
        const vector<vector<task_id>>& initial_settings,
        const vector<map<task_id, task_id>>& isomorphisms
        ){
    vector<vector<string>> lines;
    assert(s.size() == initial_settings.size());
    HLFTester hlftester;
    OnlyOneNonScheduledSiblingTester oonsst;
    SameRunTimeTester srtt;
    UnscheduledHlf1Tester uh1t;
    SingleSuccessorTester sst;
    TopmostTester tmt;
#if PYTHON_TESTS
    PythonTester pythontest;
#endif
    for(unsigned int i= 0; i<s.size(); ++i){
        lines.push_back(vector<string>());
        vector<string>& line = lines.back();
        // best snapshot?
        if(find(best.begin(), best.end(), s[i]) != best.end()){
            line.push_back(string("*"));
        }
        else{
            line.push_back(string(" "));
        }
        // Test Suite:
        // is HLF?
        line.push_back(hlftester.test_string(s[i]));
        // at most one task with unscheduled siblings?
        line.push_back(oonsst.test_string(s[i]));
        // can snapshots be aggregated?
        line.push_back(srtt.test_string(s[i]));
        // hlf-1 level tasks unscheduled, but lower ones?
        line.push_back(uh1t.test_string(s[i]));
        // singlepredecessor task unscheduled?
        line.push_back(sst.test_string(s[i]));
        // as many topmost tasks as possible scheduled?
        line.push_back(tmt.test_string(s[i]));
#if PYTHON_TESTS
        // Python user-defined tests
        line.push_back(pythontest.test_string(s[i]));
#endif
        // Delimitter
        line.push_back(" ");
        // Initially scheduled tasks
        stringstream markedstring("");
        tree_id tid;
        // we need this to compute the isomorphism
        markedstring << "[" << stringhelper::join(s[i]->marked, ", ",
            [&](task_id m) {
#if USE_CANONICAL_SNAPSHOT
                task_id nexttask = NOTASK;
                for(auto it : isomorphisms[i]){
                    if(it.second == m){
                        nexttask = it.first;
                        break;
                    }
                }
                return nexttask;
#else
                return m;
#endif
            }
        ) << "]";
        line.push_back(markedstring.str());
        line.push_back("  ");
        line.push_back(get_expected_runtime_string(s[i]));
        line.push_back("  ");
#if MYFLOAT==GNUMP_RATIONAL
        // TODO: nice output of fractionals!
        // cout << "(" << s[i]->expected_runtime().get_d() << ")";
        // line.push_back(" ");
#endif
    RuntimeTester rtt;
    line.push_back(rtt.test_string(s[i], vm["processors"].as<int>()));
    line.push_back("  ");
    line.push_back(get_count_snaps_string(s[i]));
    }
    vector<unsigned int> column_widths(lines[0].size());
    for(auto line : lines){
        for(unsigned int i=0; i<line.size(); ++i){
            column_widths[i] = max(column_widths[i], (unsigned int)(line[i].size()));
        }
    }
    for(auto line : lines){
        assert(line.size() == column_widths.size());
        for(unsigned int i=0; i<line.size(); ++i){
            cout << line[i];
            for(unsigned int l=0; l<column_widths[i]-line[i].size(); ++l){
                cout << " ";
            }
        }
        cout << endl;
    }
}

int main(int argc, char** argv){
#if 0 // Currently just testing DAG stuff    
#define EDGES(x, ...) std::make_pair(x, vector<task_id>{__VA_ARGS__})
    std::map<task_id, vector<task_id>> edges {
        EDGES(8, 4),
        EDGES(7, 4),
        EDGES(5, 1, 0),
        EDGES(4, 3, 2),
        EDGES(3, 2, 1),
        EDGES(2, 0),
        EDGES(1, 0),
        EDGES(0)
    };
    auto dag = TaskDAG(edges);
    cout << dag << endl;
#undef EDGES
    cout << "Leafs: ";
    for (auto e : dag.get_leaves()) {
        cout << e << ", ";
    }
    cout << endl;


    return 0;
#endif
#if USE_SIMPLE_OPENMP // openmp settings - useful?
    omp_set_nested(1);
#endif

    try{
        // command line parsing stuff
        po::variables_map vm;
        if(read_variables_map_from_args(argc, argv, vm)!=0){
            return 1;
        }

        if(vm["verbosity"].as<int>() == 0){
            print_version(false);
        }

        // read tree and print it for user
        vector<Intree> trees = generate_tree(vm);
        for(Intree t : trees){
            if(vm["verbosity"].as<int>() < 2){
                cout << "Raw form:\t" << t << endl;
            }
            map<task_id, task_id> isomorphism;
            tree_id tid;
            // print tree to command line
            if(vm["verbosity"].as<int>() < 2){
                CommandLineExporter cle;
                cle.export_tree(cout, t);
            }
            cout << "Treeseq:\t";
            for(task_id it = 1; it < t.count_tasks(); ++it){
                cout << t.get_successor(it) << " ";
            }
            cout << endl;

            // compute snapshot dags
            vector<vector<task_id>> initial_settings;
            vector<Snapshot*> s;
            vector<map<task_id, task_id>> isomorphisms;
            vector<myfloat> probs;
            vector<myfloat> expected_runtimes;

            if(schedulers.find(vm["scheduler"].as<string>()) == schedulers.end()){
                cout << "Scheduler " << vm["scheduler"].as<string>() << " not found." << endl;
                return 1;
            }
            create_snapshot_dags(
                vm,
                t,
                schedulers.at(vm["scheduler"].as<string>()),
                initial_settings,
                isomorphisms,
                s,
                probs,
                expected_runtimes
            );

            // optimize current snapshot
            if(vm["optimize"].as<bool>()){
                if(vm["verbosity"].as<int>() < 2){
                    cout << "Optimizing schedules" << endl;
                }
                for(unsigned int i= 0; i<s.size(); ++i){
                    s[i] = s[i]->optimize();
                }
            }

            for(unsigned int i= 0; i<s.size(); ++i){
                s[i]->finalize();
            }

            // compute expected runtimes
            if(vm["verbosity"].as<int>() < 2){
                cout << "Computing expected runtimes." << endl;
            }
            assert(expected_runtimes.size() == s.size());
            for(unsigned int i= 0; i<s.size(); ++i){
                expected_runtimes[i] = s[i]->expected_runtime();
            }
            myfloat expected_runtime = (myfloat)0;
            vector<Snapshot*> best {s[0]};
            for(unsigned int i= 0; i<s.size(); ++i){
                if(i>0 && s[i]->expected_runtime() <= best[0]->expected_runtime()){
                    if(s[i]->expected_runtime() < best[0]->expected_runtime()){
                        best.clear();
                    }
                    best.push_back(s[i]);
                }
                expected_runtime += expected_runtimes[i] * probs[i];
            }

            if(vm["verbosity"].as<int>() < 1){
                cout << "Results:" << endl;
            }

            // output stats
            generate_stats(vm, s, best, initial_settings, isomorphisms);

            if(vm["verbosity"].as<int>() < 2){
                cout << "Total expected run time: " << expected_runtime << endl;
            }

            std::set<const Snapshot*> total_snapshot_set;
            for(Snapshot* it : s){
                it->count_snapshots_in_dag(total_snapshot_set);
            }
            cout << "Total number of snaps:   " 
                << total_snapshot_set.size()
                << endl;

            // output stuff to files
            generate_output(vm, s, best, initial_settings);
        }
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
