#include "snapshot.h"

map<Snapshot::PoolKind, map<snapshot_id, Snapshot*>> Snapshot::pool;

void Snapshot::clear_pool(){
    cout << "Pool size(s): ";
    for(auto pp : Snapshot::pool){
        cout << pp.second.size() << ", ";
    }
    cout << endl;
    std::set<Snapshot*> done;
    for_each(Snapshot::pool.begin(), Snapshot::pool.end(),
        [&](const pair<Snapshot::PoolKind, map<snapshot_id, Snapshot*>>& p){
            for(auto const& it : p.second){
                if(done.find(it.second)==done.end()){
                    delete(it.second);
                    done.insert(it.second);
                }
            }
        }
    );
}

Snapshot::Snapshot() :
    Snapshot(Intree())
{
}

Snapshot::Snapshot(const Snapshot& s) :
    Snapshot(s.intree, s.marked)
{
}

Snapshot::Snapshot(const Intree& t) :
    Snapshot(t, {})
{
}

Snapshot::Snapshot(const Intree& t, vector<task_id> m) :
    Snapshot(t, m, {})
{
}

Snapshot::Snapshot(const Intree& t, 
        vector<task_id> const& m, 
        vector<SuccessorInfo> && s
    ) :
    cache_expected_runtime(0),
    finalized(false),
    successors(s),
    marked(m),
    intree(t)
{
    for(auto const& it : marked){
        if(t.get_predecessors(it).size()!=0){
            cout << "Trying to construct snapshot " 
                << "with non-leaf marked tasks." << endl;
            cout << t << endl;
            for(auto const& it : marked){
                cout << it << endl;
            }
            throw 1;
        }
    }
}
Snapshot::~Snapshot(){
#if USE_CANONICAL_SNAPSHOT
    //Snapshot::pool.clear();
#else
    for(auto it : successors){
        delete(it);
    }
#endif
}

std::pair<Snapshot*, std::map<task_id, task_id>> Snapshot::canonical_snapshot(
        const Snapshot& s,
        Snapshot::PoolKind representant){
    return canonical_snapshot(s.intree, s.marked, representant);
}

std::pair<Snapshot*, std::map<task_id, task_id>> Snapshot::canonical_snapshot(
        const Intree& t, 
        vector<task_id> const& m,
        Snapshot::PoolKind representant){
#if USE_SIMPLE_OPENMP
    cout << "Warning! Using openmp with canonical snapshots!" << endl;
#endif

    // do not use unique_ptr, because the result must possibly exist
    // after the function has terminated
	tree_id tid;
    auto pairIntreeIso = Intree::canonical_intree(t, m, tid);
    Intree tmp(std::move(pairIntreeIso.first));
    auto isomorphism(std::move(pairIntreeIso.second));

    // adjust m properly (i.e. always "lowest possible task" for 'iso-snap')
    map<task_id, unsigned int> counts;
    for (task_id tid : m) {
        auto a = isomorphism[tid];
        if (tmp.contains_task(a) && tid!=0) {
            counts[tmp.get_successor(a)]++;
        }
    }
    map<task_id, vector<task_id>> predecessor_collection;
    for(auto const& it : counts){
        predecessor_collection[it.first] = tmp.get_predecessors(it.first);
        // remove non-leaf tasks from predecessors
        predecessor_collection[it.first].erase(
            remove_if(predecessor_collection[it.first].begin(),
                predecessor_collection[it.first].end(),
                [&tmp](const task_id a) -> bool {
                    return !tmp.is_leaf(a);
                }
            )
            , predecessor_collection[it.first].end()
        );
#if USE_CANONICAL_SNAPSHOT
        assert(is_sorted(predecessor_collection[it.first].begin(),
               predecessor_collection[it.first].end()));
#else
        // TODO: is it necessary to sort here?
        sort(predecessor_collection[it.first].begin(),
             predecessor_collection[it.first].end());
#endif
    }
    vector<task_id> newmarked;
    for(auto const& it : counts){
        assert(predecessor_collection[it.first].size() >= it.second);
        for(unsigned int i=0; i<it.second; ++i){
            newmarked.push_back(predecessor_collection[it.first][i]);
        }
    }

#if USE_CANONICAL_SNAPSHOT
    assert(is_sorted(newmarked.begin(), newmarked.end()));
#else
    sort(newmarked.begin(), newmarked.end());
#endif
    auto const find_key = snapshot_id(tid, newmarked);
    auto& snappool = Snapshot::pool[representant];
    auto const snaphint = snappool.lower_bound(find_key);
    auto const result = (snaphint == snappool.end() || (snappool.key_comp()(find_key, snaphint->first))) // not found in pool
        ? snappool.emplace_hint(snaphint, find_key, new Snapshot(std::move(tmp), newmarked))->second
        : snaphint->second;
    assert(snappool.find(find_key) != snappool.end());
    return std::make_pair(std::move(result), std::move(isomorphism));
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
    auto& snappool = Snapshot::pool[representant];
    auto const snaphint = snappool.lower_bound(find_key);
    Snapshot* const result = (snaphint == snappool.end() || (snappool.key_comp()(find_key, snaphint->first))) // not found in pool
        ? snappool.emplace_hint(snaphint, find_key, new Snapshot(t, m))->second
        : snaphint->second;
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
                    successors[j].probability = myfloat(0);
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
        task_id originalsuccessor = intree.remove_task(current_finished_task);
        auto raw_sucs = scheduler.get_next_tasks(intree, marked);
        // we have to check if the scheduler even found 
        // a new task to schedule
        if(raw_sucs.size() > 0){
            for(unsigned int i=0; i<raw_sucs.size(); ++i){
                vector<task_id> newmarked(marked);
                newmarked.erase(remove_if( newmarked.begin(), newmarked.end(),
                    [current_finished_task](const task_id& a){
                        return a==current_finished_task;
                    }
                ), newmarked.end());
                assert(raw_sucs[i].first[0] != NOTASK);
                // TODO: remove the following if if the above assert
                // has been in for some time and never kicked in
                if(raw_sucs[i].first[0] != NOTASK){
                    newmarked.insert(newmarked.end(), raw_sucs[i].first.begin(), raw_sucs[i].first.end());
                }
#if USE_CANONICAL_SNAPSHOT
#else
                sort(newmarked.begin(), newmarked.end());
#endif
#if USE_CANONICAL_SNAPSHOT
                Snapshot* news = Snapshot::canonical_snapshot(
                    intree, 
                    newmarked,
                    representant
                ).first;
#else
                Snapshot* news = Snapshot::find_snapshot_in_pool(intree,
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
                        [current_finished_task](const task_id& a){
                        return a==current_finished_task;
                        }), newmarked.end());
#if USE_CANONICAL_SNAPSHOT
            Snapshot* news = Snapshot::canonical_snapshot(
                intree, 
                newmarked,
                representant
            ).first;
#else
            Snapshot* news = Snapshot::find_snapshot_in_pool(intree,
                    newmarked,
                    representant);
#endif
            assert(current_finished_task != NOTASK);
            successors.emplace_back(current_finished_task, *finish_prob_it, news);
        }
        // restore task
        intree.edges[current_finished_task] = originalsuccessor;
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

Snapshot* Snapshot::optimize() const {
    tree_id tid;
    intree.get_raw_tree_id(tid);
    
    auto opt_finder = std::make_pair(std::move(tid), marked);
    auto cached_result = Snapshot::pool[PoolOptimized].find(opt_finder);
    if(cached_result != Snapshot::pool[PoolOptimized].end()) {
        return cached_result->second;
    }

    std::map<task_id, std::vector<SuccessorInfo>> sucs_by_finished_task;
    std::map<task_id, myfloat> prob_by_finished_task;
    // sort successors into different vectors, 
    // each one only containing successors with
    // same intree structure. Thereby, only keep best ones
    for(auto suc : successors){
        auto optimized_suc = suc.snapshot->optimize();
        auto& sbf2 = sucs_by_finished_task[suc.task];
        // clear if newly found optimized snapshot is better than all current ones
        if (!sbf2.empty() && sbf2[0].snapshot->expected_runtime() > optimized_suc->expected_runtime()) {
            sbf2.clear();
        }
        // add new snapshot if at least as good as other ones collected so far
        if (sbf2.empty() || sbf2[0].snapshot->expected_runtime() == optimized_suc->expected_runtime()) {
            sbf2.emplace_back(suc.task, suc.probability, optimized_suc);
        }
        prob_by_finished_task[suc.task] += suc.probability;
    }
    
    // normalize probabilities so that they sum to 1 again
    for(auto& sbyt : sucs_by_finished_task) {
        myfloat new_prob_sum(0);
        for(auto const& sinfo : sbyt.second) {
            new_prob_sum += sinfo.probability;
        }
        for(auto& sinfo : sbyt.second) {
            sinfo.probability *= (prob_by_finished_task[sinfo.task] / new_prob_sum);
        }
    }

    // put all new successors/probs into new vectors and return optimized
    // version of snapshot
    vector<SuccessorInfo> new_successors;
    for(auto const& it : sucs_by_finished_task){
        for(auto& s : it.second){
            new_successors.push_back(s);
        }
    }
    Snapshot* result = new Snapshot(intree, marked, std::move(new_successors));
    Snapshot::pool[Snapshot::PoolKind::PoolOptimized]
                  [opt_finder] 
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
    std::set<const Snapshot*> tmp;
    return count_snapshots_in_dag(tmp);
}

unsigned long Snapshot::count_snapshots_in_dag(std::set<const Snapshot*>& tmp) const {
    if(tmp.find(this)!=tmp.end()){
        return 0;
    }
    tmp.insert(this);
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
