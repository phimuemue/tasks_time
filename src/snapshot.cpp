#include "snapshot.h"

map<Snapshot::PoolKind, map<snapshot_id, Snapshot*>> Snapshot::pool;

void Snapshot::clear_pool(){
    cout << "Pool size(s): ";
    for(auto pp : Snapshot::pool){
        cout << pp.second.size() << ", ";
    }
    cout << endl;
    map<Snapshot*, bool> done;
    for_each(Snapshot::pool.begin(), Snapshot::pool.end(),
        [&](const pair<Snapshot::PoolKind, map<snapshot_id, Snapshot*>>& p){
            for(auto it=p.second.begin(); it!=p.second.end(); ++it){
                if(done.find(it->second)!=done.end()){
                    delete(it->second);
                    done[it->second] = true;
                }
            }
        }
    );
}

Snapshot::Snapshot() :
    cache_expected_runtime(0),
    finalized(false)
{
}

Snapshot::Snapshot(const Snapshot& s) :
    cache_expected_runtime(0),
    finalized(false),
    marked(s.marked),
    intree(s.intree)
{
}

Snapshot::Snapshot(const Intree& t) :
    cache_expected_runtime(0),
    finalized(false),
    intree(t)
{
}

Snapshot::Snapshot(const Intree& t, vector<task_id> m) :
    cache_expected_runtime(0),
    finalized(false),
    marked(m),
    intree(t)
{
    sort(m.begin(), m.end());
    for(auto it=m.begin(); it!=m.end(); ++it){
        if(t.get_predecessors(*it).size()!=0){
            cout << "Trying to construct snapshot " 
                << "with non-leaf marked tasks." << endl;
            cout << t << endl;
            for(auto it=m.begin(); it!=m.end(); ++it){
                cout << *it << endl;
            }
            throw 1;
        }
    }
}

Snapshot::Snapshot(const Intree& t, 
        vector<task_id>& m, 
        vector<SuccessorInfo>& s
    ) :
    cache_expected_runtime(0),
    finalized(false),
    successors(s),
    marked(m),
    intree(t)
{
    sort(m.begin(), m.end());
    for(auto it=m.begin(); it!=m.end(); ++it){
        if(t.get_predecessors(*it).size()!=0){
            cout << "Trying to construct snapshot " 
                << "with non-leaf marked tasks." << endl;
            cout << t << endl;
            for(auto it=m.begin(); it!=m.end(); ++it){
                cout << *it << endl;
            }
            throw 1;
        }
    }
}
Snapshot::~Snapshot(){
#if USE_CANONICAL_SNAPSHOT
    //Snapshot::pool.clear();
#else
    for(auto it=successors.begin(); it!=successors.end(); ++it){
        delete(*it);
    }
#endif
}

Snapshot* Snapshot::canonical_snapshot(
        const Snapshot& s,
        map<task_id, task_id>* isomorphism,
        Snapshot::PoolKind representant){
    Intree intreecopy(s.intree);
    vector<task_id> mcopy(s.marked);
    return canonical_snapshot(intreecopy, mcopy, isomorphism, representant);
}

Snapshot* Snapshot::canonical_snapshot(
        const Intree& t, 
        vector<task_id> m,
        map<task_id, task_id>* _isomorphism,
        Snapshot::PoolKind representant){
#if USE_SIMPLE_OPENMP
    cout << "Warning! Using openmp with canonical snapshots!" << endl;
#endif

    // do not use unique_ptr, because the result must possibly exist
    // after the function has terminated
    map<task_id, task_id>* tmp_iso;
    if(_isomorphism == NULL){
        tmp_iso = new map<task_id, task_id>();
    }
    else{
        tmp_iso = _isomorphism;
    }
    map<task_id, task_id>& isomorphism = *tmp_iso;
    //map<task_id, task_id> isomorphism;
    tree_id tid;
    Intree tmp = Intree::canonical_intree(t, m, isomorphism, tid);

    // adjust m properly (i.e. always "lowest possible task" for 'iso-snap')
    transform(m.begin(), m.end(), m.begin(),
        [&](const task_id a) -> task_id {
            return isomorphism[a];
        }
    );
    m.erase(remove_if(m.begin(), m.end(),
        [&tmp](const task_id a) -> bool {
            return !tmp.contains_task(a);
        }), 
        m.end()
    );
    map<task_id, unsigned int> counts;
    for(auto it=m.begin(); it!=m.end(); ++it){
        if(*it!=0){
            counts[tmp.get_edge_from(*it).second.get_id()]++;
        }
    }
    map<task_id, vector<task_id>> predecessor_collection;
    for(auto it=counts.begin(); it!=counts.end(); ++it){
        predecessor_collection[it->first] = tmp.get_predecessors(it->first);
        // remove non-leaf tasks from predecessors
        predecessor_collection[it->first].erase(
            remove_if(predecessor_collection[it->first].begin(),
                predecessor_collection[it->first].end(),
                [&tmp](const task_id a) -> bool {
                    return !tmp.is_leaf(a);
                }
            )
            , predecessor_collection[it->first].end()
        );
#if USE_CANONICAL_SNAPSHOT
        assert(is_sorted(predecessor_collection[it->first].begin(),
               predecessor_collection[it->first].end()));
#else
        // TODO: is it necessary to sort here?
        sort(predecessor_collection[it->first].begin(),
             predecessor_collection[it->first].end());
#endif
    }
    m.clear();
    for(auto it=counts.begin(); it!=counts.end(); ++it){
        for(auto ii=predecessor_collection[it->first].begin();
            ii != predecessor_collection[it->first].end();
            ++ii){
        }
        assert(predecessor_collection[it->first].size() >= it->second);
        for(unsigned int i=0; i<it->second; ++i){
            m.push_back(predecessor_collection[it->first][i]);
        }
    }

    // construct newmarked
    vector<task_id> newmarked(m);
    sort(newmarked.begin(), newmarked.end());
    auto find_key = snapshot_id(tid, newmarked);
    auto correct_pool = 
        Snapshot::pool[representant].find(find_key);
    if(correct_pool == Snapshot::pool[representant].end()){
        Snapshot::pool[representant][find_key] 
            = new Snapshot(tmp, newmarked);
    }
    assert(Snapshot::pool[representant].find(find_key) != Snapshot::pool[representant].end());
    if(_isomorphism == NULL){
        delete tmp_iso;
    }
    return Snapshot::pool[representant].find(find_key)->second;
}



Snapshot* Snapshot::find_snapshot_in_pool(const Snapshot& s,
        Snapshot::PoolKind representant){
    Intree t(s.intree);
    vector<task_id> newmarked(s.marked);
    return find_snapshot_in_pool(t, newmarked, representant);
}

Snapshot* Snapshot::find_snapshot_in_pool(const Intree& t,
        vector<task_id> m,
        Snapshot::PoolKind representant){
    tree_id tid;
    t.get_raw_tree_id(tid);
    snapshot_id find_key = snapshot_id(tid, m);
    Snapshot* result;
    if(Snapshot::pool[representant].find(find_key) == Snapshot::pool[representant].end()){
        Snapshot::pool[representant][find_key] = new Snapshot(t, m);
    }
    result = Snapshot::pool[representant][find_key];
    assert(result->intree == t);
    return result;
}

void Snapshot::consolidate(bool strict){
    for(unsigned int i=0; i<successors.size(); ++i){
        for(unsigned int j=i+1; j<successors.size(); ++j){
            if(successors[i].snapshot == successors[j].snapshot){
                if(!strict || successors[i].task == successors[j].task){
                    successors[i].probability += successors[j].probability;
                    // nulling out position j
                    successors[j].snapshot = nullptr;
                    successors[j].probability = (myfloat)0;
                    successors[j].task = NOTASK;
                }
            }
        }
    }
    successors.erase(remove_if(successors.begin(), successors.end(),
        [](const SuccessorInfo& a) -> bool {
            return a.snapshot==nullptr;
        }
    ), successors.end());
}

void Snapshot::finalize(){
    if(finalized)
        return;
    for(auto s : successors){
        s.snapshot->finalize();
    }
    consolidate(false);
    finalized = true;
}

void Snapshot::get_successors(const Scheduler& scheduler,
    Snapshot::PoolKind representant){
    // we only want to compute the successors once
    if(successors.size()>0)
        return;
    // or maybe there aren't even successors
    if(intree.count_tasks()==1)
        return;
    vector<myfloat> finish_probs;
    Probability_Computer().compute_finish_probs(intree, marked, finish_probs);
    assert(finish_probs.size()==marked.size());
    // then, for each finished threads, compute all possible successors
    auto finish_prob_it = finish_probs.begin();
    for(auto it = marked.begin(); it!=marked.end(); 
            ++it, ++finish_prob_it){
        task_id current_finished_task = *it;
        // TODO: We can probably remove this because of canonical snapshots
#if SIMPLE_ISOMORPHISM_CHECK
        if(*finish_prob_it == 0)
            continue;
#endif
        Intree tmp = intree.remove_task(*it);
        vector<pair<vector<task_id>,myfloat>> raw_sucs;
        scheduler.get_next_tasks(tmp, marked, raw_sucs);
        // we have to check if the scheduler even found 
        // a new task to schedule
        if(raw_sucs.size() > 0){
            for(unsigned int i=0; i<raw_sucs.size(); ++i){
                vector<task_id> newmarked(marked);
                newmarked.erase(remove_if( newmarked.begin(), newmarked.end(),
                    [it](const task_id& a){
                        return a==*it;
                    }
                ), newmarked.end());
                assert(raw_sucs[i].first[0] != NOTASK);
                // TODO: remove the following if if the above assert
                // has been in for some time and never kicked in
                if(raw_sucs[i].first[0] != NOTASK){
                    newmarked.insert(newmarked.end(), raw_sucs[i].first.begin(), raw_sucs[i].first.end());
                }
                sort(newmarked.begin(), newmarked.end());
#if USE_CANONICAL_SNAPSHOT
                Snapshot* news = Snapshot::canonical_snapshot(tmp, 
                        newmarked,
                        NULL, 
                        representant);
#else
                Snapshot* news = Snapshot::find_snapshot_in_pool(tmp,
                        newmarked,
                        representant);
#endif
                assert(current_finished_task != NOTASK);
                successors.emplace_back(current_finished_task, *finish_prob_it * raw_sucs[i].second, news);
            }
        }
        else {
            vector<task_id> newmarked(marked);
            sort(newmarked.begin(), newmarked.end());
            newmarked.erase(remove_if(newmarked.begin(), newmarked.end(),
                        [it](const task_id& a){
                        return a==*it;
                        }), newmarked.end());
#if USE_CANONICAL_SNAPSHOT
            Snapshot* news = Snapshot::canonical_snapshot(tmp, 
                    newmarked,
                    NULL,
                    representant);
#else
            Snapshot* news = Snapshot::find_snapshot_in_pool(tmp,
                    newmarked,
                    representant);
#endif
            assert(current_finished_task != NOTASK);
            successors.emplace_back(current_finished_task, *finish_prob_it, news);
        }
    }
    // remove duplicate successors
    consolidate();
}

void Snapshot::compile_snapshot_dag(const Scheduler& scheduler,
    Snapshot::PoolKind representant){
    if(successors.size() > 0){
        return;
    }
    get_successors(scheduler, representant);
    for(unsigned int i=0; i<successors.size(); ++i){
        successors[i].snapshot->compile_snapshot_dag(scheduler, representant);
    }
}

size_t Snapshot::get_successor_count(){
    return successors.size();
}

myfloat Snapshot::expected_runtime() const {
    if(cache_expected_runtime != 0){
        return cache_expected_runtime;
    }
    if (successors.size() == 0){
        return Probability_Computer().get_expected_remaining_time(intree, 0);
    }
    myfloat expected_runtime_of_min_task = 
        Probability_Computer().expected_runtime_of_min_task(intree, marked);
    myfloat result = expected_runtime_of_min_task;
    myfloat suc_expected_runtimes[successors.size()];
    for(unsigned int i=0; i<successors.size(); ++i){
        suc_expected_runtimes[i] = successors[i].snapshot->expected_runtime();
    }
    for(unsigned int i=0; i<successors.size(); ++i){
        result += successors[i].probability * suc_expected_runtimes[i];
    }
    return cache_expected_runtime = result;
}

ostream& operator<<(ostream& os, const Snapshot& s){
    os << "<" << s.intree << " | [";
    
    for(auto it = s.marked.begin(); it != s.marked.end(); ++it){
        os << *it;
        if (it+1!=s.marked.end()){
            os << ", ";
        }
    }
    os << "]>";
    return os;
}

const Snapshot* Snapshot::get_next_on_successor_path(const Snapshot* t) const {
    if(this==t){
        return t;
    }
    for(auto suc : successors){
        Snapshot* it = suc.snapshot;
        const Snapshot* nosp = it->get_next_on_successor_path(t);
        if(nosp != nullptr){
            return it;
        }
    }
    return nullptr;
}

unsigned int Snapshot::count_tasks() const {
    return intree.count_tasks();
}

myfloat Snapshot::get_reaching_probability(const Snapshot* t) const {
    myfloat result = (myfloat)0;
    if(t->intree.count_tasks()==1 || t==this){
        return (myfloat)1;
    }
    if(t->intree.count_tasks() > intree.count_tasks()){
        return (myfloat)0;
    }
    for(auto it : Successors()){
        const Snapshot* nosp = it.snapshot->get_next_on_successor_path(t);
        if(nosp!=nullptr){
            result += 
                (it.probability) * it.snapshot->get_reaching_probability(t);
        }
    }
    return result;
}

// TODO: Maybe we can speed up things by not iterating over boost::tuples,
// but instead use the proper vectors directly.
Snapshot* Snapshot::optimize() const {
    tree_id tid;
    const Intree& new_intree = intree;
    vector<task_id> new_marked(marked);
#if USE_CANONICAL_SNAPSHOT
    // map<task_id, task_id> iso;
    // Intree::canonical_intree(new_intree, new_marked, iso, tid);
    new_intree.get_raw_tree_id(tid);
#else
    new_intree.get_raw_tree_id(tid);
#endif
    
    pair<tree_id, vector<task_id>> opt_finder(tid, new_marked);
    if(Snapshot::pool[PoolOptimized].find(opt_finder) != 
            Snapshot::pool[PoolOptimized].end()) {
        return Snapshot::pool[PoolOptimized][opt_finder];
    }

    vector<SuccessorInfo> new_sucs;
    for(unsigned int i = 0; i < successors.size(); ++i){
        auto optimized_suc = successors[i].snapshot->optimize();
        new_sucs.emplace_back(successors[i].task, successors[i].probability, optimized_suc);
    }
    // sort successors into different vectors, 
    // each one only containing successors with
    // same intree structure.
    map<task_id, decltype(new_sucs)> sucs_by_finished_task;
    for(auto& s : new_sucs){
        // tree_id tid;
        // map<task_id, task_id> iso;
        // vector<task_id> none_marked;
        // Intree::canonical_intree(
        //     s.get<0>()->intree,
        //     none_marked,
        //     iso,
        //     tid);
        sucs_by_finished_task[s.task].push_back(s);
    }
    // traverse all sucs with same intree structure,
    // and only leave the best!
    for(auto it=sucs_by_finished_task.begin(); it!=sucs_by_finished_task.end(); ++it){
        // sum up probabilities for current intree structure
        myfloat orig_prob_sum = (myfloat)0;
        for(auto& s : it->second){
            orig_prob_sum += s.probability;
        }
        // get one best intree structure
        auto best_one = 
            *min_element(it->second.begin(), it->second.end(),
            [](const SuccessorInfo& a, const SuccessorInfo& b) -> bool {
                return a.snapshot->expected_runtime() <= b.snapshot->expected_runtime();
            }
        );
        // only leave the best element(s)!
        it->second.erase(
            remove_if(it->second.begin(), it->second.end(),
                [&best_one](const SuccessorInfo& a) -> bool {
                    return a.snapshot->expected_runtime() > 
                           best_one.snapshot->expected_runtime();
                }
            ),
            it->second.end()
        );
        // compute new sum of probabilities to normalize the old ones
        myfloat new_prob_sum = (myfloat)0;
        for(auto const & s : it->second){
            new_prob_sum += s.probability;
        }
        // normalize old probabilities so that total sum is still 1
        for(auto& s : it->second){
            s.probability *= (orig_prob_sum / new_prob_sum);
        }
    }
    // put all new successors/probs into new vectors and return optimized
    // version of snapshot
    vector<SuccessorInfo> new_successors;
    for(auto it=sucs_by_finished_task.begin(); it!=sucs_by_finished_task.end(); ++it){
        for(auto s=it->second.begin(); s!=it->second.end(); ++s){
            new_successors.push_back(*s);
        }
    }
    Snapshot* result = new Snapshot(
        new_intree, 
        new_marked,
        new_successors
    );
    Snapshot::pool[Snapshot::PoolKind::PoolOptimized]
                  [pair<tree_id, vector<task_id>>(tid, new_marked)] 
                  = (result);
    return result;
}

string Snapshot::markedstring(){
    stringstream os;
    os << "[";
    
    for(auto it = marked.begin(); it != marked.end(); ++it){
        os << *it;
        if (it+1!=marked.end()){
            os << ", ";
        }
    }
    os << "]";
    return os.str();
}

unsigned long Snapshot::count_snapshots_in_dag() const {
    map<const Snapshot*, bool> tmp;
    return count_snapshots_in_dag(tmp);
}

unsigned long Snapshot::count_snapshots_in_dag(map<const Snapshot*, bool>& tmp) const {
    if(tmp.find(this)!=tmp.end()){
        return 0;
    }
    tmp[this] = true;
    unsigned long sum = 1;
    for(auto s : Successors()){
        sum += s.snapshot->count_snapshots_in_dag(tmp);
    }
    return sum;
}

bool Snapshot::operator== (const Snapshot& s) const {
    return marked==s.marked && intree==s.intree;
}

#if PYTHON_TESTS
#include <boost/python.hpp>
using namespace boost::python;
BOOST_PYTHON_MODULE(snapshot)
{
    class_<Snapshot>("Snapshot")
        .def("count_tasks", &Snapshot::count_tasks)
        .def("get_successor_count", &Snapshot::get_successor_count)
        .def_readonly("intree", &Snapshot::intree)
        .def_readonly("marked", &Snapshot::marked)
        ;
}
#endif
