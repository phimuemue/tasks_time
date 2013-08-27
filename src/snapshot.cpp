#include "snapshot.h"

map<Snapshot::PoolKind, map<snapshot_id, Snapshot*>> Snapshot::pool;

void Snapshot::clear_pool(){
    // We have to ensure that we don't double-delete some pointers
    // TODO: Why does it not work this way?
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
    Successors(this),
    Probabilities(this),
    SuccessorProbabilities(this)
{

}

Snapshot::Snapshot(const Snapshot& s) :
    cache_expected_runtime(0),
    marked(s.marked),
    intree(s.intree),
    Successors(this),
    Probabilities(this),
    SuccessorProbabilities(this)
    {
}

Snapshot::Snapshot(Intree& t) :
    cache_expected_runtime(0),
    intree(t),
    Successors(this),
    Probabilities(this),
    SuccessorProbabilities(this)
{

}

Snapshot::Snapshot(Intree& t, vector<task_id> m) :
    cache_expected_runtime(0),
    marked(m),
    intree(t),
    Successors(this),
    Probabilities(this),
    SuccessorProbabilities(this)
{
    sort(m.begin(), m.end());
    for(auto it=m.begin(); it!=m.end(); ++it){
        vector<task_id> tmp;
        t.get_predecessors(*it, tmp);
        if(tmp.size()!=0){
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

Snapshot::Snapshot(Intree& t, 
        vector<task_id>& m, 
        vector<Snapshot*>& s,
        vector<myfloat>& sp) :
    cache_expected_runtime(0),
    successors(s),
    successor_probs(sp),
    marked(m),
    intree(t),
    Successors(this),
    Probabilities(this),
    SuccessorProbabilities(this)
{
    sort(m.begin(), m.end());
    for(auto it=m.begin(); it!=m.end(); ++it){
        vector<task_id> tmp;
        t.get_predecessors(*it, tmp);
        if(tmp.size()!=0){
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
    Snapshot::pool.clear();
#else
    for(auto it=successors.begin(); it!=successors.end(); ++it){
        delete(*it);
    }
#endif
}

Snapshot* Snapshot::canonical_snapshot(
        const Snapshot& s,
        Snapshot::PoolKind representant){
    Intree intreecopy(s.intree);
    vector<task_id> mcopy(s.marked);
    return canonical_snapshot(intreecopy, mcopy, representant);
}

Snapshot* Snapshot::canonical_snapshot(
        Intree& t, 
        vector<task_id> m,
        Snapshot::PoolKind representant){
    vector<task_id> original_m(m);
#if USE_SIMPLE_OPENMP
    cout << "Warning! Using openmp with canonical snapshots!" << endl;
#endif

    map<task_id, task_id> isomorphism;
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
        tmp.get_predecessors(it->first,
                predecessor_collection[it->first]);
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
        sort(predecessor_collection[it->first].begin(),
             predecessor_collection[it->first].end());
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
    vector<task_id> newmarked;
    for(auto it=m.begin(); it!=m.end(); ++it){
        newmarked.push_back(*it);
    }
    sort(newmarked.begin(), newmarked.end());
    auto find_key = snapshot_id(tid, newmarked);
    auto correct_pool = 
        Snapshot::pool[representant].find(find_key);
    if(correct_pool == Snapshot::pool[representant].end()){
        Snapshot::pool[representant][find_key] 
            = new Snapshot(tmp, newmarked);
    }
    return Snapshot::pool[representant].find(find_key)->second;
}



Snapshot* Snapshot::find_snapshot_in_pool(const Snapshot& s,
        Snapshot::PoolKind representant){
    Intree t(s.intree);
    vector<task_id> newmarked(s.marked);
    return find_snapshot_in_pool(t, newmarked, representant);
}

Snapshot* Snapshot::find_snapshot_in_pool(Intree& t,
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
#if SIMPLE_ISOMORPHISM_CHECK
        if(*finish_prob_it == 0)
            continue;
#endif
        Intree tmp(intree);
        tmp.remove_task(*it);
        vector<pair<task_id,myfloat>> raw_sucs;
        scheduler.get_next_tasks(tmp, marked, raw_sucs);
        // we have to check if the scheduler even found 
        // a new task to schedule
        if(raw_sucs.size() > 0){
            for(unsigned int i=0; i<raw_sucs.size(); ++i){
                vector<task_id> newmarked(marked);
                newmarked.erase(remove_if(
                            newmarked.begin(), newmarked.end(),
                            [it](const task_id& a){
                            return a==*it;
                            }), newmarked.end());
                if(raw_sucs[i].first != NOTASK)
                    newmarked.push_back(raw_sucs[i].first);
                sort(newmarked.begin(), newmarked.end());
                // TODO: every "new" needs a "delete"
#if USE_CANONICAL_SNAPSHOT
                Snapshot* news = Snapshot::canonical_snapshot(tmp, 
                        newmarked,
                        representant);
#else
                //Snapshot* news = new Snapshot(tmp, newmarked);
                Snapshot* news = Snapshot::find_snapshot_in_pool(tmp,
                        newmarked,
                        representant);
#endif
                successors.push_back(news);
                successor_probs.push_back(
                        *finish_prob_it * raw_sucs[i].second);
            }
        }
        else {
            vector<task_id> newmarked(marked);
            sort(newmarked.begin(), newmarked.end());
            newmarked.erase(remove_if(newmarked.begin(), newmarked.end(),
                        [it](const task_id& a){
                        return a==*it;
                        }), newmarked.end());
            // TODO: every "new" needs a "delete"
#if USE_CANONICAL_SNAPSHOT
            Snapshot* news = Snapshot::canonical_snapshot(tmp, 
                    newmarked,
                    representant);
#else
            //Snapshot* news = new Snapshot(tmp, newmarked);
            Snapshot* news = Snapshot::find_snapshot_in_pool(tmp,
                    newmarked,
                    representant);
#endif
            successors.push_back(news);
            successor_probs.push_back(*finish_prob_it);
        }
    }
    // remove duplicate successors
    assert(successors.size() == successor_probs.size());
    for(unsigned int i=0; i<successors.size(); ++i){
        for(unsigned int j=i+1; j<successors.size(); ++j){
            if(successors[i] == successors[j]){
                successors[j] = NULL;
                successor_probs[i] += successor_probs[j];
                successor_probs[j] = (myfloat)0;
            }
        }
    }
    successors.erase(remove_if(successors.begin(), successors.end(),
        [](const Snapshot* a) -> bool {
            return a == NULL;
        }
    ), successors.end());
    successor_probs.erase(remove_if(
                successor_probs.begin(), successor_probs.end(),
                [](const myfloat& a) -> bool {
                    return a == (myfloat)0;
                }
    ), successor_probs.end());
}

void Snapshot::compile_snapshot_dag(const Scheduler& scheduler,
    Snapshot::PoolKind representant){
    if(successors.size() > 0){
        return;
    }
    get_successors(scheduler, representant);
    for(unsigned int i=0; i<successors.size(); ++i){
        successors[i]->compile_snapshot_dag(scheduler, representant);
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
    assert(successor_probs.size() == successors.size());
    // TODO: compute expected minimum runtime of marked 
    // threads dynamically
    myfloat expected_runtime_of_min_task = 
        ((myfloat)1)/(myfloat)marked.size();
    myfloat result = expected_runtime_of_min_task;
    myfloat suc_expected_runtimes[successors.size()];
    for(unsigned int i=0; i<successors.size(); ++i){
        suc_expected_runtimes[i] = successors[i]->expected_runtime();
    }
    for(unsigned int i=0; i<successors.size(); ++i){
        result += successor_probs[i] * suc_expected_runtimes[i];
    }
    return cache_expected_runtime = result;
}

bool Snapshot::is_hlf() const {
    if(cache_is_hlf.find(1) != cache_is_hlf.end()){
        return cache_is_hlf[1];
    }
    if(intree.count_tasks()==1){
        return true;
    }
    vector<task_id> leaves;
    intree.get_leaves(leaves);
    vector<unsigned int> leaf_levels;
    for(auto t : leaves){
        leaf_levels.push_back(intree.get_level(t));
    }
    sort(leaf_levels.begin(), leaf_levels.end(), greater<task_id>());
    vector<unsigned int> m_levels;
    for(auto t : marked){
        m_levels.push_back(intree.get_level(t));
    }
    sort(m_levels.begin(), m_levels.end(), greater<task_id>());
    for(unsigned int i=0; i<m_levels.size(); ++i){
        if(m_levels[i]<leaf_levels[i]){
            return cache_is_hlf[1] = false;
        }
    }
    for(auto it : successors){
        if(!it->is_hlf()){
            return cache_is_hlf[1] = false;
        }
    }
    return cache_is_hlf[1] = true;
}

myfloat Snapshot::expected_time_for_n_processors(unsigned int p) const {
    if(cache_expected_runtime_for_n_procs.find(p) != cache_expected_runtime_for_n_procs.end()){
        return cache_expected_runtime_for_n_procs[p];
    }
    if(p==1 && intree.count_tasks() == 1){
        return 1;
    }
    myfloat result = (myfloat)0;
    for(auto it : SuccessorProbabilities){
        myfloat tmp = it.get<1>();
        myfloat transition_time = ((myfloat)1)/((myfloat) marked.size());
        myfloat future_time = it.get<0>()->expected_time_for_n_processors(p);
        tmp *= ((marked.size() == p ? transition_time : 0) + future_time);
        result += tmp;
    }
    cache_expected_runtime_for_n_procs[p] = result;
    return result;
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
    for(Snapshot* it : successors){
        const Snapshot* nosp = it->get_next_on_successor_path(t);
        if(nosp != NULL){
            return it;
        }
    }
    return NULL;
}

myfloat Snapshot::get_reaching_probability(const Snapshot* t) const {
    myfloat result = (myfloat)0;
    if(t->intree.count_tasks()==1 || t==this){
        return (myfloat)1;
    }
    if(t->intree.count_tasks() > intree.count_tasks()){
        return (myfloat)0;
    }
    for(auto it : SuccessorProbabilities){
        const Snapshot* nosp = 
            it.get<0>()->get_next_on_successor_path(t);
        if(nosp!=NULL){
            result += 
                (it.get<1>()) * it.get<0>()->get_reaching_probability(t);
        }
    }
    return result;
}

Snapshot* Snapshot::optimize() const {
    tree_id tid;
    map<task_id, task_id> iso;
    Intree new_intree(intree);
    vector<task_id> new_marked(marked);
    Intree::canonical_intree(new_intree, new_marked, iso, tid);

    
    pair<tree_id, vector<task_id>> opt_finder(tid, new_marked);
    if(Snapshot::pool[PoolOptimized].find(opt_finder) != 
            Snapshot::pool[PoolOptimized].end()) {
        return Snapshot::pool[PoolOptimized][opt_finder];
    }

    vector<pair<Snapshot*, myfloat>> new_sucs;
    for(auto it : SuccessorProbabilities){
        auto optimized_suc = it.get<0>()->optimize();
        new_sucs.push_back(
                pair<Snapshot*, myfloat>(optimized_suc, it.get<1>())
                );
    }
    // sort successors into different vectors, 
    // each one only containing successors with
    // same intree structure.
    map<tree_id, decltype(new_sucs)> sucs_by_tree_id;
    for(const pair<Snapshot*, myfloat>& s : new_sucs){
        tree_id tid;
        map<task_id, task_id> iso;
        vector<task_id> none_marked;
        Intree::canonical_intree(
            s.first->intree,
            none_marked,
            iso,
            tid);
        sucs_by_tree_id[tid].push_back(s);
    }
    // traverse all sucs with same intree structure,
    // and only leave the best!
    for(auto it=sucs_by_tree_id.begin(); it!=sucs_by_tree_id.end(); ++it){
        // sum up probabilities for current intree structure
        myfloat orig_prob_sum = (myfloat)0;
        for(const pair<Snapshot*, myfloat>& s : it->second){
                orig_prob_sum += s.second;
        }
        // cout << "Successors to choose from:" << endl;
        // for(auto iit : it->second){
        //     cout << iit.first << ": " << iit.first->expected_runtime() << endl;
        // }
        // get one best intree structure
        pair<Snapshot*, myfloat> best_one = 
            *min_element(it->second.begin(), it->second.end(),
            [](const pair<Snapshot*, myfloat>& a, 
               const pair<Snapshot*, myfloat>& b) -> bool {
                    return a.first->expected_runtime() <=    
                           b.first->expected_runtime();
                }
        );
        // only leave the best element(s)!
        it->second.erase(
            remove_if(it->second.begin(), it->second.end(),
                [&best_one](const pair<Snapshot*, myfloat>& a) -> bool {
                    return a.first->expected_runtime() > 
                           best_one.first->expected_runtime();
                }
            ),
            it->second.end()
        );
        // cout << "Successors chosen:" << endl;
        // for(auto iit : it->second){
        //     cout << iit.first << ": " << iit.first->expected_runtime() << endl;
        // }
        // compute new sum of probabilities to normalize the old ones
        myfloat new_prob_sum = (myfloat)0;
        for_each(it->second.begin(), it->second.end(),
            [&](const pair<Snapshot*, myfloat>& s) {
                new_prob_sum += s.second;
            }
        );
        // normalize old probabilities so that total sum is still 1
        transform(it->second.begin(), it->second.end(), it->second.begin(),
            [&](pair<Snapshot*, myfloat>& s) -> pair<Snapshot*, myfloat> {
                return pair<Snapshot*, myfloat>(
                        s.first,
                        (orig_prob_sum / new_prob_sum) * s.second
                    );
            }
        );
    }
    // put all new successors/probs into new vectors and return optimized
    // version of snapshot
    vector<Snapshot*> new_successors;
    vector<myfloat> new_successor_probs;
    for(auto it=sucs_by_tree_id.begin(); it!=sucs_by_tree_id.end(); ++it){
        for(auto s=it->second.begin(); s!=it->second.end(); ++s){
            new_successors.push_back(s->first);
            new_successor_probs.push_back(s->second);
        }
    }
    Snapshot* result = new Snapshot(
        new_intree, 
        new_marked,
        new_successors,
        new_successor_probs
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
    for(auto s : Successors){
        sum += s->count_snapshots_in_dag(tmp);
    }
    return sum;
}

bool Snapshot::operator== (const Snapshot& s) const {
    return marked==s.marked && intree==s.intree;
}
