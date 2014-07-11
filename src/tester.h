#ifndef TESTER_H
#define TESTER_H

#include "snapshot.h"

#include<iostream>
#include<string>
#include<vector>
#include<map>

#include "boost/tuple/tuple.hpp"
#include "boost/tuple/tuple_comparison.hpp"

using namespace std;

// Tester offers an interface that can be used to examine snapshots.
// It is implemented using static polymorphism via the CRTP.
// The "subclasses" must implement the following methods:
//   string test_string(const Snapshot*, Args...)
//   RetVal test(const Snapshot*, Args...)
//   RetVal combine_function(const Snapshot*, RetVal, vector<RetVal>)
template<class Derived, typename RetVal, typename... Args>
class Tester {
    public:
        RetVal test_first(const Snapshot* s, Args... args);
        RetVal test_all(const Snapshot* s, Args... args);
    private:
        map<boost::tuple<const Snapshot*, Args...>, RetVal> cache_first;
        map<boost::tuple<const Snapshot*, Args...>, RetVal> cache_all;
};

template<class Derived, typename RetVal, typename... Args>
RetVal Tester<Derived, RetVal, Args...>::test_first(const Snapshot* s, Args... args){
    auto finder = boost::tuple<const Snapshot*, Args...>
        (s, args...);
    if (cache_first.find(finder) != cache_first.end()){
        return cache_first[finder];
    }
    return cache_first[finder] = static_cast<Derived*>(this)->test(s, args...);
}

template<class Derived, typename RetVal, typename... Args>
RetVal Tester<Derived, RetVal, Args...>::test_all(const Snapshot* s, Args... args){
    auto finder = boost::tuple<const Snapshot*, Args...>
        (s, args...);
    if (cache_all.find(finder) != cache_all.end()){
        return cache_all[finder];
    }
    vector<RetVal> values;
    for(auto suc : s->Successors()){
        values.push_back(test_all(suc.snapshot, args...));
    }
    return cache_all[finder] = static_cast<Derived*>(this)->combine_function(s, static_cast<Derived*>(this)->test(s, args...), values);
}

#endif
